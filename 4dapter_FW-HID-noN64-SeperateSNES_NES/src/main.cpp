/*
 * RETROdapter 4dapter - Dual Controller Adapter Firmware
 * 
 * Multi-retro controller adapter supporting:
 * - NES Controller (Port 1)
 * - SNES Controller (Port 2) 
 * - Genesis/Mega Drive Controller (Port 3)
 *
 * Features:
 * - Three separate USB HID gamepad interfaces
 * - Nintendo Power Pad support on NES port
 * - SNES NTT Data Keypad support  
 * - Genesis 3-button and 6-button controller support
 * - MiSTer and RetroArch compatibility
 *
 * Hardware: Arduino Leonardo/Pro Micro (ATmega32U4)
 * 
 * GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "SegaController32U4.h"
#include "Gamepad.h"
#include "NESController.h"
#include "SNESController.h"

// USB device identification (max 20 chars including NULL terminator)
// Serial number differentiates projects for unique button mapping
const char *gp_serial = "4DAPTER_DUAL";

// Controller type definitions
#define NES_CONTROLLER       0
#define SNES_CONTROLLER      1  
#define GENESIS_CONTROLLER   2

// Data type definitions
#define BUTTONS              0
#define AXES                 1

// D-pad direction bit masks
#define UP                   0x01
#define DOWN                 0x02
#define LEFT                 0x04
#define RIGHT                0x08

// Special data indicators
#define NTT_BIT              0x00
#define NODATA               0x00

// Function prototypes
void sendState();
void initializeHardware();
void processNESController();
void processSNESController();
void processGenesisController();

/*
 * Hardware Pin Configuration (Arduino Leonardo/Pro Micro)
 * 
 * Controller DB9 pins (looking at plug face):
 * 5 4 3 2 1
 *  9 8 7 6
 *
 * Pin Mapping:
 * VCC                  - VCC (5V)
 * GND                  - GND
 * NES/SNES-LATCH       - Pin 2  (PD1)
 * NES/SNES-CLOCK       - Pin 3  (PD0)  
 * NES-Data1 (Pin 4)    - Pin A0 (PF7)
 * NES-DataD4 (Pin 5)   - Pin 9  (PB5) - Power Pad bottom row
 * NES-DataD3 (Pin 6)   - Pin 8  (PB4) - Power Pad middle row
 * SNES-Data1 (Pin 4)   - Pin A1 (PF6)
 * SNES-DataD2 (Pin 5)  - Pin RX (PD2) - NTT Data Keypad  
 * SNES-DataD3 (Pin 6)  - Pin TX (PD3) - NTT Data Keypad
 * Genesis DB9-1        - Pin 5  (PC6)
 * Genesis DB9-2        - Pin 6  (PD7) 
 * Genesis DB9-3        - Pin A2 (PF5)
 * Genesis DB9-4        - Pin A3 (PF4)
 * Genesis DB9-5        - Pin 16 (PB2) - Power (+5V)
 * Genesis DB9-6        - Pin 14 (PB3)
 * Genesis DB9-7        - Pin 7  (PE6)
 * Genesis DB9-8        - GND
 * Genesis DB9-9        - Pin 15 (PB1)
 */

/*
 * Nintendo Power Pad Button Layout
 * Reference: http://www.neshq.com/hardgen/powerpad.txt
 *
 * SIDE B Layout:
 * +--------------------------------+
 * |   1    2      3     4         |
 * |   5    6      7     8         |  
 * |   9   10     11    12         |
 * +--------------------------------+
 *
 * SIDE A Layout:  
 * +--------------------------------+
 * |        3       2              |
 * |  8  7       6     5           |
 * |       11      10              |
 * +--------------------------------+
 */

// EEPROM management for persistent settings
enum EEPROMIndices { 
    GENESIS_EEPROM = 0
};

// USB HID gamepad instances - one for each controller type
Gamepad_ Gamepad[3];

// Genesis controller instance with EEPROM storage
SegaController32U4 genesisController(GENESIS_EEPROM);

// NES controller instance  
NESController nesController;

// SNES controller instance
SNESController snesController;

// Controller data storage [controller_type][data_type]
uint32_t controllerData[3][2] = {{0,0}, {0,0}, {0,0}};

// Current Genesis controller state
uint16_t currentGenesisState = 0;

/**
 * Arduino setup function
 * Initializes hardware, controllers, and USB HID interfaces
 */
void setup() {
    // Initialize hardware pins and pull-up resistors
    initializeHardware();
    
    // Initialize controller instances
    nesController.init();
    snesController.init();
    
    // Small delay to ensure stable startup
    delay(250);
}

/**
 * Arduino main loop
 * Continuously polls all controllers and sends USB HID reports
 */
void loop() {
    while(true) {
        // Process Genesis controller (requires multiple cycles for 6-button detection)
        processGenesisController();
        
        // Process NES controller (includes Power Pad support)
        processNESController();
        
        // Process SNES controller (includes NTT Data Keypad support)  
        processSNESController();
        
        // Send all controller states via USB HID
        sendState();
    }
}

