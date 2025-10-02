/**
 * SNESController.cpp - Super Nintendo Entertainment System Controller Implementation
 * 
 * Implements SNES controller communication protocol and NTT Data Keypad support
 * 
 * Communication Protocol:
 * 1. Send 12μs latch pulse (high-low transition)
 * 2. Read 16 bits by clocking data line (standard controller)
 * 3. Extended to 32 bits if NTT Data Keypad is detected
 * 4. Each clock cycle is 6μs high, 6μs low
 * 5. Data is valid on clock falling edge
 * 
 * NTT Data Keypad Protocol:
 * - Bit 13 indicates NTT presence when low during standard polling
 * - Extended polling from bit 16-31 reads keypad data
 * - Supports numeric keys 0-9 plus *, #, ., C, and End
 * - Originally designed for telephone dialing applications
 *
 * GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * 
 * Copyright (c) 2025 RETROdapter Project
 * Based on original 4dapter firmware
 */

#include "SNESController.h"
#include <Arduino.h>

// Hardware pin definitions (ATmega32U4 specific)
#define SNES_LATCH_PIN   2   // PD1 (shared with NES)
#define SNES_CLOCK_PIN   3   // PD0 (shared with NES)
#define SNES_DATA_PIN    A1  // PF6
#define NTT_D2_PIN       0   // PD2 (RX pin)
#define NTT_D3_PIN       1   // PD3 (TX pin)

// Timing constants (in CPU cycles @ 16MHz)
#define LATCH_PULSE_CYCLES   192  // ~12μs
#define LATCH_LOW_CYCLES     72   // ~4.5μs  
#define CLOCK_HIGH_CYCLES    96   // ~6μs
#define CLOCK_LOW_CYCLES     72   // ~4.5μs

// Protocol constants
#define NTT_INDICATOR_BIT    13   // Bit position that indicates NTT presence

/**
 * Constructor - Initialize SNES controller instance
 */
SNESController::SNESController() {
    controllerConnected = false;
    currentPlayer = 1;
    reset();
}

/**
 * Initialize SNES controller hardware interface
 * Configures GPIO pins and sets initial states
 */
void SNESController::init() {
    // Configure latch and clock pins as outputs (initially low)
    // Note: These are shared with NES controller
    pinMode(SNES_LATCH_PIN, OUTPUT);
    pinMode(SNES_CLOCK_PIN, OUTPUT);
    digitalWrite(SNES_LATCH_PIN, LOW);
    digitalWrite(SNES_CLOCK_PIN, LOW);
    
    // Configure data pins as inputs with pull-up resistors
    pinMode(SNES_DATA_PIN, INPUT_PULLUP);
    pinMode(NTT_D2_PIN, INPUT_PULLUP);
    pinMode(NTT_D3_PIN, INPUT_PULLUP);
    
    // Reset controller state
    reset();
}

/**
 * Read current SNES controller state
 * Performs complete controller polling sequence including NTT detection
 * @return Updated controller state structure
 */
SNESControllerState SNESController::readController() {
    // Clear previous state
    currentState.standardButtons = 0;
    currentState.nttKeypad = 0;
    currentState.nttConnected = false;
    
    // Send latch pulse to capture button states
    sendLatch();
    
    // Read standard 16 bits of SNES controller data
    for (uint8_t bit = 0; bit < SNES_STANDARD_BITS; bit++) {
        // Check standard SNES controller data line
        if (!readDataBit()) {
            currentState.standardButtons |= (1 << bit);
        }
        
        // Check for NTT indicator bit
        if (bit == NTT_INDICATOR_BIT && !readDataBit()) {
            currentState.nttConnected = true;
        }
        
        // Send clock pulse for next bit
        sendClock();
    }
    
    // If NTT keypad detected, continue polling for extended data
    if (currentState.nttConnected) {
        processNTTExtended(SNES_STANDARD_BITS);
    }
    
    // Update connection status based on response pattern
    controllerConnected = (currentState.standardButtons != 0xFFFF) || currentState.nttConnected;
    
    return currentState;
}

/**
 * Get most recent controller state
 * @return Current controller state without re-polling
 */
SNESControllerState SNESController::getState() {
    return currentState;
}

/**
 * Check controller connection status
 * @return True if controller is responding
 */
bool SNESController::isConnected() {
    return controllerConnected;
}

/**
 * Check NTT Data Keypad connection status
 * @return True if NTT keypad is detected
 */
bool SNESController::isNTTConnected() {
    return currentState.nttConnected;
}

