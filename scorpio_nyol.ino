/*
*** THIS IS THE PLACE TO START! *** Adafruit_NeoPXL8 has complex wiring
options that vary from board to board. This example discusses all that and
lights pixels in predictable test patterns as a wiring diagnostic tool.
Subsequent examples forego the long explanantion and just handle their task.
Figure out your connections here, then copy your findings into other code.
Pixel-setting operations are the same as Adafruit_NeoPixel and are not
explained here -- for that, see examples included with Adafruit_NeoPixel.

REQUIREMENTS:
* Adafruit_NeoPixel library.
* For M0 and M4 boards (SAMD21, SAMD51): Adafruit_ZeroDMA library.
* For RP2040 boards: install Earle F. Philhower's "Raspberry Pi Pico/RP2040"
                     board package.
* For ESP32S3 boards: install Espressif ESP32 package. ONLY the S3 is
*                     supported; no S2, C3 or original ESP32.
* NeoPixels with suitable wiring and adequate power supply.
* 5V-powered NeoPixels may require a logic level shifter (e.g. 75HCT245),
  or use a NeoPXL8 FeatherWing or Friend. There are DIFFERENT VERSIONS of
  the NeoPXL8 FeatherWing for M0 and M4 Feathers. For non-Feather boards,
  consider the NeoPXL8 Friend breakout board.

PRODUCT LINKS:
* NeoPXL8 FeatherWing M0: https://www.adafruit.com/product/3249
* NeoPXL8 FeatherWing M4: https://www.adafruit.com/product/4537
* NeoPXL8 Friend: https://www.adafruit.com/product/3975
* NeoPixels: https://www.adafruit.com/category/168

RESOURCES:
* NeoPixel Uberguide: https://learn.adafruit.com/adafruit-neopixel-uberguide
*/

#include <Adafruit_NeoPXL8.h>

//#include "pico/stdlib.h"
//#include "pico/stdio.h"
//#include "pico/stdio_usb.h"

#define NUM_LEDS    180      // NeoPixels PER STRAND, total number is 8X this!
#define COLOR_ORDER NEO_GRB // NeoPixel color format (see Adafruit_NeoPixel)

// In a moment we'll declare a global Adafruit_NeoPXL8 object.
// The constructor expects three arguments:
// * The number of NeoPixels PER STRAND (there can be up to 8 strands).
// * A uint8_t array of 8 output pins, or pass NULL to use pins 0-7 on Metro
//   Express or Arduino Zero boards.
// * NeoPixel color order, same as with the Adafruit_NeoPixel library.
//   Different types and revisions of NeoPixel and WS2812-compatible LEDs
//   expect color data in a particular order.
// Two of these were #defined above for easy use. But the middle one --
// the pin array -- requires a whole DISCUSSION. What follows are some pin
// arrays for different situations. MOST ARE COMMENTED OUT HERE, idea being
// that you would enable one or another, or come up with your own list
// following the rules explained here...

// To use a default pin setup (pins 0-7 on Adafruit Metro M0/M4, Arduino
// Zero, etc.), NULL can be used in place of the pin array. Comment this out
// if using one of the pin lists that follow, or your own list:

// int8_t *pins = NULL; // COMMENT THIS OUT IF USING A PIN LIST BELOW

// In most situations you'll declare an int8_t array of 8 elements, one per
// pin. You can use fewer than 8 outputs by placing a -1 in one or more
// places. The array MUST have 8 elements, no more or less, and each board
// has SPECIFIC RULES about pin choices. Within that list and those rules,
// valid pins can be arranged in any order. For example: if integrating
// NeoPXL8 into an existing FadeCandy or OctoWS2811 installation, you might
// need to reverse or reorder the pin list to get a coherent LED pattern.

// M0 AND M4 BOARDS (SAMD21, SAMD51 MICROCONTROLLERS) ----------------------

// For Feather M0 and M4, the corresponding NeoPXL8 FeatherWings are NOT
// interchangeable -- you must have a matched Feather and 'Wing!

// Here's a pinout that works with the Feather M0 (NOT M4) w/NeoPXL8 M0
// FeatherWing as it ships from the factory:
// int8_t pins[8] = { PIN_SERIAL1_RX, PIN_SERIAL1_TX, MISO, 13, 5, SDA, A4, A3 };

// 5 pins on the M0 Featherwing have configurable pads that can be cut and
// solder-jumpered to altername pins, in case the default connections
// interfere with a needed peripheral (Serial1, I2C or SPI). You do NOT need
// to use all 5 alternates; pick and choose as needed! But if changing all 5,
// they would be:
// int8_t pins[8] = { 12, 10, 11, 13, SCK, MOSI, A4, A3 };
// Notice the last two are unchanged; those outputs are not reconfigurable.

