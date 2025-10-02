/**
 * NESController.h - Nintendo Entertainment System Controller Interface
 * 
 * Handles NES controller communication including:
 * - Standard 8-button NES controller
 * - Nintendo Power Pad accessory support
 * - Proper timing and signal handling
 * 
 * Hardware Requirements:
 * - Latch Pin: Arduino Pin 2 (PD1) 
 * - Clock Pin: Arduino Pin 3 (PD0)
 * - Data Pin:  Arduino Pin A0 (PF7)
 * - Power Pad D4: Arduino Pin 9 (PB5) 
 * - Power Pad D3: Arduino Pin 8 (PB4)
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

#ifndef NES_CONTROLLER_H
#define NES_CONTROLLER_H

#include <stdint.h>

// NES Controller button bit definitions
#define NES_BTN_A       0x01
#define NES_BTN_B       0x02  
#define NES_BTN_SELECT  0x04
#define NES_BTN_START   0x08
#define NES_BTN_UP      0x10
#define NES_BTN_DOWN    0x20
#define NES_BTN_LEFT    0x40
#define NES_BTN_RIGHT   0x80

// Power Pad button definitions (12 buttons total)
#define POWERPAD_BTN_1   0x0001
#define POWERPAD_BTN_2   0x0002
#define POWERPAD_BTN_3   0x0004
#define POWERPAD_BTN_4   0x0008
#define POWERPAD_BTN_5   0x0010
#define POWERPAD_BTN_6   0x0020
#define POWERPAD_BTN_7   0x0040
#define POWERPAD_BTN_8   0x0080
#define POWERPAD_BTN_9   0x0100
#define POWERPAD_BTN_10  0x0200
#define POWERPAD_BTN_11  0x0400
#define POWERPAD_BTN_12  0x0800

/**
 * NES Controller State Structure
 * Contains button states for both standard controller and Power Pad
 */
typedef struct {
    uint8_t standardButtons;    // Standard 8 NES controller buttons
    uint16_t powerPadButtons;   // 12 Power Pad buttons (if connected)
    bool powerPadConnected;     // True if Power Pad is detected
} NESControllerState;

/**
 * NES Controller Class
 * Manages communication with NES controllers and Power Pad accessory
 */
class NESController {
public:
    /**
     * Constructor
     */
    NESController();
    
    /**
     * Initialize NES controller interface
     * Sets up GPIO pins and initial states
     */
    void init();
    
    /**
     * Read current controller state
     * Polls controller and updates internal state
     * @return Current controller state structure
     */
    NESControllerState readController();
    
    /**
     * Get last read controller state
     * @return Most recent controller state
     */
    NESControllerState getState();
    
    /**
     * Check if controller is connected
     * @return True if controller responds to polling
     */
    bool isConnected();
    
    /**
     * Check if Power Pad is connected
     * @return True if Power Pad accessory is detected
     */
    bool isPowerPadConnected();
    
    /**
     * Reset controller state
     * Clears all button states and connection flags
     */
    void reset();
    
private:
    NESControllerState currentState;    // Current controller state
    bool controllerConnected;           // Connection status flag
    
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
     * Read Power Pad D4 line (bottom row)
     * @return True if any D4 button is pressed
     */
    bool readPowerPadD4();
    
    /**
     * Read Power Pad D3 line (middle row)  
     * @return True if any D3 button is pressed
     */
    bool readPowerPadD3();
    
    /**
     * Detect Power Pad presence
     * Checks for Power Pad specific signal patterns
     * @return True if Power Pad is detected
     */
    bool detectPowerPad();
};

#endif // NES_CONTROLLER_H