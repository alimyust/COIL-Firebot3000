#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "radio.h"
#include "ProtocolCommands.hpp"

enum class EventPriority : uint8_t {
    PRIORITY_LOW = 0,
    PRIORITY_MEDIUM,
    PRIORITY_HIGH,
    PRIORITY_CRITICAL
};

typedef void (*PacketHandlerCallback)(const RadioComm::RF69_Packet& packet, void* context);
typedef void (*PeriodicTaskCallback)(void* context);

struct SystemEvent {
    bool is_periodic;
    EventPriority priority;
    void* context;
    
    RadioComm::RF69_Packet packet;
    PacketHandlerCallback packet_callback;
    PeriodicTaskCallback periodic_callback;
};

class EventScheduler {
public:
    static const size_t MAX_EVENTS = 12;
    static const size_t MAX_PACKET_HANDLERS = 8;
    static const size_t MAX_PERIODIC_TASKS = 4;

    EventScheduler(RadioComm& radio, bool debug_enabled) 
        : _radio(radio), _event_count(0), _task_count(0), _packet_handler_count(0), _debug_enabled(debug_enabled) {}

    /**
     * @brief NEW: Replaces ProtocolLayer outbound queue mechanics.
     * Passes raw payloads down to Layer 1 safely and non-blockingly.
     */
    bool sendPacket(uint8_t target_node, uint8_t command_id, const void* payload, uint8_t payload_len) {
        if (_debug_enabled) {
            Serial.print("Tx: ");
            printIncomingPayload(command_id, payload);
        }
        return _radio.send(target_node, command_id, payload, payload_len);
    }

    bool registerPacketHandler(uint8_t command_id, EventPriority priority, PacketHandlerCallback callback, void* context) {
        if (_packet_handler_count >= MAX_PACKET_HANDLERS) return false;
        _packet_registry[_packet_handler_count] = {command_id, priority, callback, context};
        _packet_handler_count++;
        return true;
    }

    bool addPeriodicTask(unsigned long interval_ms, EventPriority priority, PeriodicTaskCallback callback, void* context) {
        if (_task_count >= MAX_PERIODIC_TASKS) return false;
        _periodic_tasks[_task_count] = {interval_ms, 0, priority, callback, context, true};
        _task_count++;
        return true;
    }

    void update() {
        unsigned long current_time = millis();

        // 1. Evaluate Periodic Clock States
        for (size_t i = 0; i < _task_count; i++) {
            PeriodicTask& task = _periodic_tasks[i];
            if (task.enabled && (current_time - task.last_run_ms >= task.interval_ms)) {
                task.last_run_ms = current_time;

                SystemEvent ev;
                ev.is_periodic = true;
                ev.priority = task.priority;
                ev.periodic_callback = task.callback;
                ev.packet_callback = nullptr;
                ev.context = task.context;
                
                pushEvent(ev);
            }
        }

        // 2. Route Inbound Frames based on command mappings
        while (_radio.available()) {
            RadioComm::RF69_ReceivedPacket rx_packet;
            if (_radio.receive(rx_packet)) {
                for (size_t i = 0; i < _packet_handler_count; i++) {
                    if (_packet_registry[i].command_id == rx_packet.packet.command) {
                        SystemEvent ev;
                        ev.is_periodic = false;
                        ev.priority = _packet_registry[i].priority;
                        ev.packet = rx_packet.packet;
                        ev.packet_callback = _packet_registry[i].callback;
                        ev.periodic_callback = nullptr;
                        ev.context = _packet_registry[i].context;
                        if (_debug_enabled) {
                            Serial.print("Rx: ");
                            printIncomingPayload(rx_packet.packet.command, rx_packet.packet.payload);
                        }
                        pushEvent(ev);
                        break;
                    }
                }
            }
        }

        // 3. Process Highest Priority Item
        dispatchNextEvent();
    }

private:
    struct PeriodicTask {
        unsigned long interval_ms;
        unsigned long last_run_ms;
        EventPriority priority;
        PeriodicTaskCallback callback;
        void* context;
        bool enabled;
    };

    struct PacketHandlerMapping {
        uint8_t command_id;
        EventPriority priority;
        PacketHandlerCallback callback;
        void* context;
    };

    RadioComm& _radio;
    SystemEvent _priority_queue[MAX_EVENTS];
    size_t _event_count;
    
    PeriodicTask _periodic_tasks[MAX_PERIODIC_TASKS];
    size_t _task_count;
    
    PacketHandlerMapping _packet_registry[MAX_PACKET_HANDLERS];
    size_t _packet_handler_count;

    bool _debug_enabled = true;

    void printIncomingPayload(uint8_t command, const void* payload) {
        switch (command) {
            case ProtocolCommands::CMD_THROTTLE: {
                const ProtocolCommands::ThrottlePayload* throttle = (const ProtocolCommands::ThrottlePayload*)payload;
                Serial.print("Throttle Duty: ");
                Serial.println(throttle->duty); 
                break;
            }
            
            case ProtocolCommands::CMD_STEERING: {
                const ProtocolCommands::SteeringPayload* steering = (const ProtocolCommands::SteeringPayload*)payload;
                Serial.print("Steering Duty: ");
                Serial.println(steering->duty);
                break;
            }
            
            case ProtocolCommands::CMD_AUDIO: {
                const ProtocolCommands::RadioAudioPacket* audio = (const ProtocolCommands::RadioAudioPacket*)payload;

                Serial.print("AUD [Seq: ");
                Serial.print(audio->sequence);
                Serial.print(" | Pred: ");
                Serial.print(audio->init_predicted);
                Serial.print(" | StepIdx: ");
                Serial.print(audio->init_step_index);
                Serial.print(" | Hex: ");
                
                // Print out just the first 4 bytes of compressed data in hex to verify it's not flatlining
                for (int i = 0; i < 4; i++) {
                    if (audio->data[i] < 0x10) Serial.print("0"); // Leading zero padding
                    Serial.print(audio->data[i], HEX);
                    Serial.print(" ");
                }
                Serial.println("...]");
                break;
                break;
            }
            case ProtocolCommands::CMD_HB: {
                const ProtocolCommands::HeartbeatPayload* hb = (const ProtocolCommands::HeartbeatPayload*)payload;
                Serial.print("Heartbeat Timestamp: ");
                Serial.println(hb->timestamp);
                break;
            }
            default:
                Serial.println("Unknown command, can't print structure.");
                Serial.print("Command ID: ");
                Serial.println(command, HEX);
                break;
        }
    }

    void pushEvent(const SystemEvent& ev) {
        if (_event_count >= MAX_EVENTS) return;
        size_t i = _event_count;
        while (i > 0 && static_cast<uint8_t>(_priority_queue[i - 1].priority) < static_cast<uint8_t>(ev.priority)) {
            _priority_queue[i] = _priority_queue[i - 1];
            i--;
        }
        _priority_queue[i] = ev;
        _event_count++;
    }

    void dispatchNextEvent() {
        if (_event_count == 0) return;
        SystemEvent active_event = _priority_queue[0];
        for (size_t i = 1; i < _event_count; i++) {
            _priority_queue[i - 1] = _priority_queue[i];
        }
        _event_count--;

        if (active_event.is_periodic && active_event.periodic_callback != nullptr) {
            active_event.periodic_callback(active_event.context);
        } else if (!active_event.is_periodic && active_event.packet_callback != nullptr) {
            active_event.packet_callback(active_event.packet, active_event.context);
        }
    }
};

#endif // SCHEDULER_HPP