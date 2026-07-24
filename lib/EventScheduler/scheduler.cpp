
// #include "scheduler.h"

// EventScheduler::EventScheduler(RadioComm& radio) : _radio(radio), _event_count(0) {
//     for (size_t i = 0; i < MAX_HANDLERS; i++) {
//         _registry[i].type = EventType::UNKNOWN;
//         _registry[i].callback = nullptr;
//     }
// }

// bool EventScheduler::registerHandler(EventType type, EventHandlerCallback callback) {
//     for (size_t i = 0; i < MAX_HANDLERS; i++) {
//         if (_registry[i].type == EventType::UNKNOWN) {
//             _registry[i].type = type;
//             _registry[i].callback = callback;
//             return true;
//         }
//     }
//     return false; // Registry full
// }

// void EventScheduler::update() {
//     // Step 1: Ingest packets from Stage 1 and translate them to prioritized events
//     while (_radio.available()) {
//         RadioComm::RF69_ReceivedPacket rx_packet;
//         if (_radio.receive(rx_packet)) {
//             SystemEvent ev = translatePacket(rx_packet.packet);
//             pushEvent(ev);
//         }
//     }

//     // Step 2: Dispatch queued prioritized events to Stage 3 handlers
//     dispatchNextEvent();
// }

// SystemEvent EventScheduler::translatePacket(const RadioComm::RF69_Packet& packet) {
//     SystemEvent ev;
//     ev.packet = packet;

//     // Map your raw radio command bytes to specific Event architecture properties
//     switch (packet.command) {
//         case 0x01:
//             ev.type = EventType::MOTOR_THROTTLE;
//             ev.priority = EventPriority::HIGH;
//             break;
//         case 0x02:
//             ev.type = EventType::MOTOR_STEERING;
//             ev.priority = EventPriority::HIGH;
//             break;
//         case 0x03:
//             ev.type = EventType::SENSOR_DATA;
//             ev.priority = EventPriority::MEDIUM;
//             break;
//         case 0x99:
//             ev.type = EventType::SYSTEM_CRITICAL;
//             ev.priority = EventPriority::CRITICAL;
//             break;
//         default:
//             ev.type = EventType::UNKNOWN;
//             ev.priority = EventPriority::LOW;
//             break;
//     }
//     return ev;
// }

// void EventScheduler::pushEvent(const SystemEvent& ev) {
//     if (_event_count >= MAX_EVENTS) return; // Queue overflow prevention

//     size_t i = _event_count;
//     // Shift lower priority elements downward to bubble high priority to the front (Index 0)
//     while (i > 0 && static_cast<uint8_t>(_priority_queue[i - 1].priority) < static_cast<uint8_t>(ev.priority)) {
//         _priority_queue[i] = _priority_queue[i - 1];
//         i--;
//     }

//     _priority_queue[i] = ev;
//     _event_count++;
// }

// void EventScheduler::dispatchNextEvent() {
//     if (_event_count == 0) return;

//     // Highest priority event is always at index 0
//     SystemEvent active_event = _priority_queue[0];

//     // Shift remaining items forward
//     for (size_t i = 1; i < _event_count; i++) {
//         _priority_queue[i - 1] = _priority_queue[i];
//     }
//     _event_count--;

//     // Execute the registered callback matching the event type
//     for (size_t i = 0; i < MAX_HANDLERS; i++) {
//         if (_registry[i].type == active_event.type && _registry[i].callback != nullptr) {
//             _registry[i].callback(active_event.packet); // Handover execution to Stage 3
//             return;
//         }
//     }
// }