/**
 * Reset controller state to defaults
 * Clears all button states and connection flags
 */
void SNESController::reset() {
    currentState.standardButtons = 0;
    currentState.nttKeypad = 0;
    currentState.nttConnected = false;
    currentState.playerNumber = currentPlayer;
    controllerConnected = false;
}

/**
 * Set multi-tap player number for future expansion
 * @param player Player number (1-4)
 */
void SNESController::setPlayerNumber(uint8_t player) {
    if (player >= 1 && player <= 4) {
        currentPlayer = player;
        currentState.playerNumber = player;
    }
}

/**
 * Send latch pulse to SNES controller
 * 12μs high pulse followed by 4.5μs low period
 * Private method for internal use
 */
void SNESController::sendLatch() {
    // Set latch high
    PORTD |= B00000010;  // Set PD1 high
    __builtin_avr_delay_cycles(LATCH_PULSE_CYCLES);
    
    // Set latch low
    PORTD &= ~B00000010; // Set PD1 low
    __builtin_avr_delay_cycles(LATCH_LOW_CYCLES);
}

/**
 * Send clock pulse to SNES controller
 * 6μs high pulse followed by 4.5μs low period  
 * Private method for internal use
 */
void SNESController::sendClock() {
    // Set clock high
    PORTD |= B00000001;  // Set PD0 high
    __builtin_avr_delay_cycles(CLOCK_HIGH_CYCLES);
    
    // Set clock low
    PORTD &= ~B00000001; // Set PD0 low
    __builtin_avr_delay_cycles(CLOCK_LOW_CYCLES);
}

/**
 * Read single bit from SNES controller data line
 * @return True if data line is high, false if low (button pressed)
 * Private method for internal use
 */
bool SNESController::readDataBit() {
    return (PINF & B01000000) != 0; // Read PF6 state
}

/**
 * Read NTT keypad data from D2 and D3 lines
 * Processes keypad input during extended polling sequence
 * @param bit Current bit position in extended sequence
 * @return NTT keypad button mask for current bit
 * Private method for internal use
 */
uint32_t SNESController::readNTTKeypad(uint8_t bit) {
    uint32_t keypadData = 0;
    
    // Check NTT D2 line (PD2)
    if ((PIND & B00000100) == 0) {
        // Map bit position to NTT keypad buttons
        switch (bit - SNES_STANDARD_BITS) {
            case 0:  keypadData |= NTT_KEY_0; break;
            case 1:  keypadData |= NTT_KEY_1; break;
            case 2:  keypadData |= NTT_KEY_2; break;
            case 3:  keypadData |= NTT_KEY_3; break;
            case 4:  keypadData |= NTT_KEY_4; break;
            case 5:  keypadData |= NTT_KEY_5; break;
            case 6:  keypadData |= NTT_KEY_6; break;
            case 7:  keypadData |= NTT_KEY_7; break;
            case 8:  keypadData |= NTT_KEY_8; break;
            case 9:  keypadData |= NTT_KEY_9; break;
            case 10: keypadData |= NTT_KEY_STAR; break;
            case 11: keypadData |= NTT_KEY_HASH; break;
            case 12: keypadData |= NTT_KEY_DOT; break;
            case 13: keypadData |= NTT_KEY_CLEAR; break;
            case 15: keypadData |= NTT_KEY_END; break;
        }
    }
    
    // Check NTT D3 line (PD3) for additional functionality
    if ((PIND & B00001000) == 0) {
        // D3 can provide additional keypad functions or status
        // Implementation depends on specific NTT keypad variant
    }
    
    return keypadData;
}

/**
 * Detect NTT Data Keypad presence
 * Checks for NTT indicator bit during standard polling
 * @return True if NTT keypad is detected
 * Private method for internal use
 */
bool SNESController::detectNTTKeypad() {
    return currentState.nttConnected;
}

/**
 * Process extended NTT polling sequence
 * Continues polling beyond standard 16 bits for NTT keypad data
 * @param startBit Starting bit position (typically 16)
 * Private method for internal use
 */
void SNESController::processNTTExtended(uint8_t startBit) {
    // Poll additional 16 bits for NTT keypad data
    for (uint8_t bit = startBit; bit < SNES_EXTENDED_BITS; bit++) {
        // Read NTT keypad data from D2 and D3 lines
        currentState.nttKeypad |= readNTTKeypad(bit);
        
        // Send clock pulse for next bit
        sendClock();
    }
}