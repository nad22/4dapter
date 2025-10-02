# RETROdapter 4dapter - Dual Controller Adapter# 4dapter - HID Firmware for MiSTer / PC



A multi-retro controller adapter firmware supporting NES, SNES, and Genesis/Mega Drive controllers with modern USB HID compatibility.The default HID firmware will allow the 4dapter to appear a multiplayer input controller with 3 controller inputs acting as it's own separate player/input (NES and SNES are combined due to Arduino USB endpoint limitations.)



## Overview## MiSTer - Define Joystick Buttons (Mapping)



The RETROdapter 4dapter is a PlatformIO-based firmware project that converts vintage game controllers into modern USB HID gamepads. This version supports three separate controller types as individual USB devices, providing optimal compatibility with MiSTer FPGA, RetroArch, and modern gaming systems.### Via .map File

For maximum compatibly, install the MiSTer controller Map file found in the [MiSTer Maps Folder](https://github.com/timville85/4dapter/tree/main/MiSTer%20Maps) to your `/media/fat/config/inputs` directory on your MiSTer SD card and reboot your MiSTer. After doing this, you'll need to map the N64 controller in the N64 core for all buttons to work. The SNES / Genesis / NES cores will already be properly configured via the Map file, so do not map the individual cores or else conflicts may occur.

## Supported Controllers

### Manual Mapping 

### ✅ Nintendo Entertainment System (NES)If manually mapping in the MiSTer main menu, use the N64 controller using the following steps and NES / SNES / Genesis will be appropriately mapped for their cores:

- **Standard NES Controller**: 8-button layout (A, B, Start, Select, D-Pad)```

- **Nintendo Power Pad**: 12-button exercise mat accessoryDPAD Test: Press RIGHT     ---  D-Right

- **Dedicated USB Interface**: Reports as Player 1Stick 1 Test: Tilt RIGHT   ---  Analog Stick Right

Stick 1 Test: Tilt DOWN    ---  Analog Stick Down

### ✅ Super Nintendo Entertainment System (SNES) Stick 2 Test: Tilt RIGHT   ---  Undefine (User / Space to Skip)

- **Standard SNES Controller**: 12-button layout (A, B, X, Y, L, R, Start, Select, D-Pad)Press: RIGHT               ---  Analog Stick Right

- **NTT Data Keypad**: Japanese telephone keypad accessory (0-9, *, #, ., C)Press: LEFT                ---  Analog Stick Left

- **Dedicated USB Interface**: Reports as Player 2Press: DOWN                ---  Analog Stick Down

Press: UP                  ---  Analog Stick Up

### ✅ Sega Genesis/Mega DrivePress: A                   ---  A Button

- **3-Button Controllers**: Classic Genesis pad (A, B, C, Start, D-Pad)Press: B                   ---  B Button

- **6-Button Controllers**: Full Genesis pad (A, B, C, X, Y, Z, Mode, Start, D-Pad)Press: X                   ---  C-Down Button

- **MiSTer Mode**: Button remapping for consistent cross-platform layoutPress: Y                   ---  C-Left Button

- **Dedicated USB Interface**: Reports as Player 3Press: L                   ---  Left Bumper Button

Press: R                   ---  Right Bumper Button

### ❌ Removed SupportPress: Select              ---  C-Right Button

- **Nintendo 64**: Completely removed from this version for simplified designPress: Start               ---  Start Button

Press: Mouse Move RIGHT    ---  Undefine (User / Space to Skip)

## Hardware RequirementsPress: Mouse Move LEFT     ---  Undefine (User / Space to Skip)

### Microcontroller
- **Arduino Leonardo** or **Arduino Pro Micro** (ATmega32U4 based)
- **USB HID Support**: Native USB capability required
- **16MHz Clock**: Standard Arduino clock speed

### Pin Configuration

| Function | Arduino Pin | ATmega32U4 | Description |
|----------|-------------|------------|-------------|
| **Shared NES/SNES** | | | |
| Latch | 2 | PD1 | Controller latch signal |
| Clock | 3 | PD0 | Controller clock signal |
| **NES Specific** | | | |
| Data | A0 | PF7 | NES controller data |
| Power Pad D4 | 9 | PB5 | Power Pad bottom row |
| Power Pad D3 | 8 | PB4 | Power Pad middle row |
| **SNES Specific** | | | |
| Data | A1 | PF6 | SNES controller data |
| NTT D2 | 0 (RX) | PD2 | NTT keypad data |
| NTT D3 | 1 (TX) | PD3 | NTT keypad data |
| **Genesis/Mega Drive** | | | |
| DB9-1 | 5 | PC6 | Genesis pin 1 |
| DB9-2 | 6 | PD7 | Genesis pin 2 |
| DB9-3 | A2 | PF5 | Genesis pin 3 |
| DB9-4 | A3 | PF4 | Genesis pin 4 |
| DB9-5 | 16 | PB2 | Genesis +5V power |
| DB9-6 | 14 | PB3 | Genesis pin 6 |
| DB9-7 | 7 | PE6 | Genesis pin 7 |
| DB9-9 | 15 | PB1 | Genesis pin 9 |

### Hardware Wiring Details

#### Shared Clock/Latch Design
The **NES and SNES controllers share Clock and Latch signals** for optimal pin efficiency:

```
SHARED SIGNALS (NES + SNES):
├── Pin 2 (PD1) ─────┬─── NES Controller Pin 2 (Latch)
│                    └─── SNES Controller Pin 2 (Latch)
└── Pin 3 (PD0) ─────┬─── NES Controller Pin 3 (Clock)
                     └─── SNES Controller Pin 3 (Clock)

SEPARATE DATA LINES:
├── Pin A0 (PF7) ────── NES Controller Pin 4 (Data)
└── Pin A1 (PF6) ────── SNES Controller Pin 4 (Data)
```

#### Complete Wiring Diagram
```
Arduino Leonardo/Pro Micro Connections:

POWER DISTRIBUTION:
├── VCC (5V) ────────┬─── NES Controller Pin 1
│                    ├─── SNES Controller Pin 1
│                    └─── Genesis DB9-5 (via Pin 16/PB2)
└── GND ─────────────┬─── NES Controller Pin 8
                     ├─── SNES Controller Pin 8
                     └─── Genesis DB9-8

NES CONTROLLER (7-pin connector):
├── Pin 1: VCC (+5V)
├── Pin 2: Latch ─────── Arduino Pin 2 (PD1) [SHARED]
├── Pin 3: Clock ─────── Arduino Pin 3 (PD0) [SHARED]
├── Pin 4: Data ──────── Arduino Pin A0 (PF7)
├── Pin 5: Not used (or Power Pad D4 → Pin 9/PB5)
├── Pin 6: Not used (or Power Pad D3 → Pin 8/PB4)
├── Pin 7: Not used
└── Pin 8: GND

SNES CONTROLLER (7-pin connector):
├── Pin 1: VCC (+5V)
├── Pin 2: Latch ─────── Arduino Pin 2 (PD1) [SHARED]
├── Pin 3: Clock ─────── Arduino Pin 3 (PD0) [SHARED]
├── Pin 4: Data ──────── Arduino Pin A1 (PF6)
├── Pin 5: Not used (or NTT D2 → Pin 0/PD2)
├── Pin 6: Not used (or NTT D3 → Pin 1/PD3)
├── Pin 7: Not used
└── Pin 8: GND

GENESIS CONTROLLER (DB9 connector):
├── Pin 1: Up ────────── Arduino Pin 5 (PC6)
├── Pin 2: Down ──────── Arduino Pin 6 (PD7)
├── Pin 3: Left ──────── Arduino Pin A2 (PF5)
├── Pin 4: Right ─────── Arduino Pin A3 (PF4)
├── Pin 5: VCC (+5V) ─── Arduino Pin 16 (PB2)
├── Pin 6: A/B ───────── Arduino Pin 14 (PB3)
├── Pin 7: Select ────── Arduino Pin 7 (PE6)
├── Pin 8: GND
└── Pin 9: Start/C ──── Arduino Pin 15 (PB1)
```

#### Why Shared Clock/Latch Works
1. **Identical Protocol**: NES and SNES use the same timing (12μs latch, 6μs clock cycles)
2. **Simultaneous Polling**: Both controllers are read at exactly the same time
3. **Separate Recognition**: Individual data lines prevent conflicts
4. **Pin Efficiency**: Saves 2 precious Arduino pins for other features
5. **Proven Design**: Used in commercial multi-controller adapters

## Controller Button Mapping

### Updated Button Mapping (No N64)

```
     NES    PowerPad  SNES     GEN(normal)  GEN(MiSTer)
--------------------------------------------------------
X    U/D    N/A       U/D      U/D          U/D        
Y    L/R    N/A       L/R      L/R          L/R        
01   B      Pad 01    B        B            A          
02   A      Pad 02    A        A            B          
03   N/A    Pad 03    Y        Y            X          
04   N/A    Pad 04    X        X            Y          
05   N/A    Pad 05    L        Z            Z          
06   N/A    Pad 06    R        C            C          
07   SELECT Pad 07    SELECT   MODE         MODE       
08   START  Pad 08    START    START        START      
09   N/A    Pad 09    NTT 0    HOME(8BitDo) [SPECIAL]  
10   N/A    Pad 10    NTT 1    N/A          N/A        
11   N/A    Pad 11    NTT 2    N/A          N/A        
12   N/A    Pad 12    NTT 3    N/A          N/A        
13   N/A    N/A       NTT 4    N/A          N/A        
14   N/A    N/A       NTT 5    N/A          N/A        
15   N/A    N/A       NTT 6    N/A          N/A        
16   N/A    N/A       NTT 7    N/A          N/A        
17   N/A    N/A       NTT 8    N/A          N/A        
18   N/A    N/A       NTT 9    N/A          N/A        
19   N/A    N/A       NTT *    N/A          N/A        
20   N/A    N/A       NTT #    N/A          N/A        
21   N/A    N/A       NTT .    N/A          N/A        
22   N/A    N/A       NTT C    N/A          N/A        
23   N/A    N/A       N/A      N/A          N/A        
24   N/A    N/A       NTT End  N/A          N/A        

* GENESIS(MiSTer): Mode will send Select + Down

06   N/A    Pad 06    R        C            C            R

## Controller Button Mapping07   SELECT Pad 07    SELECT   MODE         MODE         C-Right

08   START  Pad 08    START    START        START        START

### Updated Button Mapping (No N64)09   N/A    Pad 09    NTT 0    HOME(8BitDo) [SPECIAL]    Z

10   N/A    Pad 10    NTT 1    N/A          N/A          D-Up

```11   N/A    Pad 11    NTT 2    N/A          N/A          D-Down

     NES    PowerPad  SNES     GEN(normal)  GEN(MiSTer)12   N/A    Pad 12    NTT 3    N/A          N/A          D-Left

--------------------------------------------------------13   N/A    N/A       NTT 4    N/A          N/A          D-Right

X    U/D    N/A       U/D      U/D          U/D        14   N/A    N/A       NTT 5    N/A          N/A          C-Up

Y    L/R    N/A       L/R      L/R          L/R        15   N/A    N/A       NTT 6    N/A          N/A          N/A

01   B      Pad 01    B        B            A          16   N/A    N/A       NTT 7    N/A          N/A          N/A

02   A      Pad 02    A        A            B          17   N/A    N/A       NTT 8    N/A          N/A          N/A

03   N/A    Pad 03    Y        Y            X          18   N/A    N/A       NTT 9    N/A          N/A          N/A

04   N/A    Pad 04    X        X            Y          19   N/A    N/A       NTT *    N/A          N/A          N/A

05   N/A    Pad 05    L        Z            Z          20   N/A    N/A       NTT #    N/A          N/A          N/A

06   N/A    Pad 06    R        C            C          21   N/A    N/A       NTT .    N/A          N/A          N/A

07   SELECT Pad 07    SELECT   MODE         MODE       22   N/A    N/A       NTT C    N/A          N/A          N/A

08   START  Pad 08    START    START        START      23   N/A    N/A       N/A      N/A          N/A          N/A

09   N/A    Pad 09    NTT 0    HOME(8BitDo) [SPECIAL]  24   N/A    N/A       NTT End  N/A          N/A          N/A

10   N/A    Pad 10    NTT 1    N/A          N/A        

11   N/A    Pad 11    NTT 2    N/A          N/A        * GENESIS(MiSTer): Mode will send Select + Down

12   N/A    Pad 12    NTT 3    N/A          N/A        ```

13   N/A    N/A       NTT 4    N/A          N/A        

14   N/A    N/A       NTT 5    N/A          N/A        ## MiSTer Home Menu Suggestion

15   N/A    N/A       NTT 6    N/A          N/A        * **NES:** SELECT + DOWN

16   N/A    N/A       NTT 7    N/A          N/A        * **SNES:** SELECT + DOWN

17   N/A    N/A       NTT 8    N/A          N/A        * **GENESIS:** MODE + DOWN

18   N/A    N/A       NTT 9    N/A          N/A        

19   N/A    N/A       NTT *    N/A          N/A        *Note: SELECT + DOWN = HOME on 8BitDo N30*

20   N/A    N/A       NTT #    N/A          N/A        

21   N/A    N/A       NTT .    N/A          N/A        ## MiSTer mode on 8bitdo M30 controller

22   N/A    N/A       NTT C    N/A          N/A        

23   N/A    N/A       N/A      N/A          N/A        The 4dapter exposes three "players" on one USB device.  Unfortunately MiSTer does not support setting keymaps per "player", MiSTer mode works around this by making sure the positional mapping is the same between all controllers.

24   N/A    N/A       NTT End  N/A          N/A        

MiSTer mode can be toggled on and off with "HOME + Z" (you have to press HOME first, Z second). This setting is saved to EEPROM and preserved across power cycles. The default is "normal mode".

* GENESIS(MiSTer): Mode will send Select + Down

```When the Genesis controller is in MiSTer mode:



## MiSTer FPGA Configuration- The HOME button sends "DOWN + MODE" (This is because no equivalent of the HOME button exists on the other controller ports)



### Automatic Setup (Recommended)- Buttons are swapped: A with B and X with Y. This is such that the position of the buttons is consistent between SNES and Genesis.

1. **Download** the controller map file from project releases

2. **Copy** `RETROdapter_4dapter.map` to `/media/fat/config/inputs/`## Retroarch Port Binding

3. **Reboot MiSTer** to apply configuration

4. **Controllers auto-configured** for all coresSome Retroarch users have reported issues regarding setting the correct “Device Index” setting, since the 3dapter reports a single “device” to RetroArch with 3 different controllers, each as their own “index”.



### MiSTer Home Menu SuggestionsIn Retroarch, go to Settings → Input → Port 1 Binds → Device Index and then choose #3 for N64 and presumably #1 or #2 for NES/SNES and Genesis. Save and it sticks for that game, core or content directory.

* **NES:** SELECT + DOWN

* **SNES:** SELECT + DOWNhttps://retropie.org.uk/forum/topic/26681/port-binds/

* **GENESIS:** MODE + DOWNhttps://retropie.org.uk/docs/RetroArch-Configuration/#core-input-remapping



*Note: SELECT + DOWN = HOME on 8BitDo N30*## Install Instructions



### MiSTer Mode on Genesis Controllers### 1. Select "Arduino AVR Boards - Arduino Leonardo" from Boards List



The 4dapter exposes three separate "players" as individual USB devices. MiSTer mode can be toggled with "HOME + Z" (press HOME first, then Z). This setting is saved to EEPROM.### 2. Download project to Arduino Pro Micro board



When Genesis controller is in MiSTer mode:## Restoring Firmware After Alternative Firmware Downloads

- **HOME button** sends "DOWN + MODE"

- **Buttons swapped**: A↔B and X↔Y for layout consistencyIf you previously used the Analogue Pocket or Nintendo Switch firmware versions, your Arduino Pro Micro will no longer report as a Serial device and will not appear in the "Port" list in the Arduino software. Using the default 3dapter firmware does not suffer from this issue.



## Installation InstructionsTo restore the default firmware for your Arduino Pro Micro, you will need to manually reset the Arduino Pro Micro by shorting the Reset + Ground pins together to trigger a reset. 



### PlatformIO Setup (Recommended)1. Open the default firmware project in the Arduino software.

1. **Install PlatformIO** IDE or Core2. Connect your Arduino Pro Micro to your computer.

2. **Clone/download** this project3. Trigger the download from the Arduino software. The Arduino software will first compile the project before beginning the download.

3. **Open project** in PlatformIO4. Once the "compiling" step has finished, trigger a reset on the Arduino Pro Micro by briefly touching the Reset (RST) and Ground (GND) pins together. This should make the Arduino forcefully switch into bootloader mode and allow the download to complete. If the download fails initially due to not finding the COM port, repeat the download/reset process and it should work the second time.

4. **Connect Arduino** via USB
5. **Build and upload**:
   ```bash
   pio run --target upload
   ```

### Arduino IDE (Legacy)
- **Board**: Arduino Leonardo
- **Upload** main.cpp as .ino file
- **Install dependencies** manually

## RetroArch Configuration

### Device Index Setup
The adapter creates three separate USB devices:

1. **Go to**: Settings → Input → Port Binds
2. **Set Device Index**:
   - **Device #1**: NES Controller
   - **Device #2**: SNES Controller  
   - **Device #3**: Genesis Controller
3. **Save configuration** per core or globally

## Troubleshooting

### Controllers Not Detected
- **Check connections**: Verify all pins properly connected
- **Test controllers**: Ensure controllers work on original hardware
- **Power supply**: Confirm Arduino receives adequate power (500mA+)

### Incorrect Button Mapping
- **Clear existing maps**: Remove old controller configurations
- **Use provided maps**: Install official MiSTer/RetroArch configurations
- **Manual calibration**: Re-run controller setup in target system

### Firmware Recovery
If you previously used alternative firmware versions:

1. **Open this project** in PlatformIO
2. **Connect Arduino** to computer
3. **Start upload process** in PlatformIO
4. **During compilation**: Short Reset (RST) and Ground (GND) pins
5. **Repeat if necessary**: May require 2-3 attempts

## Technical Specifications

### Project Structure
```
├── platformio.ini          # PlatformIO configuration
├── src/
│   ├── main.cpp            # Main firmware logic
│   ├── Gamepad.cpp         # USB HID gamepad implementation
│   ├── NESController.cpp   # NES controller driver
│   ├── SNESController.cpp  # SNES controller driver
│   └── SegaController32U4.cpp # Genesis controller driver
├── include/
│   ├── Gamepad.h           # USB HID gamepad interface
│   ├── NESController.h     # NES controller interface
│   ├── SNESController.h    # SNES controller interface
│   └── SegaController32U4.h # Genesis controller interface
└── README.md               # This documentation
```

### Performance
- **Input Latency**: <1ms controller to USB
- **Polling Rate**: 1000Hz USB HID updates
- **Memory Usage**: ~8KB Flash, ~512B SRAM
- **CPU Usage**: ~15% at 16MHz

## Software Architecture

### Key Features

#### Separated Controller Interfaces
- **Independent USB HID devices** for each controller type
- **No shared button conflicts** between controller types
- **Optimal compatibility** with multi-controller games

#### Advanced Controller Support
- **Power Pad detection** with 12-button mapping
- **NTT Data Keypad** with numeric input support
- **6-button Genesis** with automatic detection
- **MiSTer mode** for consistent button layouts

#### Timing-Critical Communication
- **Precise timing** using CPU cycle delays
- **Hardware register access** for optimal performance
- **Interrupt-safe** polling sequences

## Development and Modification

### Building from Source
```bash
# Clone repository
git clone [repository-url]
cd retrodapter-4dapter

# Build with PlatformIO
pio run

# Upload to device  
pio run --target upload

# Monitor serial output
pio device monitor
```

### Adding New Controllers
1. **Create controller class** following existing patterns
2. **Implement communication protocol** in separate files
3. **Update main.cpp** to include new controller processing
4. **Add pin definitions** and hardware configuration
5. **Update documentation** with new controller support

## License

GNU General Public License v3.0

```
RETROdapter 4dapter Firmware
Copyright (C) 2025 RETROdapter Project

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
```

## Credits and Acknowledgments

### Original Contributors
- **Mikael Norrgård** - Original 4dapter firmware foundation
- **Jon Thysell** - Genesis controller implementation  
- **NicoHood** - HID library contributions
- **Daemonbite** - Controller protocol research

### Project Evolution
- **Base Project**: Original Arduino IDE 4dapter firmware
- **This Version**: PlatformIO conversion with separated controllers
- **Key Changes**: N64 removed, NES/SNES separated, enhanced documentation

## Changelog

### Version 2.0.0 (Current)
- ✅ **Converted to PlatformIO** project structure
- ✅ **Removed N64 support** for simplified design
- ✅ **Separated NES and SNES** into individual controllers  
- ✅ **Maintained Genesis support** with MiSTer mode
- ✅ **Enhanced documentation** in English
- ✅ **Improved code organization** with separate controller classes
- ✅ **Added comprehensive comments** for maintainability

### Version 1.x (Legacy)
- Arduino IDE based firmware
- Combined NES/SNES controller interface
- N64 controller support included
- Limited documentation

---

**Happy retro gaming with modern convenience!** 🎮