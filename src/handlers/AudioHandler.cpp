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
      _last_rx_sequence(0),
      _jitter_head(0),
      _jitter_tail(0),
      _jitter_count(0),
      _expected_sequence(0),
      _have_previous_frame(false),
      _have_last_frame(false) {}

void AudioHandler::processAudioLoop() {
    static unsigned long last_tx_ms = 0;
    static const unsigned long tx_interval_ms = 20; // ~50 Hz packet cadence for 24-sample frames
    static int16_t pending_samples[kSamplesPerFrame];
    static size_t pending_count = 0;

    if (!_mic.isBufferReady()) {
        return;
    }

    if (millis() - last_tx_ms < tx_interval_ms) {
        return;
    }

    int16_t pcm_buffer[SAMPLE_BLOCK_LENGTH];
    _mic.readActiveBuffer(pcm_buffer);

    for (size_t i = 0; i < kSamplesPerFrame; ++i) {
        pending_samples[i] = pcm_buffer[i];
    }
    pending_count = kSamplesPerFrame;

    processAndSendRaw(pending_samples);
    last_tx_ms = millis();
}

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

void AudioHandler::processAndSendRaw(const int16_t* pcm_segment) {
    ProtocolCommands::RadioAudioPacketRaw packet;
    packet.sequence = _global_sequence++;

    const size_t num_samples = 24;
    for (size_t i = 0; i < num_samples; i++) {
        packet.data[i] = pcm_segment[i];
    }

    if (_debug_enabled) {
        Serial.print("Sending Raw Audio Packet: Seq=");
        Serial.println(packet.sequence);
    }
    _scheduler.sendPacket(TARGET_ROBOT_NODE, CMD_AUDIO_RAW, &packet, sizeof(ProtocolCommands::RadioAudioPacketRaw));
}

// ============================================================================
// RECEIVE PATH (RX)
// ============================================================================
void AudioHandler::processAudio(const ProtocolCommands::RadioAudioPacket& payload) {
    _last_rx_sequence = payload.sequence;

    int16_t decompressed_pcm[24];
    decodePacket(&payload, decompressed_pcm);

    queueJitterBufferFrame(decompressed_pcm, payload.sequence);
    flushJitterBufferToSpeaker();
}

void AudioHandler::processAudioRaw(const ProtocolCommands::RadioAudioPacketRaw& payload) {
    _last_rx_sequence = payload.sequence;

    int16_t pcm_samples[24];
    decodePacketRaw(&payload, pcm_samples);

    queueJitterBufferFrame(pcm_samples, payload.sequence);
    flushJitterBufferToSpeaker();
}

void AudioHandler::decodePacket(const ProtocolCommands::RadioAudioPacket* packet, int16_t* output_pcm) {
    const size_t num_samples = 24;
    for (size_t i = 0; i < num_samples; i++) {
        output_pcm[i] = decodeSample(packet->data[i]);
    }
}

void AudioHandler::decodePacketRaw(const ProtocolCommands::RadioAudioPacketRaw* packet, int16_t* output_pcm) {
    const size_t num_samples = 24;
    for (size_t i = 0; i < num_samples; i++) {
        output_pcm[i] = packet->data[i];
    }
}

void AudioHandler::queueJitterBufferFrame(const int16_t* pcm_samples, uint16_t sequence) {
    if (_jitter_count >= kJitterBufferSize) {
        _jitter_tail = (_jitter_tail + 1) % kJitterBufferSize;
        _jitter_count--;
    }

    int16_t* dst = _jitter_buffer[_jitter_head];
    for (size_t i = 0; i < kSamplesPerFrame; ++i) {
        dst[i] = pcm_samples[i];
    }

    _jitter_head = (_jitter_head + 1) % kJitterBufferSize;
    _jitter_count++;

    if (_have_last_frame && sequence > _expected_sequence) {
        const size_t missing_frames = sequence - _expected_sequence;
        for (size_t gap = 0; gap < missing_frames; ++gap) {
            queueFrameToSpeaker(_last_frame);
        }
    }

    if (sequence < _expected_sequence) {
        return;
    }

    queueFrameToSpeaker(pcm_samples);
    memcpy(_last_frame, pcm_samples, sizeof(_last_frame));
    _have_last_frame = true;
    _expected_sequence = sequence + 1;
    _have_previous_frame = true;
}

void AudioHandler::flushJitterBufferToSpeaker() {
    if (_jitter_count == 0) {
        return;
    }

    int16_t* frame = _jitter_buffer[_jitter_tail];
    queueFrameToSpeaker(frame);
    _jitter_tail = (_jitter_tail + 1) % kJitterBufferSize;
    _jitter_count--;
}

void AudioHandler::queueFrameToSpeaker(const int16_t* pcm_samples) {
    for (size_t i = 0; i < kSamplesPerFrame; ++i) {
        if (!_speaker.queueAudio(pcm_samples[i])) {
            break;
        }
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