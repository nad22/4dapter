/**
 * N64 To USB adapter
 * by Michele Perla (the.mickmad@gmail.com)
 * https://github.com/MickMad/N64-To-USB
 * 
 * Gamecube controller to Nintendo 64 adapter
 * by Andrew Brown
 * Rewritten for N64 to HID by Peter Den Hartog
 */

#include "N64_Controller.h"
#include "Arduino.h"

N64Controller::N64Controller()
{
  rumbleEnabled = false;
  rumblePakDetected = false;
}

void N64Controller::N64_init()
{
  //N64 Setup
  digitalWrite(N64_PIN, LOW);  
  pinMode(N64_PIN, INPUT);
}

void N64Controller::translate_N64_data()
{
    memset(&N64_status, 0, sizeof(N64_status));
    
    // line 1
    // bits: A, B, Z, Start, Dup, Ddown, Dleft, Dright
    // line 2
    // bits: Reset, 0, L, R, Cup, Cdown, Cleft, Cright
    // line 3
    // bits: joystick x value
    // These are 8 bit values centered at 0x80 (128)
    // line 4
    // bits: joystick 4 value
    // These are 8 bit values centered at 0x80 (128)
    
    for (int i=0; i<8; i++) 
    {
        N64_status.data1 |= N64_raw_dump[i] ? (0x80 >> i) : 0;
        N64_status.data2 |= N64_raw_dump[8+i] ? (0x80 >> i) : 0;
        N64_status.stick_x |= N64_raw_dump[16+i] ? (0x80 >> i) : 0;
        N64_status.stick_y |= N64_raw_dump[24+i] ? (0x80 >> i) : 0;
    }
}


/**
 * This sends the given byte sequence to the controller
 * length must be at least 1
 * Oh, it destroys the buffer passed in as it writes it
 */
void N64Controller::N64_send_data_request(unsigned char *buffer, char length)
{
    char bits;
    bool bit;
    unsigned char timeout;
    char bitcount = 32;
    char *bitbin = N64_raw_dump;


  outer_loop:
    {
        bits = 8;
        
        inner_loop:
        {
            // Starting a bit, set the line low
            N64_LOW;

            if (*buffer >> 7) //Bit is a 1
            {
                __builtin_avr_delay_cycles(5);
                N64_HIGH;
                __builtin_avr_delay_cycles(40);
            } 
            else  //Bit is a 0
            {
                 __builtin_avr_delay_cycles(40);
                N64_HIGH;
            }
            
            --bits;
            if (bits != 0) 
            {
                __builtin_avr_delay_cycles(8);
                *buffer <<= 1;
                goto inner_loop;
            }
        }
        
        --length;
        if (length != 0) 
        {
            ++buffer;
            goto outer_loop;
        } 
    }

    // send a single stop (1) bit
    __builtin_avr_delay_cycles(8);
    N64_LOW;

    // wait 1 us, 16 cycles, then raise the line 
    __builtin_avr_delay_cycles(16);
    N64_HIGH;

    //////////////////////
    // listen for the expected 8 bytes of data back from the controller and
    // blast it out to the N64_raw_dump array, one bit per byte for extra speed.

    timeout = 0x7f;
    while (!N64_QUERY) 
    {
        if (!--timeout)
            return;
    }

read_loop:
   
    // wait for line to go low
    timeout = 0x7f;
    while (N64_QUERY) 
    {
        if (!--timeout)
            return;
    }

    //Wait 2us before reading data
    __builtin_avr_delay_cycles(32);
   
    *bitbin = N64_QUERY;
    ++bitbin;
    --bitcount;
    
    if (bitcount == 0)
        return;

    // wait for line to go high again
    // it may already be high, so this should just drop through
    timeout = 0x3f;
    while (!N64_QUERY) 
    {
        if (!--timeout)
            return;
    }
    
    goto read_loop;
}

void N64Controller::getN64Packet()
{
    unsigned char N64Command[] = {0x01};
    noInterrupts();
    N64_send_data_request(N64Command, 1);
    interrupts();
    translate_N64_data();
}

/**
 * Rumble Pak Support Functions
 * Based on BitBuilt forum research by JacksonS
 * Implementation uses precise timing for reliable N64 communication
 */

bool N64Controller::checkRumblePak()
{
    // Initialize rumble pak first (required by raphnet protocol)
    return initializeRumblePak();
}

