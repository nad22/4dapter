/**
 * NESController.cpp - Nintendo Entertainment System Controller Implementation
 * 
 * Implements NES controller communication protocol and Power Pad support
 * 
 * Communication Protocol:
 * 1. Send 12μs latch pulse (high-low transition)
 * 2. Read 8 bits by clocking data line
 * 3. Each clock cycle is 6μs high, 6μs low
 * 4. Data is valid on clock falling edge
 * 
 * Power Pad Detection:
 * - Monitors D3 and D4 lines for additional button data
 * - Power Pad provides 12 buttons across two additional data lines
 * - Compatible with both Side A and Side B button layouts
 *
 * GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * 
 * Copyright (c) 2025 RETROdapter Project
 * Based on original 4dapter firmware
 */

#include "NESController.h"
#include <Arduino.h>

// Hardware pin definitions (ATmega32U4 specific)
#define NES_LATCH_PIN    2   // PD1
#define NES_CLOCK_PIN    3   // PD0  
#define NES_DATA_PIN     A0  // PF7
#define POWERPAD_D4_PIN  9   // PB5
#define POWERPAD_D3_PIN  8   // PB4

// Timing constants (in CPU cycles @ 16MHz)
#define LATCH_PULSE_CYCLES   192  // ~12μs
#define LATCH_LOW_CYCLES     72   // ~4.5μs
#define CLOCK_HIGH_CYCLES    96   // ~6μs  
#define CLOCK_LOW_CYCLES     72   // ~4.5μs

/**
 * Constructor - Initialize NES controller instance
 */
NESController::NESController() {
    controllerConnected = false;
    reset();
}

/**
 * Initialize NES controller hardware interface
 * Configures GPIO pins and sets initial states
 */
void NESController::init() {
    // Configure latch and clock pins as outputs (initially low)
    pinMode(NES_LATCH_PIN, OUTPUT);
    pinMode(NES_CLOCK_PIN, OUTPUT);
    digitalWrite(NES_LATCH_PIN, LOW);
    digitalWrite(NES_CLOCK_PIN, LOW);
    
    // Configure data pins as inputs with pull-up resistors
    pinMode(NES_DATA_PIN, INPUT_PULLUP);
    pinMode(POWERPAD_D4_PIN, INPUT_PULLUP);
    pinMode(POWERPAD_D3_PIN, INPUT_PULLUP);
    
    // Reset controller state
    reset();
}

/**
 * Read current NES controller state
 * Performs complete controller polling sequence
 * @return Updated controller state structure
 */
NESControllerState NESController::readController() {
    // Clear previous state
    currentState.standardButtons = 0;
    currentState.powerPadButtons = 0;
    currentState.powerPadConnected = false;
    
    // Send latch pulse to capture button states
    sendLatch();
    
    // Read 8 bits of standard controller data
    for (uint8_t bit = 0; bit < 8; bit++) {
        // Check standard NES controller data line
        if (!readDataBit()) {
            currentState.standardButtons |= (1 << bit);
        }
        
        // Check Power Pad D4 line (bottom row buttons)
        if (!readPowerPadD4()) {
            switch (bit) {
                case 0: currentState.powerPadButtons |= POWERPAD_BTN_4; break;
                case 1: currentState.powerPadButtons |= POWERPAD_BTN_3; break;
                case 2: currentState.powerPadButtons |= POWERPAD_BTN_12; break;
                case 3: currentState.powerPadButtons |= POWERPAD_BTN_8; break;
            }
        }
        
        // Check Power Pad D3 line (middle row buttons)
        if (!readPowerPadD3()) {
            switch (bit) {
                case 0: currentState.powerPadButtons |= POWERPAD_BTN_2; break;
                case 1: currentState.powerPadButtons |= POWERPAD_BTN_1; break;
                case 2: currentState.powerPadButtons |= POWERPAD_BTN_5; break;
                case 3: currentState.powerPadButtons |= POWERPAD_BTN_9; break;
                case 4: currentState.powerPadButtons |= POWERPAD_BTN_6; break;
                case 5: currentState.powerPadButtons |= POWERPAD_BTN_10; break;
                case 6: currentState.powerPadButtons |= POWERPAD_BTN_11; break;
                case 7: currentState.powerPadButtons |= POWERPAD_BTN_7; break;
            }
        }
        
        // Send clock pulse for next bit
        sendClock();
    }
    
    // Detect Power Pad connection
    currentState.powerPadConnected = detectPowerPad();
    
    // Update connection status based on response pattern
    controllerConnected = (currentState.standardButtons != 0xFF) || currentState.powerPadConnected;
    
    return currentState;
}

/**
 * Get most recent controller state
 * @return Current controller state without re-polling
 */
NESControllerState NESController::getState() {
    return currentState;
}

/**
 * Check controller connection status
 * @return True if controller is responding
 */
bool NESController::isConnected() {
    return controllerConnected;
}

/**
 * Check Power Pad connection status
 * @return True if Power Pad is detected
 */
bool NESController::isPowerPadConnected() {
    return currentState.powerPadConnected;
}

/**
 * Reset controller state to defaults
 * Clears all button states and connection flags
 */
void NESController::reset() {
    currentState.standardButtons = 0;
    currentState.powerPadButtons = 0;
    currentState.powerPadConnected = false;
    controllerConnected = false;
}

/**
 * Send latch pulse to NES controller
 * 12μs high pulse followed by 4.5μs low period
 * Private method for internal use
 */
void NESController::sendLatch() {
    // Set latch high
    PORTD |= B00000010;  // Set PD1 high
    __builtin_avr_delay_cycles(LATCH_PULSE_CYCLES);
    
    // Set latch low  
    PORTD &= ~B00000010; // Set PD1 low
    __builtin_avr_delay_cycles(LATCH_LOW_CYCLES);
}

/**
 * Send clock pulse to NES controller
 * 6μs high pulse followed by 4.5μs low period
 * Private method for internal use
 */
void NESController::sendClock() {
    // Set clock high
    PORTD |= B00000001;  // Set PD0 high
    __builtin_avr_delay_cycles(CLOCK_HIGH_CYCLES);
    
    // Set clock low
    PORTD &= ~B00000001; // Set PD0 low  
    __builtin_avr_delay_cycles(CLOCK_LOW_CYCLES);
}

/**
 * Read single bit from NES controller data line
 * @return True if data line is high, false if low (button pressed)
 * Private method for internal use
 */
bool NESController::readDataBit() {
    return (PINF & B10000000) != 0; // Read PF7 state
}

/**
 * Read Power Pad D4 data line
 * @return True if D4 line is high, false if low (button pressed)
 * Private method for internal use  
 */
bool NESController::readPowerPadD4() {
    return (PINB & B00100000) != 0; // Read PB5 state
}

/**
 * Read Power Pad D3 data line
 * @return True if D3 line is high, false if low (button pressed)
 * Private method for internal use
 */
bool NESController::readPowerPadD3() {
    return (PINB & B00010000) != 0; // Read PB4 state
}

/**
 * Detect Power Pad accessory presence
 * Checks for activity on D3 and D4 lines indicating Power Pad connection
 * @return True if Power Pad response patterns are detected
 * Private method for internal use
 */
bool NESController::detectPowerPad() {
    // Power Pad is detected if either D3 or D4 lines show activity
    // (any button pressed during polling sequence)
    return (currentState.powerPadButtons != 0);
}