// Here's a pinout that works with the Feather M4 (not M0) w/NeoPXL8 M4
// FeatherWing in the factory configuration:
// int8_t pins[8] = { 13, 12, 11, 10, SCK, 5, 9, 6 };
// Similar to the M0 Wing above where the first 5 pins are configurable, on
// M4 the last 4 can be changed with some cutting/soldering:
// int8_t pins[8] = { 13, 12, 11, 10, PIN_SERIAL1_RX, PIN_SERIAL1_TX, SCL, SDA };
// Notice the first four are unchanged; those outputs are not reconfigurable.

// Here's a pinout that works on the Metro M4:
// int8_t pins[8] = { 7, 4, 5, 6, 3, 2, 10, 11 };
// An alternate set of pins on Metro M4, but only 7 outputs:
// int8_t pins[8] = { 9, 8, 0, 1, 13, 12, -1, SCK };

// For Grand Central, here are primary and alternate pin options:
// int8_t pins[8] = { 30, 31, 32, 33, 36, 37, 34, 35 };
// int8_t pins[8] = { 30, 31, 32, 33, 15, 14, 27, 26 };

// For other SAMD21/SAMD51 (M0 and M4) boards not listed here: NeoPXL8
// relies on these chip's "pattern generator" peripheral, which is only
// availabe on certain pins. This requires some schematic and/or datasheet
// sleuthing to identify PCC/DATA[0] through [7] pins.

// RP2040 BOARDS -----------------------------------------------------------

// IMPORTANT: when used with RP2040 devices, the pin array requires "GPxx"
// pin numbers, which sometimes vary from the Arduino pin numbers
// silkscreened on the board's top side. Some boards helpfully use Arduino
// numbers on top, with GPxx equivalents on the bottom side for reference.
// The GPxx numbers MUST be within any contiguous range of 8 pins, though
// they can be re-ordered within that range, or unused elements set to -1.

// The M4 FeatherWing *almost* aligns with the Feather RP2040, but requires
// cutting the trace between the "n0" SCK selector pad, then soldering a
// wire from the n0 center pad (there's no via) to D4. You can then use this
// array to access all 8 outputs:
// int8_t pins[8] = { 6, 7, 9, 8, 13, 12, 11, 10 }; // GPxx indices!
// On Feather RP2040, corresponds to top-labeled 4, 5, 9, 6, 13, 12, 11, 10.
// There are no alternate pins for Feather RP2040, since this is the only
// 8-contiguous-bits combination, though you can reverse, reorder or use -1.

// For the Feather RP2040 SCORPIO, use this list:
int8_t pins[8] = { 16, 17, 18, 19, 20, 21, 22, 23 };

// For Raspberry Pi Pico, you can use any 8 contiguous GPIO pins (e.g. the
// default 0-7) with a level shifter or NeoPXL8 Friend.

// ESP32S3 BOARDS ----------------------------------------------------------

// These allow ANY 8 pins for output...so you can use the NeoPXL8 M0 or M4
// FeatherWings unmodified, with one of the pin lists provided earlier.

// LET'S DO THE THING! -----------------------------------------------------

// Here's the global constructor as explained near the start:
Adafruit_NeoPXL8 leds(NUM_LEDS, pins, COLOR_ORDER);

// For this demo we use a table of 8 hues, one for each strand of pixels:
static uint8_t demo_colors[8][3] = {
  255,   0,   0, // Row 0: Red
  255, 160,   0, // Row 1: Orange
  255, 255,   0, // Row 2: Yellow
    0, 255,   0, // Row 3: Green
    0, 255, 255, // Row 4: Cyan
    0,   0, 255, // Row 5: Blue
  192,   0, 255, // Row 6: Purple
  255,   0, 255  // Row 7: Magenta
};

uint8_t *pixels;
unsigned long lastDataReceived = 0;
const unsigned long TIMEOUT = 1000; // 1 second timeout

