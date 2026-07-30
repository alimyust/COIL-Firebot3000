// test/test_mic_diagnostics/test_mic_diagnostics.cpp
//
// Hardware-in-the-loop diagnostic for the Microphone TC4->EVSYS->ADC->DMA
// trigger chain. This is NOT a native/desktop test - it pokes real SAMD21
// registers, so it must run ON the Feather M0 board:
//
//   pio test -e <your_env_name>
//
// Each test isolates one link in the chain. Run them in order - the first
// one that fails tells you exactly where the pipeline is breaking. Later
// tests are only meaningful if the earlier ones pass (e.g. there's no point
// checking "ADC auto-triggers" if TC4 itself never counted).
//
// NOTE: exact register/flag macro names below match the ASF/CMSIS headers
// used by the Arduino/Adafruit SAMD core as of this writing. If your core
// version renamed something, the compiler error will point you straight
// at it - the *structure* of the test is what matters.

#include <Arduino.h>
#include <unity.h>
#include "Microphone.hpp"

static Microphone mic;

void setUp(void) {}
void tearDown(void) {}

// ---------------------------------------------------------------------
// Stage 1: Is TC4 clocked, enabled, and actually counting at all?
// If this fails: GCLK_CLKCTRL_ID_TC4_TC5 isn't reaching TC4, or
// PM->APBCMASK.TC4 is off, or CTRLA.ENABLE never took (check SYNCBUSY).
// ---------------------------------------------------------------------
void test_tc4_is_counting(void) {
    TEST_ASSERT_MESSAGE(TC4->COUNT16.CTRLA.bit.ENABLE, "TC4 CTRLA.ENABLE is not set");

    uint16_t c1 = TC4->COUNT16.COUNT.reg;
    delayMicroseconds(500);
    uint16_t c2 = TC4->COUNT16.COUNT.reg;

    char msg[96];
    snprintf(msg, sizeof(msg), "TC4 COUNT not changing (c1=%u c2=%u) - timer isn't clocked/running", c1, c2);
    TEST_ASSERT_MESSAGE(c1 != c2, msg);
}

// ---------------------------------------------------------------------
// Stage 2: Does TC4 actually reach its CC0 match periodically?
// This is independent of EVSYS - it just confirms the timer's own
// match logic fires. If Stage 1 passes but this fails: CC[0] value,
// WAVEGEN mode, or prescaler is wrong (won't ever reach 374, or wraps
// without matching).
// ---------------------------------------------------------------------
void test_tc4_match_fires(void) {
    TC4->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0; // write-1-to-clear
    uint32_t start = millis();
    bool fired = false;
    while (millis() - start < 5) {
        if (TC4->COUNT16.INTFLAG.bit.MC0) { fired = true; break; }
    }
    TEST_ASSERT_MESSAGE(fired, "TC4 MC0 compare match never set within 5ms - expected ~40 matches at 8kHz");
}

// ---------------------------------------------------------------------
// Stage 3: Is the EVSYS channel actually carrying events?
// CHSTATUS.CHBUSYx pulses busy whenever an event is presently being
// routed through that channel. If Stage 2 passes but this fails:
// EVCTRL.MCEO0 isn't set on TC4, or the EVSYS_CHANNEL_EVGEN id is
// wrong, or CHANNEL/USER routing doesn't actually match.
// ---------------------------------------------------------------------
void test_evsys_channel_busy(void) {
    uint32_t start = millis();
    bool sawBusy = false;
    while (millis() - start < 5) {
        if (EVSYS->CHSTATUS.reg & EVSYS_CHSTATUS_CHBUSY0) { sawBusy = true; break; }
    }
    TEST_ASSERT_MESSAGE(sawBusy, "EVSYS channel 0 never went busy - USER_CHANNEL / EVGEN routing is wrong");
}

// ---------------------------------------------------------------------
// Stage 4: Is the ADC actually being auto-triggered by the event
// (as opposed to just the one manual SWTRIG at init)?
// If Stage 3 passes but this fails: ADC.EVCTRL.STARTEI isn't set,
// or EVSYS_ID_USER_ADC_START mapping / USER.reg write is wrong.
// ---------------------------------------------------------------------
void test_adc_auto_triggers(void) {
    TEST_ASSERT_MESSAGE(ADC->EVCTRL.bit.STARTEI, "ADC EVCTRL.STARTEI is not set");

    ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY; // write-1-to-clear
    uint32_t start = millis();
    bool fired = false;
    while (millis() - start < 5) {
        if (ADC->INTFLAG.bit.RESRDY) { fired = true; break; }
    }
    TEST_ASSERT_MESSAGE(fired, "ADC RESRDY never set on its own - EVSYS event isn't reaching the ADC start trigger");
}

// ---------------------------------------------------------------------
// Stage 5: End to end - does a full double-buffer actually complete?
// 64 samples @ 8kHz = 8ms nominal per half-buffer; give generous margin.
// If everything above passes but this fails: look at the DMA
// descriptor config itself (BEATSIZE, BLOCKACT, trigger source,
// looping) rather than the timing chain.
// ---------------------------------------------------------------------
void test_dma_buffer_fills(void) {
    uint32_t start = millis();
    while (millis() - start < 50) {
        if (mic.isBufferReady()) break;
    }
    TEST_ASSERT_MESSAGE(mic.isBufferReady(),
        "adc_buffer_filled never became true within 50ms (~4 expected blocks at 8kHz)");
}

void setup() {
    delay(2000); // let USB/Serial settle before UNITY output
    UNITY_BEGIN();

    mic.begin();
    delay(5); // let a handful of TC4 cycles run before probing anything

    RUN_TEST(test_tc4_is_counting);
    RUN_TEST(test_tc4_match_fires);
    RUN_TEST(test_evsys_channel_busy);
    RUN_TEST(test_adc_auto_triggers);
    RUN_TEST(test_dma_buffer_fills);

    UNITY_END();
}

void loop() {
    // nothing - all work happens once in setup() for this diagnostic
}