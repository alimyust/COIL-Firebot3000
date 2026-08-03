#ifndef AUDIO_HANDLER_HPP
#define AUDIO_HANDLER_HPP

#include <Arduino.h>
#include "scheduler.h"
#include <ProtocolCommands.h>
#include "Microphone.hpp"
#include "Speaker.hpp"

class AudioHandler {
public:
    static const uint8_t TARGET_ROBOT_NODE = ProtocolCommands::NODE_ROBOT;
    static const uint8_t CMD_AUDIO = ProtocolCommands::CMD_AUDIO;
    static const uint8_t CMD_AUDIO_RAW = ProtocolCommands::CMD_AUDIO_RAW;

    AudioHandler(EventScheduler &scheduler, Microphone &mic, Speaker &speaker, bool debug_enabled);

    // Business logic processing routines
    void onAudioTrigger();
    void processAudio(const ProtocolCommands::RadioAudioPacket& payload);

    // ========================================================================
    // UNIVERSAL SCHEDULER STATIC ROUTING BRIDGES
    // ========================================================================
    
    // Periodic trigger bridge (For TX)
    static void onAudioUpdate(void* context) {
        static_cast<AudioHandler*>(context)->onAudioTrigger();
    }

    // Packet reception bridge (For RX)
    static void onAudioPacketReceived(const RadioComm::RF69_Packet& packet, void* context) {
        const ProtocolCommands::RadioAudioPacket* payload = reinterpret_cast<const ProtocolCommands::RadioAudioPacket*>(packet.payload);
        static_cast<AudioHandler*>(context)->processAudio(*payload);
    }

    static void onAudioPacketReceivedRaw(const RadioComm::RF69_Packet& packet, void* context) {
        const ProtocolCommands::RadioAudioPacketRaw* payload = reinterpret_cast<const ProtocolCommands::RadioAudioPacketRaw*>(packet.payload);
        static_cast<AudioHandler*>(context)->processAudioRaw(*payload);
    }

    void beginTimer();


    void processAndSend(const int16_t* pcm_segment);
    void processAndSendRaw(const int16_t* pcm_segment);
    void decodePacket(const ProtocolCommands::RadioAudioPacket* packet, int16_t* output_pcm);
    void processAudioRaw(const ProtocolCommands::RadioAudioPacketRaw& payload);
    void decodePacketRaw(const ProtocolCommands::RadioAudioPacketRaw* packet, int16_t* output_pcm);

    // G.711 u-law codec methods
    uint8_t encodeSample(int16_t sample);
    int16_t decodeSample(uint8_t ulaw_byte);
    
private:

    EventScheduler &_scheduler;
    Microphone &_mic;
    Speaker &_speaker;
    bool _debug_enabled;

    // Sequence counters
    uint16_t _global_sequence;
    uint16_t _last_rx_sequence;
};

#endif // AUDIO_HANDLER_HPP