// setup() runs once on program startup:
void setup() {
  // The baud rate is seemingly ignored by the Feather / RP2040 simulated UART, which seems to always run at line speed.
  // I observe just under 1 Mbps, but that's ignoring all overheads.
  Serial.begin(2000000);
  /*
  while (!Serial) {
  }
  // XXX: fine to come up without serial, just cope with it
  */
  Serial.println("LED controller starting up...");

  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 5; ++i) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(10);                       // wait for a second
  }

  // Start NeoPXL8. If begin() returns false, either an invalid pin list
  // was provided, or requested too many pixels for available RAM.
  // - dbuf=true means we get double buffering! More RAM, more speed!
  if (!leds.begin(true)) {
    Serial.println("Failed to initialize LEDs.");
    // Blink the onboard LED if that happens.
    pinMode(LED_BUILTIN, OUTPUT);
    for (;;) digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
  }
  Serial.println("Initialized LEDs. Testing color pattern (should flash in order RED/GREEN/BLUE).");
  // Otherwise, NeoPXL8 is now running, we can continue.

  leds.setBrightness(10); // Tone it down, NeoPixels are BRIGHT!
  pixels = leds.getPixels();
  // XXX: DON'T DO THIS
  //leds.setLatchTime(200);

  // Cycle all pixels red/green/blue on startup. If you see a different
  // sequence, COLOR_ORDER doesn't match your particular NeoPixel type.
  // If you get a jumble of colors, you're using RGBW NeoPixels with an
  // RGB order. Try different COLOR_ORDER values until code and hardware
  // are in harmony.
  for (uint32_t color = 0xFF0000; color > 0; color >>= 8) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    leds.fill(color);
    leds.show();
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(200);                       // wait for a second
  }

  Serial.println("Testing strips (should flash each strip in sequence.)");
  // Light each strip in sequence. This helps verify the mapping of pins to
  // a physical LED installation. If strips flash out of sequence, you can
  // either re-wire, or just change the order of the pins[] array.
  for (int i=0; i<8; i++) {
    if (pins && (pins[i] < 0)) continue; // No pixels on this pin
    leds.fill(0);
    uint32_t color = leds.Color(demo_colors[i][0], demo_colors[i][1], demo_colors[i][2]);
    leds.fill(color, i * NUM_LEDS, NUM_LEDS);
    leds.show();
    delay(100);
  }

  Serial.println("Startup complete.");
  Serial.println("OK");
}

// XXX: no point making this so large, we will never get an available read larger than some internal buffer (and it might arbitrarily be smaller due to timing.)
#define BUFLEN 18000  // enough for two bytes per color per pixel per strand, more than twice over
char buf[BUFLEN];
char *bufptr = buf;

unsigned long cmd_start = 0;
unsigned long cmd_end = 0;

// loop() runs over and over indefinitely. We use this to render each frame
// of a repeating animation cycle based on elapsed time:

int serialOkay = 1;

void loop() {
  if (millis() - lastDataReceived > TIMEOUT) {
    serialOkay = 0;
    /*
    // Connection might be stuck, try to reset it
    for (int i = 0; i < 3; ++i) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
    Serial.end();
    delay(100);
    Serial.begin(2000000);
    Serial.println("nobody is listening, does this kill the line");
    while (!Serial) {
    }
    for (int i = 0; i < 3; ++i) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
    lastDataReceived = millis();
    */
  } else {
    serialOkay = 1;
  }
  if (Serial) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  //Serial.print("%");
  Serial.flush(); // no idea if this should make any difference to anything; wait for outgoing serial before continuing. Trying to prevent mysterious serial lockups.
  while (serialOkay && !Serial.available()) {
    if (millis() - lastDataReceived > TIMEOUT) {
      serialOkay = 0;
    }
    // just ... don't do anything and wait patiently. We have nothing else to do here.
  }
  int toread = Serial.available();
  if (toread) {
    lastDataReceived = millis();
    if (cmd_start == 0) {
      // perhaps slight underestimate, since we start counting the first packet late, but whatever
      cmd_start = micros();
    }
  }
  int space = BUFLEN - (bufptr - buf) - 1;
  if (space == 0) {
    Serial.println("WARNING: filled buffer without finding newline. Buffer flushed without processing.");
    cmd_start = 0;
    bufptr = buf;
    space = BUFLEN - 1;
  }
  if (toread > space) {
    Serial.println("(buffer nearly full!)");
    toread = space;
  }
  if (toread) {
    //Serial.print("#");
    Serial.flush(); // ??? maybe?
    delay(1); // ??? ??? maaaybe?
    size_t readbytes = Serial.readBytesUntil('\n', bufptr, toread);
    // Must do this here for debug print to not explode.
    bufptr[readbytes] = 0;
    /*
    Serial.print("Serial read: '");
    Serial.print(bufptr);
    Serial.println("'");
    Serial.print("Whole buffer now: '");
    Serial.print(buf);
    Serial.println("'");
    */
    bufptr += readbytes;

    if (readbytes < toread) {
      // Saw a line terminator.
      // (This should always trigger, even if we unluckily read exactly to the byte before the newline; it will just take an extra loop to notice.)
      docmd();
    }
  }

  if (!serialOkay) {
    // go into demo mode if we lose the serial port. :-(
    uint32_t now = millis(); // Get time once at start of each frame
    for(uint8_t r=0; r<8; r++) { // For each row...
      for(int p=0; p<NUM_LEDS; p++) { // For each pixel of row...
        leds.setPixelColor(r * NUM_LEDS + p, rain(now, r, p));
      }
    }
    leds.show();
  }
}

