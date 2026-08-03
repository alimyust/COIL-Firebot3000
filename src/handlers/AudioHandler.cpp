#include "AudioHandler.hpp"

// ============================================================================
// CONSTRUCTOR
// ============================================================================
AudioHandler::AudioHandler(EventScheduler &scheduler, Microphone &mic, Speaker &speaker, bool debug_enabled)
    : _scheduler(scheduler), 
      _mic(mic), 
      _speaker(speaker), 
      _debug_enabled(debug_enabled),
      _global_sequence(0),
      _last_rx_sequence(0) {}

// ============================================================================
// TRANSMIT PATH (TX)
// ============================================================================
void AudioHandler::onAudioTrigger() {
    if (!_mic.isBufferReady()) {
        return;
    }
    int16_t pcm_buffer[SAMPLE_BLOCK_LENGTH];
    _mic.readActiveBuffer(pcm_buffer);
    processAndSend(&pcm_buffer[0]);
}

void AudioHandler::processAndSend(const int16_t* pcm_segment) {
    ProtocolCommands::RadioAudioPacket packet;
    packet.sequence = _global_sequence++;

    // Standard u-law is 8 bits per sample (1 byte per sample).
    // Iterates across the block to encode samples into 8-bit u-law bytes.
    const size_t num_samples = SAMPLE_BLOCK_LENGTH / 2; 
    for (size_t i = 0; i < num_samples; i++) {
        packet.data[i] = encodeSample(pcm_segment[i]);
    }

    if (_debug_enabled) {
        Serial.print("Sending Audio Packet: Seq=");
        Serial.println(packet.sequence);
    }
    _scheduler.sendPacket(TARGET_ROBOT_NODE, CMD_AUDIO, &packet, sizeof(ProtocolCommands::RadioAudioPacket));
}

// ============================================================================
// RECEIVE PATH (RX)
// ============================================================================
void AudioHandler::processAudio(const ProtocolCommands::RadioAudioPacket& payload) {
    // if (_debug_enabled) Serial.println("queue audio");
    _last_rx_sequence = payload.sequence;

    int16_t decompressed_pcm[SAMPLE_BLOCK_LENGTH];
    decodePacket(&payload, decompressed_pcm);

    const size_t num_samples = SAMPLE_BLOCK_LENGTH / 2;
    for (size_t i = 0; i < num_samples; i++) {
        if (!_speaker.queueAudio(decompressed_pcm[i])) {
            Serial.println("BUFFER OVERFLOW: Dropped sample!");
        }   
    }
}

void AudioHandler::decodePacket(const ProtocolCommands::RadioAudioPacket* packet, int16_t* output_pcm) {
    const size_t num_samples = SAMPLE_BLOCK_LENGTH;
    for (size_t i = 0; i < num_samples; i++) {
        output_pcm[i] = decodeSample(packet->data[i]);
    }
}

// ============================================================================
// G.711 u-LAW CODEC ALGORITHMS
// ============================================================================
#define BIAS 0x84 // 132 bias for u-law conversion
#define CLIP 32635

uint8_t AudioHandler::encodeSample(int16_t sample) {
    uint16_t sign = (sample < 0) ? 0x80 : 0x00;
    if (sample < 0) {
        sample = -sample;
    }
    if (sample > CLIP) {
        sample = CLIP;
    }

    sample += BIAS;

    uint8_t exponent = 7;
    for (int exp_mask = 0x4000; exp_mask > 0; exp_mask >>= 1) {
        if (sample & exp_mask) {
            break;
        }
        exponent--;
    }

    uint8_t mantissa = (sample >> (exponent + 3)) & 0x0F;
    uint8_t ulaw_byte = ~(sign | (exponent << 4) | mantissa);
    
    return ulaw_byte;
}

int16_t AudioHandler::decodeSample(uint8_t ulaw_byte) {
    // Invert all bits according to standard G.711 u-law spec
    ulaw_byte = ~ulaw_byte;

    uint8_t sign = ulaw_byte & 0x80;
    uint8_t exponent = (ulaw_byte >> 4) & 0x07;
    uint8_t mantissa = ulaw_byte & 0x0F;

    // Reconstruct 14-bit linear magnitude with bias added back
    int32_t sample = ((mantissa << 3) + 0x84) << exponent;
    sample -= BIAS;

    return (sign) ? -((int16_t)sample) : (int16_t)sample;
}