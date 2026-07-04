#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "radio.h"

// ============================================================================
// SYSTEM ARCHITECTURE TYPES & DEFINITIONS
// ============================================================================

// The Shared Source of Truth for the vehicle. 
// Stage 3 writes to this; Stage 2 reads from this to build radio packets.
struct VehicleTelemetry {
    int16_t battery_voltage;
    int16_t sensor_reading;
    uint32_t last_update_time_ms;
};

// System-wide Event Types
enum class EventType : uint8_t {
    UNKNOWN = 0,
    MOTOR_THROTTLE,
    MOTOR_STEERING,
    SENSOR_POLL,       // Clock-driven trigger to read local physical sensors
    SYSTEM_CRITICAL
};

// Urgency tiers for sorting the execution queue
enum class EventPriority : uint8_t {
    PRIORITY_LOW,
    PRIORITY_MEDIUM,
    PRIORITY_HIGH,
    PRIORITY_CRITICAL
};

// Unified internal container for all executable items
struct SystemEvent {
    EventType type;
    EventPriority priority;
    RadioComm::RF69_Packet packet; // Empty/zeroed out for purely internal timer events
};

// Stage 3 Functional Callback Types
typedef void (*PacketHandlerCallback)(const RadioComm::RF69_Packet& packet);
typedef void (*PeriodicTaskCallback)(VehicleTelemetry& state);

// ============================================================================
// EVENT SCHEDULER CLASS INTERFACE
// ============================================================================
class EventScheduler {
public:
    static const size_t MAX_EVENTS = 12;
    static const size_t MAX_PACKET_HANDLERS = 8;
    static const size_t MAX_PERIODIC_TASKS = 4;

    EventScheduler(RadioComm& radio) 
        : _radio(radio), _event_count(0), _task_count(0), _packet_handler_count(0) {}

    /**
     * @brief Registers a Stage 3 callback handler to process incoming radio packets
     */
    bool registerPacketHandler(EventType type, PacketHandlerCallback callback) {
        if (_packet_handler_count >= MAX_PACKET_HANDLERS) return false;
        _packet_registry[_packet_handler_count] = {type, callback};
        _packet_handler_count++;
        return true;
    }

    /**
     * @brief Schedules an internal clock-driven hardware reading event
     */
    bool addPeriodicTask(EventType type, EventPriority priority, unsigned long interval_ms, PeriodicTaskCallback callback) {
        if (_task_count >= MAX_PERIODIC_TASKS) return false;
        _periodic_tasks[_task_count] = {type, priority, interval_ms, 0, callback, true};
        _task_count++;
        return true;
    }

    /**
     * @brief Main processing cycle. Manages software clocks, updates hardware ring buffers,
     * prioritizes events, and dispatches actions using the shared system telemetry state.
     */
    void update(VehicleTelemetry& global_state) {
        unsigned long current_time = millis();

        // Step A: Evaluate Internal Periodic Timers
        for (size_t i = 0; i < _task_count; i++) {
            PeriodicTask& task = _periodic_tasks[i];
            if (task.enabled && (current_time - task.last_run_ms >= task.interval_ms)) {
                task.last_run_ms = current_time;

                SystemEvent internal_ev;
                internal_ev.type = task.type;
                internal_ev.priority = task.priority;
                memset(&internal_ev.packet, 0, sizeof(internal_ev.packet));

                pushEvent(internal_ev);
            }
        }

        // Step B: Ingest Asynchronous Radio Packets from Stage 1
        while (_radio.available()) {
            RadioComm::RF69_ReceivedPacket rx_packet;
            if (_radio.receive(rx_packet)) {
                SystemEvent ev = translatePacket(rx_packet.packet);
                pushEvent(ev);
            }
        }

        // Step C: Pop and execute the single highest priority action
        dispatchNextEvent(global_state);
    }

private:
    struct PeriodicTask {
        EventType type;
        EventPriority priority;
        unsigned long interval_ms;
        unsigned long last_run_ms;
        PeriodicTaskCallback callback;
        bool enabled;
    };

