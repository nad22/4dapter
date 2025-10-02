/**
 * SNESController.h - Super Nintendo Entertainment System Controller Interface
 * 
 * Handles SNES controller communication including:
 * - Standard 12-button SNES controller
 * - NTT Data Keypad accessory support (Japanese telephone keypad)
 * - Multi-tap support (future expansion)
 * - Proper timing and extended polling sequences
 * 
 * Hardware Requirements:
 * - Latch Pin: Arduino Pin 2 (PD1) - shared with NES
 * - Clock Pin: Arduino Pin 3 (PD0) - shared with NES  
 * - Data Pin:  Arduino Pin A1 (PF6)
 * - NTT D2:    Arduino Pin RX/0 (PD2) - NTT Data Keypad
 * - NTT D3:    Arduino Pin TX/1 (PD3) - NTT Data Keypad
 *
 * NTT Data Keypad:
 * The NTT Data Keypad was a Japanese accessory providing numeric input
 * for telephone number entry and special applications. It extends the
 * standard 16-bit SNES protocol to 32 bits with keypad data.
 *
 * GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * 
 * Copyright (c) 2025 RETROdapter Project
 * Based on original 4dapter firmware
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef SNES_CONTROLLER_H
#define SNES_CONTROLLER_H

#include <stdint.h>

// SNES Controller button bit definitions
#define SNES_BTN_B       0x0001
#define SNES_BTN_Y       0x0002
#define SNES_BTN_SELECT  0x0004
#define SNES_BTN_START   0x0008
#define SNES_BTN_UP      0x0010
#define SNES_BTN_DOWN    0x0020
#define SNES_BTN_LEFT    0x0040
#define SNES_BTN_RIGHT   0x0080
#define SNES_BTN_A       0x0100
#define SNES_BTN_X       0x0200
#define SNES_BTN_L       0x0400
#define SNES_BTN_R       0x0800

// NTT Data Keypad button definitions
#define NTT_KEY_0        0x00010000
#define NTT_KEY_1        0x00020000
#define NTT_KEY_2        0x00040000
#define NTT_KEY_3        0x00080000
#define NTT_KEY_4        0x00100000
#define NTT_KEY_5        0x00200000
#define NTT_KEY_6        0x00400000
#define NTT_KEY_7        0x00800000
#define NTT_KEY_8        0x01000000
#define NTT_KEY_9        0x02000000
#define NTT_KEY_STAR     0x04000000  // * key
#define NTT_KEY_HASH     0x08000000  // # key
#define NTT_KEY_DOT      0x10000000  // . key
#define NTT_KEY_CLEAR    0x20000000  // C key
#define NTT_KEY_END      0x80000000  // End communication

// SNES protocol timing constants
#define SNES_STANDARD_BITS  16    // Standard SNES controller bits
#define SNES_EXTENDED_BITS  32    // Extended for NTT Data Keypad

/**
 * SNES Controller State Structure
 * Contains button states for both standard controller and NTT keypad
 */
typedef struct {
    uint16_t standardButtons;   // Standard 12 SNES controller buttons
    uint32_t nttKeypad;        // NTT Data Keypad buttons (if connected)
    bool nttConnected;         // True if NTT Data Keypad is detected
    uint8_t playerNumber;      // Multi-tap player number (1-4, future use)
} SNESControllerState;

/**
 * SNES Controller Class
 * Manages communication with SNES controllers and NTT Data Keypad
 */
class SNESController {
public:
    /**
     * Constructor
     */
    SNESController();
    
    /**
     * Initialize SNES controller interface
     * Sets up GPIO pins and initial states
     */
    void init();
    
    /**
     * Read current controller state
     * Polls controller and updates internal state
     * @return Current controller state structure
     */
    SNESControllerState readController();
    
    /**
     * Get last read controller state
     * @return Most recent controller state
     */
    SNESControllerState getState();
    
    /**
     * Check if controller is connected
     * @return True if controller responds to polling
     */
    bool isConnected();
    
    /**
     * Check if NTT Data Keypad is connected
     * @return True if NTT keypad accessory is detected
     */
    bool isNTTConnected();
    
    /**
     * Reset controller state
     * Clears all button states and connection flags
     */
    void reset();
    
    /**
     * Set multi-tap player number (future expansion)
     * @param player Player number (1-4)
     */
    void setPlayerNumber(uint8_t player);
    
private:
    SNESControllerState currentState;   // Current controller state
    bool controllerConnected;           // Connection status flag
    uint8_t currentPlayer;              // Current player number for multi-tap
    
    /**
     * Send latch pulse to controller
     * Signals controller to latch current button states
     */
    void sendLatch();
    
    /**
     * Send clock pulse to controller
     * Advances controller shift register
     */
    void sendClock();
    
    /**
     * Read single bit from controller data line
     * @return True if data line is low (button pressed)
     */
    bool readDataBit();
    
    /**
     * Read NTT keypad data lines
     * Checks D2 and D3 lines for keypad input
     * @param bit Current bit position in polling sequence
     * @return NTT keypad button mask for current bit
     */
    uint32_t readNTTKeypad(uint8_t bit);
    
    /**
     * Detect NTT Data Keypad presence
     * Checks for NTT keypad indicator bit during polling
     * @return True if NTT keypad is detected
     */
    bool detectNTTKeypad();
    
    /**
     * Process extended NTT polling sequence
     * Continues polling beyond standard 16 bits for NTT data
     * @param startBit Starting bit position (typically 16)
     */
    void processNTTExtended(uint8_t startBit);
};

#endif // SNES_CONTROLLER_H