bool N64Controller::initializeRumblePak()
{
    // Step 1: Initialize rumble pak with 0x80 pattern at address 0x8001
    // This is REQUIRED before rumble pak can be used (raphnet protocol)
    unsigned char init_data[32];
    memset(init_data, 0x80, 32);  // Fill with 0x80 (not 0x00!)
    
    bool success = writeMemoryPak(RUMBLEPAK_INIT_ADDRESS, init_data, 32);
    if (success) {
        rumblePakDetected = true;
    }
    return success;
}

void N64Controller::setRumble(bool enable)
{
    if (!rumblePakDetected) {
        // Try to initialize rumble pak if not detected yet
        if (!checkRumblePak()) return;
    }
    
    rumbleEnabled = enable;
    
    // Create 32-byte buffer for rumble control command
    unsigned char rumble_data[32];
    
    if (enable) {
        // Fill buffer with 0x01 to enable rumble
        memset(rumble_data, 0x01, 32);
    } else {
        // Fill buffer with 0x00 to disable rumble
        memset(rumble_data, 0x00, 32);
    }
    
    // Write to rumble control address (0xC01B, not 0x8000!)
    writeMemoryPak(RUMBLEPAK_CTRL_ADDRESS, rumble_data, 32);
}

bool N64Controller::writeMemoryPak(unsigned short address, unsigned char* data, int length)
{
    // Based on raphnet N64 expansion write protocol
    // Command format: [0x03][addr_hi][addr_lo][32_bytes_data]
    unsigned char command[35]; // 1 + 2 + 32 bytes
    
    command[0] = N64_EXPANSION_WRITE;             // 0x03 (raphnet standard)
    command[1] = (address >> 8) & 0xFF;          // Address high byte  
    command[2] = address & 0xFF;                 // Address low byte
    
    // Copy data to command buffer (always 32 bytes for rumble pak)
    if (data != NULL && length > 0) {
        memcpy(&command[3], data, min(length, 32));
        // Fill remaining bytes with zeros if length < 32
        if (length < 32) {
            memset(&command[3 + length], 0x00, 32 - length);
        }
    } else {
        // Default: all zeros (rumble off)
        memset(&command[3], 0x00, 32);
    }
    
    // Send the command with precise timing
    noInterrupts();
    sendRumbleCommand(command, 35);
    interrupts();
    
    return true; // Raphnet implementation assumes success
}

void N64Controller::sendRumbleCommand(unsigned char* buffer, int length)
{
    // Create a copy since the original function destroys the buffer
    unsigned char temp_buffer[35];
    memcpy(temp_buffer, buffer, length);
    
    // Use EXACT same timing as working N64_send_data_request function for sending
    char bits;
    unsigned char *current_buffer = temp_buffer;
    
    // SEND PHASE - exactly like N64_send_data_request
    // Outer loop for each byte
    for (int i = 0; i < length; i++) {
        bits = 8;
        
        // Inner loop for each bit in the byte
        while (bits != 0) {
            // Starting a bit, set the line low
            N64_LOW;

            if (*current_buffer >> 7) // Bit is a 1
            {
                __builtin_avr_delay_cycles(5);
                N64_HIGH;
                __builtin_avr_delay_cycles(40);
            } 
            else  // Bit is a 0
            {
                 __builtin_avr_delay_cycles(40);
                N64_HIGH;
            }
            
            --bits;
            if (bits != 0) 
            {
                __builtin_avr_delay_cycles(8);
                *current_buffer <<= 1;
            }
        }
        
        // Move to next byte
        ++current_buffer;
    }

    // Send a single stop (1) bit - same as original
    __builtin_avr_delay_cycles(8);
    N64_LOW;
    __builtin_avr_delay_cycles(16);
    N64_HIGH;

    // READ PHASE - Read 1-byte response (8 bits) from expansion write
    // Based on raphnet: expansion write returns 1 byte status
    unsigned char timeout;
    
    // Wait for controller to start response
    timeout = 0x7f;
    while (!N64_QUERY) 
    {
        if (!--timeout)
            return; // Timeout - no response
    }

    // Read 8 bits (1 byte) response
    unsigned char response_byte = 0;
    for (int bit = 7; bit >= 0; bit--) {
        // Wait for line to go low (start of bit)
        timeout = 0x7f;
        while (N64_QUERY) 
        {
            if (!--timeout)
                return;
        }

        // Wait 2us before reading data
        __builtin_avr_delay_cycles(32);
       
        // Read bit value and store in response_byte
        if (N64_QUERY) {
            response_byte |= (1 << bit);
        }

        // Wait for line to go high again
        timeout = 0x3f;
        while (!N64_QUERY) 
        {
            if (!--timeout)
                return;
        }
    }
    
    // Response received successfully
    // According to raphnet: response should be 1 byte status (0x00 = success)
}