/**
 * Initialize hardware pins and configurations
 * Sets up GPIO pins, pull-up resistors, and initial states
 */
void initializeHardware() {
    // Setup NES/SNES latch and clock pins (2/3 or PD1/PD0)
    DDRD  |=  B00000011; // Set as outputs
    PORTD &= ~B00000011; // Set initial state low
    
    // Setup NES/SNES data pins (A0/A1 or PF6/PF7)  
    DDRF  &= ~B11000000; // Set as inputs
    PORTF |=  B11000000; // Enable internal pull-up resistors
    
    // Setup NES Power Pad data pins (8/9 or PB4/PB5)
    DDRB  &= ~B00110000; // Set as inputs
    PORTB |=  B00110000; // Enable internal pull-up resistors
    
    // Setup Genesis power pin (DB9 Pin 5) as output high (PB2)
    DDRB  |= B00000100; // Set as output
    PORTB |= B00000100; // Set high (+5V)
}

/**
 * Process Genesis/Mega Drive controller
 * Handles both 3-button and 6-button controller detection and mapping
 */
void processGenesisController() {
    // Genesis controllers require 8 cycles for proper 6-button detection
    for(uint8_t i = 0; i < 8; i++) {
        currentGenesisState = genesisController.updateState();
    }
    
    // Get final controller state after all cycles
    currentGenesisState = genesisController.getFinalState();
    
    // Map Genesis buttons to USB HID report (shift to remove direction bits)
    Gamepad[GENESIS_CONTROLLER]._GamepadReport.buttons = currentGenesisState >> 4;
    
    // Map Genesis D-pad to analog axes
    if (((currentGenesisState & SC_BTN_DOWN) >> SC_BIT_SH_DOWN)) {
        Gamepad[GENESIS_CONTROLLER]._GamepadReport.Y = 0x7F;      // Down
    } else if (((currentGenesisState & SC_BTN_UP) >> SC_BIT_SH_UP)) {
        Gamepad[GENESIS_CONTROLLER]._GamepadReport.Y = 0x80;      // Up  
    } else {
        Gamepad[GENESIS_CONTROLLER]._GamepadReport.Y = 0;         // Center
    }
    
    if (((currentGenesisState & SC_BTN_RIGHT) >> SC_BIT_SH_RIGHT)) {
        Gamepad[GENESIS_CONTROLLER]._GamepadReport.X = 0x7F;      // Right
    } else if (((currentGenesisState & SC_BTN_LEFT) >> SC_BIT_SH_LEFT)) {
        Gamepad[GENESIS_CONTROLLER]._GamepadReport.X = 0x80;      // Left
    } else {
        Gamepad[GENESIS_CONTROLLER]._GamepadReport.X = 0;         // Center
    }
}

/**
 * Process NES controller and Power Pad
 * Reads standard NES controller plus Nintendo Power Pad accessory
 */
void processNESController() {
    // Read current NES controller state
    NESControllerState nesState = nesController.readController();
    
    // Clear controller data
    controllerData[NES_CONTROLLER][BUTTONS] = 0;
    controllerData[NES_CONTROLLER][AXES] = 0;
    
    // Map NES standard buttons
    if (nesState.standardButtons & NES_BTN_A)      controllerData[NES_CONTROLLER][BUTTONS] |= 0x02;
    if (nesState.standardButtons & NES_BTN_B)      controllerData[NES_CONTROLLER][BUTTONS] |= 0x01;
    if (nesState.standardButtons & NES_BTN_START)  controllerData[NES_CONTROLLER][BUTTONS] |= 0x40;
    if (nesState.standardButtons & NES_BTN_SELECT) controllerData[NES_CONTROLLER][BUTTONS] |= 0x80;
    
    // Map NES D-pad to axes
    if (nesState.standardButtons & NES_BTN_UP)     controllerData[NES_CONTROLLER][AXES] |= UP;
    if (nesState.standardButtons & NES_BTN_DOWN)   controllerData[NES_CONTROLLER][AXES] |= DOWN;
    if (nesState.standardButtons & NES_BTN_LEFT)   controllerData[NES_CONTROLLER][AXES] |= LEFT;
    if (nesState.standardButtons & NES_BTN_RIGHT)  controllerData[NES_CONTROLLER][AXES] |= RIGHT;
    
    // Map Power Pad buttons if connected
    if (nesState.powerPadConnected) {
        controllerData[NES_CONTROLLER][BUTTONS] |= nesState.powerPadButtons;
    }
    
    // Update USB HID report for NES controller
    Gamepad[NES_CONTROLLER]._GamepadReport.buttons = controllerData[NES_CONTROLLER][BUTTONS];
    
    // Map NES D-pad to analog axes
    if (controllerData[NES_CONTROLLER][AXES] & DOWN) {
        Gamepad[NES_CONTROLLER]._GamepadReport.Y = 0x7F;          // Down
    } else if (controllerData[NES_CONTROLLER][AXES] & UP) {
        Gamepad[NES_CONTROLLER]._GamepadReport.Y = 0x80;          // Up
    } else {
        Gamepad[NES_CONTROLLER]._GamepadReport.Y = 0;             // Center
    }
    
    if (controllerData[NES_CONTROLLER][AXES] & RIGHT) {
        Gamepad[NES_CONTROLLER]._GamepadReport.X = 0x7F;          // Right  
    } else if (controllerData[NES_CONTROLLER][AXES] & LEFT) {
        Gamepad[NES_CONTROLLER]._GamepadReport.X = 0x80;          // Left
    } else {
        Gamepad[NES_CONTROLLER]._GamepadReport.X = 0;             // Center
    }
}

