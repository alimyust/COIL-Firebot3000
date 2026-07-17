#ifndef AUDIO_HANDLER_HPP
#define AUDIO_HANDLER_HPP

#include <Arduino.h>
#include "scheduler.h"
#include "ProtocolCommands.hpp"
#include "Microphone.hpp"
#include "Speaker.hpp"



class AudioHandler {
public:
    static const uint8_t TARGET_ROBOT_NODE = ProtocolCommands::NODE_ROBOT;
    static const uint8_t CMD_AUDIO = ProtocolCommands::CMD_AUDIO;

    AudioHandler(EventScheduler &scheduler, Microphone &mic, Speaker &speaker, bool debug_enabled);

    // Business logic processing routines
    void onAudioTrigger();
    void processAudio(const ProtocolCommands::RadioAudioPacket& payload);

    // ========================================================================
    // UNIVERSAL SCHEDULER STATIC ROUTING BRIDGES
    // ========================================================================
    
    // Periodic trigger bridge (For TX)
    static void onAudioUpdate(void* context) {
        if (static_cast<AudioHandler*>(context)->_debug_enabled) {
            Serial.println("Audio Update Triggered");
        }
        static_cast<AudioHandler*>(context)->onAudioTrigger();
    }

    // Packet reception bridge (For RX)
    static void onAudioPacketReceived(const RadioComm::RF69_Packet& packet, void* context) {
        // Extract the structured payload directly from the raw packet
        if (static_cast<AudioHandler*>(context)->_debug_enabled) {
            Serial.print("Audio Packet Received");
        }
        const ProtocolCommands::RadioAudioPacket* payload = reinterpret_cast<const ProtocolCommands::RadioAudioPacket*>(packet.payload);
        static_cast<AudioHandler*>(context)->processAudio(*payload);
    }

private:
    void processAndSend(const int16_t* pcm_segment);
    void decodePacket(const ProtocolCommands::RadioAudioPacket* packet, int16_t* output_pcm);
    
    uint8_t encodeSample(int16_t sample);
    int16_t decodeSample(uint8_t nibble);

    EventScheduler &_scheduler;
    Microphone &_mic;
    Speaker &_speaker;
    bool _debug_enabled;

    // Tx/Encoder State
    int16_t _encoder_predicted;
    int8_t _encoder_step;
    uint16_t _global_sequence;

    // Rx/Decoder State
    int16_t _decoder_predicted;
    int8_t _decoder_step;
    uint16_t _last_rx_sequence;
};

#endif // AUDIO_HANDLER_HPP