    struct PacketHandlerMapping {
        EventType type;
        PacketHandlerCallback callback;
    };

    RadioComm& _radio;
    
    // Static fixed-size priority queue memory layouts
    SystemEvent _priority_queue[MAX_EVENTS];
    size_t _event_count;
    
    PeriodicTask _periodic_tasks[MAX_PERIODIC_TASKS];
    size_t _task_count;
    
    PacketHandlerMapping _packet_registry[MAX_PACKET_HANDLERS];
    size_t _packet_handler_count;

    /**
     * @brief Maps a raw radio command protocol byte to a designated systemic Event Type
     */
    SystemEvent translatePacket(const RadioComm::RF69_Packet& packet) {
        SystemEvent ev;
        ev.packet = packet;
        switch (packet.command) {
            case 0x01: ev.type = EventType::MOTOR_THROTTLE;  ev.priority = EventPriority::PRIORITY_HIGH;     break;
            case 0x02: ev.type = EventType::MOTOR_STEERING;  ev.priority = EventPriority::PRIORITY_HIGH;     break;
            case 0x99: ev.type = EventType::SYSTEM_CRITICAL;  ev.priority = EventPriority::PRIORITY_CRITICAL; break;
            default:   ev.type = EventType::UNKNOWN;         ev.priority = EventPriority::PRIORITY_LOW;      break;
        }
        return ev;
    }

    /**
     * @brief Inserts an event using standard compile-time array insertion-sort mechanics
     */
    void pushEvent(const SystemEvent& ev) {
        if (_event_count >= MAX_EVENTS) return; // Buffer overflow safety mitigation
        
        size_t i = _event_count;
        while (i > 0 && static_cast<uint8_t>(_priority_queue[i - 1].priority) < static_cast<uint8_t>(ev.priority)) {
            _priority_queue[i] = _priority_queue[i - 1];
            i--;
        }
        _priority_queue[i] = ev;
        _event_count++;
    }

    /**
     * @brief Executes the highest priority item and handles Stage 2 bridging logic
     */
    void dispatchNextEvent(VehicleTelemetry& global_state) {
        if (_event_count == 0) return;

        // Extracts head of queue (highest priority item)
        SystemEvent active_event = _priority_queue[0];

        // Shift lower array items forward cleanly
        for (size_t i = 1; i < _event_count; i++) {
            _priority_queue[i - 1] = _priority_queue[i];
        }
        _event_count--;

        // Branch execution based on whether it is an internal timer or external radio packet
        if (active_event.type == EventType::SENSOR_POLL) {
            // Find and run the registered periodic task
            for (size_t i = 0; i < _task_count; i++) {
                if (_periodic_tasks[i].type == active_event.type && _periodic_tasks[i].callback != nullptr) {
                    
                    // 1. EXECUTE STAGE 3 (Purely updates telemetry struct variables)
                    _periodic_tasks[i].callback(global_state);

                    // 2. STAGE 2 BRIDGING LOGIC (Extracts data and transmits out via Stage 1)
                    char payload_buffer[16];
                    itoa(global_state.sensor_reading, payload_buffer, 10);
                    
                    // Transmit telemetric packet non-blockingly to Base Station (Node 0) under Command 0x03
                    _radio.send(0, 0x03, payload_buffer);
                    return;
                }
            }
        } else {
            // Find and execute matching incoming packet handlers
            for (size_t i = 0; i < _packet_handler_count; i++) {
                if (_packet_registry[i].type == active_event.type && _packet_registry[i].callback != nullptr) {
                    
                    // EXECUTE STAGE 3 packet parsing
                    _packet_registry[i].callback(active_event.packet);
                    return;
                }
            }
        }
    }
};

#endif // SCHEDULER_HPP