/**
 * Process SNES controller and NTT Data Keypad
 * Reads standard SNES controller plus optional NTT Data Keypad accessory
 */
void processSNESController() {
    // Read current SNES controller state
    SNESControllerState snesState = snesController.readController();
    
    // Clear controller data
    controllerData[SNES_CONTROLLER][BUTTONS] = 0;
    controllerData[SNES_CONTROLLER][AXES] = 0;
    
    // Map SNES standard buttons
    if (snesState.standardButtons & SNES_BTN_B)      controllerData[SNES_CONTROLLER][BUTTONS] |= 0x01;
    if (snesState.standardButtons & SNES_BTN_Y)      controllerData[SNES_CONTROLLER][BUTTONS] |= 0x04;
    if (snesState.standardButtons & SNES_BTN_A)      controllerData[SNES_CONTROLLER][BUTTONS] |= 0x02;
    if (snesState.standardButtons & SNES_BTN_X)      controllerData[SNES_CONTROLLER][BUTTONS] |= 0x08;
    if (snesState.standardButtons & SNES_BTN_L)      controllerData[SNES_CONTROLLER][BUTTONS] |= 0x10;
    if (snesState.standardButtons & SNES_BTN_R)      controllerData[SNES_CONTROLLER][BUTTONS] |= 0x20;
    if (snesState.standardButtons & SNES_BTN_START)  controllerData[SNES_CONTROLLER][BUTTONS] |= 0x40;
    if (snesState.standardButtons & SNES_BTN_SELECT) controllerData[SNES_CONTROLLER][BUTTONS] |= 0x80;
    
    // Map SNES D-pad to axes
    if (snesState.standardButtons & SNES_BTN_UP)     controllerData[SNES_CONTROLLER][AXES] |= UP;
    if (snesState.standardButtons & SNES_BTN_DOWN)   controllerData[SNES_CONTROLLER][AXES] |= DOWN;
    if (snesState.standardButtons & SNES_BTN_LEFT)   controllerData[SNES_CONTROLLER][AXES] |= LEFT;
    if (snesState.standardButtons & SNES_BTN_RIGHT)  controllerData[SNES_CONTROLLER][AXES] |= RIGHT;
    
    // Map NTT keypad buttons if connected
    if (snesState.nttConnected) {
        // Map NTT numeric and special keys to additional button bits
        controllerData[SNES_CONTROLLER][BUTTONS] |= (snesState.nttKeypad & 0xFFFF);
        // Extended NTT buttons use higher bits
        controllerData[SNES_CONTROLLER][BUTTONS] |= ((snesState.nttKeypad >> 16) & 0xFF) << 16;
    }
    
    // Update USB HID report for SNES controller
    Gamepad[SNES_CONTROLLER]._GamepadReport.buttons = controllerData[SNES_CONTROLLER][BUTTONS];
    
    // Map SNES D-pad to analog axes
    if (controllerData[SNES_CONTROLLER][AXES] & DOWN) {
        Gamepad[SNES_CONTROLLER]._GamepadReport.Y = 0x7F;         // Down
    } else if (controllerData[SNES_CONTROLLER][AXES] & UP) {
        Gamepad[SNES_CONTROLLER]._GamepadReport.Y = 0x80;         // Up  
    } else {
        Gamepad[SNES_CONTROLLER]._GamepadReport.Y = 0;            // Center
    }
    
    if (controllerData[SNES_CONTROLLER][AXES] & RIGHT) {
        Gamepad[SNES_CONTROLLER]._GamepadReport.X = 0x7F;         // Right
    } else if (controllerData[SNES_CONTROLLER][AXES] & LEFT) {
        Gamepad[SNES_CONTROLLER]._GamepadReport.X = 0x80;         // Left
    } else {
        Gamepad[SNES_CONTROLLER]._GamepadReport.X = 0;            // Center
    }
}



/**
 * Send USB HID reports for all controllers
 * Transmits current controller states to host via USB
 */
void sendState() {
    Gamepad[NES_CONTROLLER].send();
    Gamepad[SNES_CONTROLLER].send();  
    Gamepad[GENESIS_CONTROLLER].send();
    __builtin_avr_delay_cycles(16000);
}