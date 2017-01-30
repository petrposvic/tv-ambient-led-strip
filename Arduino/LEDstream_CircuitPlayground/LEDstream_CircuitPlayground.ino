// This is a pared-down version of the LEDstream sketch specifically
// for Circuit Playground.  It is NOT a generic solution to NeoPixel
// support with Adalight!  The NeoPixel library disables interrupts
// while issuing data...but Serial transfers depend on interrupts.
// This code works (or appears to work, it hasn't been extensively
// battle-tested) only because of the finite number of pixels (10)
// on the Circuit Playground board.  With 10 NeoPixels, interrupts are
// off for about 300 microseconds...but if the incoming data rate is
// sufficiently limited (<= 60 FPS or so with the given number of
// pixels), things seem OK, no data is missed.  Balancing act!

// --------------------------------------------------------------------
//   This file is part of Adalight.

//   Adalight is free software: you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as
//   published by the Free Software Foundation, either version 3 of
//   the License, or (at your option) any later version.

//   Adalight is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU Lesser General Public License for more details.

//   You should have received a copy of the GNU Lesser General Public
//   License along with Adalight.  If not, see
//   <http://www.gnu.org/licenses/>.
// --------------------------------------------------------------------

#include <PololuLedStrip.h>

PololuLedStrip<2> ledStrip;

static const uint8_t magic[] = { 'A','d','a' };
#define MAGICSIZE  sizeof(magic)
#define HEADERSIZE (MAGICSIZE + 3)
static uint8_t
  buffer[HEADERSIZE], // Serial input buffer
  bytesBuffered = 0;  // Amount of data in buffer

static const unsigned long serialTimeout = 10000; // 10 seconds
static unsigned long       lastByteTime;

void setup() {
  Serial.begin(115200);
  lastByteTime = millis(); // Initialize timers
}

// Function is called when no pending serial data is available.
static boolean timeout(
  unsigned long t,       // Current time, milliseconds
  int           nLEDs) { // Number of LEDs
  // If no data received for an extended time, turn off all LEDs.
  if((t - lastByteTime) > serialTimeout) {
    rgb_color colors[nLEDs];
    rgb_color color;
    color.red = 0;
    color.green = 0;
    color.blue = 0;
    for (int i = 0; i < nLEDs; i++) {
      colors[i] = color;
    }
    ledStrip.write(colors, nLEDs);

    lastByteTime  = t; // Reset counter
    bytesBuffered = 0; // Clear serial buffer
    return true;
  }

  return false; // No timeout
}

void loop() {
  uint8_t       i, hi, lo, byteNum;
  int           c;
  long          nLEDs, pixelNum;
  unsigned long t;

  // HEADER-SEEKING BLOCK: locate 'magic word' at start of frame.

  // If any data in serial buffer, shift it down to starting position.
  for(i=0; i<bytesBuffered; i++)
    buffer[i] = buffer[HEADERSIZE - bytesBuffered + i];

  // Read bytes from serial input until there's a full header's worth.
  while(bytesBuffered < HEADERSIZE) {
    t = millis();
    if((c = Serial.read()) >= 0) { // Data received?
      buffer[bytesBuffered++] = c; // Store in buffer
      lastByteTime = t;            // Reset timeout counter
    } else {                       // No data, check for timeout...
      // Second parameter should be high number, but not extremly
      // high. My TV uses 128 LEDs so 128 is minimum value for me.
      if(timeout(t, 128) == true) return; // Start over
    }
  }

  // Have a header's worth of data.  Check for 'magic word' match.
  for(i=0; i<MAGICSIZE; i++) {
    if(buffer[i] != magic[i]) {      // No match...
      if(i == 0) bytesBuffered -= 1; // resume search at next char
      else       bytesBuffered -= i; // resume at non-matching char
      return;
    }
  }

  // Magic word matches.  Now how about the checksum?
  hi = buffer[MAGICSIZE];
  lo = buffer[MAGICSIZE + 1];
  if(buffer[MAGICSIZE + 2] != (hi ^ lo ^ 0x55)) {
    bytesBuffered -= MAGICSIZE; // No match, resume after magic word
    return;
  }

  // Checksum appears valid.  Get 16-bit LED count, add 1 (nLEDs always > 0)
  nLEDs = 256L * (long)hi + (long)lo + 1L;
  bytesBuffered = 0; // Clear serial buffer
  byteNum = 0;
  rgb_color colors[nLEDs];

  // DATA-FORWARDING BLOCK: move bytes from serial input to NeoPixels.

  for(pixelNum = 0; pixelNum < nLEDs; ) { // While more LED data is expected...
    t = millis();
    if((c = Serial.read()) >= 0) { // Successful read?
      lastByteTime = t;            // Reset timeout counters
      buffer[byteNum++] = c;       // Store in data buffer
      if(byteNum == 3) {           // Have a full LED's worth?
        rgb_color color;
        color.red = buffer[0];
        color.green = buffer[1];
        color.blue = buffer[2];
        colors[pixelNum++] = color;
        byteNum = 0;
      }
    } else { // No data, check for timeout...
      if(timeout(t, nLEDs) == true) return; // Start over
    }
  }

  ledStrip.write(colors, nLEDs);
}

