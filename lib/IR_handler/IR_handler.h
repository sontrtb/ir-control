#ifndef IR_HANDLER_H
#define IR_HANDLER_H

#include <Arduino.h>
#include <vector>

#define DECODE_DENON
#define DECODE_JVC
#define DECODE_KASEIKYO
#define DECODE_PANASONIC
#define DECODE_LG
#define DECODE_NEC
#define DECODE_SAMSUNG
#define DECODE_SONY
#define DECODE_RC5
#define DECODE_RC6
#define DECODE_BOSEWAVE
#define DECODE_LEGO_PF
#define DECODE_MAGIQUEST
#define DECODE_WHYNTER
#define DECODE_FAST
//

#if !defined(RAW_BUFFER_LENGTH)
// For air condition remotes it requires 750. Default is 200.
#if !((defined(RAMEND) && RAMEND <= 0x4FF) || (defined(RAMSIZE) && RAMSIZE < 0x4FF))
#define RAW_BUFFER_LENGTH 750
#endif
#endif

// #define EXCLUDE_UNIVERSAL_PROTOCOLS // Saves up to 1000 bytes program memory.
// #define EXCLUDE_EXOTIC_PROTOCOLS // saves around 650 bytes program memory if all other protocols are active
// #define NO_LED_FEEDBACK_CODE      // saves 92 bytes program memory
#define RECORD_GAP_MICROS 12000 // Default is 8000. Activate it for some LG air conditioner protocols
// #define SEND_PWM_BY_TIMER         // Disable carrier PWM generation in software and use (restricted) hardware PWM.
// #define USE_NO_SEND_PWM           // Use no carrier PWM, just simulate an active low receiver signal. Overrides SEND_PWM_BY_TIMER definition

// MARK_EXCESS_MICROS is subtracted from all marks and added to all spaces before decoding,
// to compensate for the signal forming of different IR receiver modules. See also IRremote.hpp line 142.
#define MARK_EXCESS_MICROS 20


class IRHandler {
public:
    IRHandler(int receivePin, int sendPin);
    void begin();
    void sendCode(std::vector<uint8_t> rawData);
    void saveIRData();
    bool decode();

private:
    int receivePin;
    int sendPin;
};

extern IRHandler irHandler;
extern bool isReceiver;
extern String device;
extern String fuc;

#endif