void docmd() {
  Serial.println("Got a full line.");
  Serial.print("time taken (microseconds): ");
  cmd_end = micros();
  Serial.println(cmd_end - cmd_start);
  //Serial.print("Whole buffer now: '");
  //Serial.print(buf);
  //Serial.println("'");
  cmd_start = 0;
  int cmdlen = bufptr-buf;
  bufptr = buf;
  if (buf[0] == '*') {  // set all lights
    cmdlen -= 1;
    Serial.print("Got * command, setting lights...");
    for (int i = 0; i < cmdlen / 6; ++i) {
      uint32_t color = read_hex_color(buf + i*6 + 1);
      //Serial.print(i);
      //Serial.print("-");
      //Serial.print(color);
      //Serial.print(" ");
      leds.setPixelColor(i, color);
      //pixels[i] = color; <-- wrong
    }
    Serial.print("updating leds...");
    leds.show();
    Serial.print("updating leds after getting cmd took (microseconds): ");
    Serial.println(micros() - cmd_end);
  } else if (buf[0] == '@') {  // set all lights, binary
    // XXX: this is badly defined, since a '\n' could appear in the binary data and terminate the line. We'll resync but it's stupid.
    cmdlen -= 1;
    Serial.print("Got @ command, setting lights (binary)...");
    for (int i = 0; i < cmdlen / 3; ++i) {
      uint32_t color = (buf[i*3 + 1] << 16) + (buf[i*3 + 2] << 8) + (buf[i*3 + 3]);
      //Serial.print(i);
      //Serial.print("-");
      //Serial.print(color);
      //Serial.print(" ");
      leds.setPixelColor(i, color);
      //pixels[i] = color; <-- wrong
    }
    Serial.print("updating leds...");
    leds.show();
    Serial.print("updating leds after getting cmd took (microseconds): ");
    Serial.println(micros() - cmd_end);
  } else if (buf[0] == '%') {  // set all lights, compact binary
    // XXX: this is badly defined, since a '\n' could appear in the binary data and terminate the line. We'll resync but it's stupid.
    Serial.print("Got % command, setting lights (compact binary)...");
    cmdlen -= 1;
    for (int i = 0; i < cmdlen; ++i) {
      int red = buf[i+1] & 0xc0;
      int green = (buf[i+1] & 0x38) << 2;
      int blue = (buf[i+1] & 0x07) << 5;
      uint32_t color = (red << 16) + (green << 8) + blue;
      //Serial.print(i);
      //Serial.print("-");
      //Serial.print(color);
      //Serial.print(" ");
      leds.setPixelColor(i, color);
      //pixels[i*3] = red;
      //pixels[i*3+1] = green;
      //pixels[i*3+2] = blue;
    }
    Serial.print("updating leds...");
    leds.show();
    Serial.print("updating leds after getting cmd took (microseconds): ");
    Serial.println(micros() - cmd_end);
  } else if (buf[0] == '#') {
    Serial.println("ignoring comment.");
  } else {
    Serial.println("I didn't understand that line.");
  }

  Serial.println("OK");
}

uint8_t read_hex_byte(char* hex) {
    //uint8_t val = 0;
    // Can't use sscanf here because it's slow as motherfucking donkey balls, for some odd reason. Probably calls strlen...
    // sscanf(hex, "%2hhx", &val);
    char digits[3];
    digits[0] = hex[0];
    digits[1] = hex[1];
    digits[2] = 0;
    return (uint8_t)strtol(digits, NULL, 16);
    /*
    Serial.print("(rhb");
    Serial.print(hex[0]);
    Serial.print(hex[1]);
    Serial.print("-");
    Serial.print(val);
    Serial.print(")");
    */
}

uint32_t read_hex_color(char* hex) {
    uint32_t val = 0;
    for (int i = 0; i < 3; ++i) {
      val <<= 8;
      val |= read_hex_byte(hex + 2*i);
    }
    return val;
}

// Given current time in milliseconds, row number (0-7) and pixel number
// along row (0 - (NUM_LEDS-1)), first calculate brightness (b) of pixel,
// then multiply row color by this and run it through NeoPixel library’s
// gamma-correction table.
uint32_t rain(uint32_t now, uint8_t row, int pixelNum) {
  uint8_t frame = now / 4; // uint8_t rolls over for a 0-255 range
  uint16_t b = 256 - ((frame - row * 32 + pixelNum * 256 / NUM_LEDS) & 0xFF);
  return leds.Color(leds.gamma8((demo_colors[row][0] * b) >> 8),
                    leds.gamma8((demo_colors[row][1] * b) >> 8),
                    leds.gamma8((demo_colors[row][2] * b) >> 8));
}
