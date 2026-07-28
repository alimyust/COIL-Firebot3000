#include "AudioHandler.hpp"

// ============================================================================
// ADPCM STATIC LOOKUP TABLES
// ============================================================================
static const int16_t STEP_TABLE[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230,
    253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963,
    1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
    3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487,
    12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static const int8_t INDEX_TABLE[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

// ============================================================================
// CONSTRUCTOR
// ============================================================================
AudioHandler::AudioHandler(EventScheduler &scheduler, Microphone &mic, Speaker &speaker, bool debug_enabled)
    : _scheduler(scheduler), 
      _mic(mic), 
      _speaker(speaker), 
      _debug_enabled(debug_enabled),
      _encoder_predicted(0), 
      _encoder_step(0), 
      _global_sequence(0),
      _decoder_predicted(0), 
      _decoder_step(0), 
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
    // if (_debug_enabled) {
    //     Serial.println("Audio Triggered");
    // }
    processAndSend(&pcm_buffer[0]);
    // processAndSend(&pcm_buffer[SAMPLE_BLOCK_LENGTH / 2]);
}

void AudioHandler::beginTimer() {
    // Enable GCLK1/GCLK0 for TC3
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | 
                        GCLK_CLKCTRL_GEN_GCLK0 | 
                        GCLK_CLKCTRL_ID_TCC2_TC3;
    while (GCLK->STATUS.bit.SYNCBUSY);

    TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);

    // 48 MHz / 16 = 3 MHz clock
    // Target: 16000 Hz -> 3,000,000 / 16000 = 187.5 counts - 1 = 186
    TC3->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | 
                             TC_CTRLA_PRESCALER_DIV16 | 
                             TC_CTRLA_WAVEGEN_MFRQ;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);

    TC3->COUNT16.CC[0].reg = 186; // Exactly 16 kHz interrupt frequency
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);

    TC3->COUNT16.INTENSET.reg = TC_INTENSET_MC0;
    
    NVIC_SetPriority(TC3_IRQn, 2);
    NVIC_EnableIRQ(TC3_IRQn);

    TC3->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
}

void AudioHandler::processAndSend(const int16_t* pcm_segment) {
    ProtocolCommands::RadioAudioPacket packet;
    packet.sequence = _global_sequence++;
    packet.init_predicted = _encoder_predicted;
    packet.init_step_index = _encoder_step;

    size_t sample_idx = 0;   // V runs 32 times to pack 64 bytes into 32 bytes (1+1 nibbles per iteration)
    for (size_t i = 0; i < SAMPLE_BLOCK_LENGTH/2; i++) {
        uint8_t nibble1 = encodeSample(pcm_segment[sample_idx++]);
        uint8_t nibble2 = encodeSample(pcm_segment[sample_idx++]);
        packet.data[i] = (nibble1 << 4) | (nibble2 & 0x0F);
    }
    if (_debug_enabled) {
        Serial.print("Sending Audio Packet: Seq=");
        Serial.print(packet.sequence);
        Serial.print(", InitPredicted=");
        Serial.print(packet.init_predicted);
        Serial.print(", InitStep=");
        Serial.println(packet.init_step_index);
    }
    _scheduler.sendPacket(TARGET_ROBOT_NODE, CMD_AUDIO, &packet, sizeof(ProtocolCommands::RadioAudioPacket));
}

// ============================================================================
// BUSINESS LOGIC PROCESSING / RECEIVE PATH (RX)
// ============================================================================
void AudioHandler::processAudio(const ProtocolCommands::RadioAudioPacket& payload) {
    _last_rx_sequence = payload.sequence;
    _decoder_predicted = payload.init_predicted;
    _decoder_step = payload.init_step_index;

    // Scale dynamically to the block size
    int16_t decompressed_pcm[SAMPLE_BLOCK_LENGTH];
    decodePacket(&payload, decompressed_pcm);

    for (size_t i = 0; i < SAMPLE_BLOCK_LENGTH; i++) {
        _speaker.write(decompressed_pcm[i]);
    }
}

void AudioHandler::decodePacket(const ProtocolCommands::RadioAudioPacket* packet, int16_t* output_pcm) {
    size_t out_idx = 0;
    const size_t iterations = SAMPLE_BLOCK_LENGTH / 2;

    for (size_t i = 0; i < iterations; i++) {
        uint8_t byte = packet->data[i];
        output_pcm[out_idx++] = decodeSample((byte >> 4) & 0x0F);
        output_pcm[out_idx++] = decodeSample(byte & 0x0F);
    }
}

// ============================================================================
// MATHEMATICAL ADPCM CODEC ALGORITHMS
// ============================================================================
uint8_t AudioHandler::encodeSample(int16_t sample) {
    int32_t predicted = _encoder_predicted;
    int32_t step = STEP_TABLE[_encoder_step];
    int32_t diff = sample - predicted;
    
    uint8_t nibble = 0;
    if (diff < 0) {
        nibble = 8;
        diff = -diff;
    }
    
    int32_t temp_step = step;
    if (diff >= temp_step) {
        nibble |= 4;
        diff -= temp_step;
    }
    temp_step >>= 1;
    if (diff >= temp_step) {
        nibble |= 2;
        diff -= temp_step;
    }
    temp_step >>= 1;
    if (diff >= temp_step) {
        nibble |= 1;
    }
    
    int32_t diffq = step >> 3;
    if (nibble & 4) diffq += step;
    if (nibble & 2) diffq += (step >> 1);
    if (nibble & 1) diffq += (step >> 2);
    
    if (nibble & 8) _encoder_predicted -= diffq;
    else _encoder_predicted += diffq;
    
    if (_encoder_predicted > 32767) _encoder_predicted = 32767;
    else if (_encoder_predicted < -32768) _encoder_predicted = -32768;
    
    _encoder_step += INDEX_TABLE[nibble];
    if (_encoder_step < 0) _encoder_step = 0;
    else if (_encoder_step > 88) _encoder_step = 88;
    
    _encoder_predicted = (int16_t)_encoder_predicted;
    return nibble;
}

int16_t AudioHandler::decodeSample(uint8_t nibble) {
    int32_t step = STEP_TABLE[_decoder_step];
    
    int32_t diffq = step >> 3;
    if (nibble & 4) diffq += step;
    if (nibble & 2) diffq += (step >> 1);
    if (nibble & 1) diffq += (step >> 2);
    
    if (nibble & 8) {
        _decoder_predicted -= diffq;
    } else {
        _decoder_predicted += diffq;
    }
    
    if (_decoder_predicted > 32767) _decoder_predicted = 32767;
    else if (_decoder_predicted < -32768) _decoder_predicted = -32768;
    
    _decoder_step += INDEX_TABLE[nibble];
    if (_decoder_step < 0) _decoder_step = 0;
    else if (_decoder_step > 88) _decoder_step = 88;
    
    return (int16_t)_decoder_predicted;
}