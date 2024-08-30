#include <Adafruit_NeoPXL8.h>

#define NUM_LEDS    180      // NeoPixels PER STRAND, total number is 8X this!
#define NUM_LEDS_ALL ((NUM_LEDS) * 8)
#define COLOR_ORDER NEO_GRB // NeoPixel color format (see Adafruit_NeoPixel)
#define MS_PER_FRAME 20

// For the Feather RP2040 SCORPIO, use this list:
int8_t pins[8] = { 16, 17, 18, 19, 20, 21, 22, 23 };

Adafruit_NeoPXL8 leds(NUM_LEDS, pins, COLOR_ORDER);
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

enum modes {
  INIT = 0,
  MANUAL = 1,
  FIRST_DEMO = 2,
  RAIN_DEMO = 2,
  RAINBOW_WAVE_DEMO = 3,
  SPARKLE_FADE_DEMO = 4,
  SLOW_RAINBOW_DEMO = 5,
  N_MODES,
};

modes mode = INIT;
modes DEFAULT_DEMO_MODE = SLOW_RAINBOW_DEMO;
int framerate = MS_PER_FRAME;

void setup() {
  // The baud rate is seemingly ignored by the Feather / RP2040 simulated UART, which seems to always run at line speed.
  // I observe just under 1 Mbps, but that's ignoring all overheads.
  Serial.begin(2000000);
  /*
  while (!Serial) {
S  }
  // XXX: fine to come up without serial, just cope with it
  */
  Serial.println("LED controller starting up...");

  pinMode(LED_BUILTIN, OUTPUT);
  flashfor(30, 30, 3);

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

  // XXX: fix this
  // leds.setBrightness(10); // Tone it down, NeoPixels are BRIGHT!
  // leds.setBrightness(255); // I believe this is the default?
  // pixels = leds.getPixels();
  // XXX: DON'T DO THIS
  //leds.setLatchTime(200);

  // Cycle all pixels red/green/blue on startup. If you see a different
  // sequence, COLOR_ORDER doesn't match your particular NeoPixel type.
  // If you get a jumble of colors, you're using RGBW NeoPixels with an
  // RGB order. Try different COLOR_ORDER values until code and hardware
  // are in harmony.
  rgbtest();
  black();
  delay(200);

  Serial.println("Testing strips (should flash each strip in sequence.)");
  seqtest();

  Serial.println("Startup complete.");
  mode = DEFAULT_DEMO_MODE;
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
  if (mode == RAIN_DEMO) {
    raindemo();
  } else if (mode == RAINBOW_WAVE_DEMO) {
    rainbowWave();
  } else if (mode == SPARKLE_FADE_DEMO) {
    sparkleFade();
  } else if (mode == SLOW_RAINBOW_DEMO) {
    slowRainbow();
  }

  if (millis() - lastDataReceived > TIMEOUT) {
    serialOkay = 0;
    /*
    // Connection might be stuck, try to reset it
    flashfor(100, 100, 3);
    Serial.end();
    delay(100);
    Serial.begin(2000000);
    Serial.println("nobody is listening, does this kill the line");
    while (!Serial) {
    }
    flashfor(100, 100, 3);
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
  while (mode == MANUAL && serialOkay && !Serial.available()) {
    if (millis() - lastDataReceived > TIMEOUT) {
      serialOkay = 0;
      mode = DEFAULT_DEMO_MODE;
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
    // we lost the serial port. :-(
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
  if (buf[0] == 'h' || buf[0] == '?') {
    Serial.println("COMMANDS (all commands are a line followed by \\n):");
    Serial.println(" h / ? - help");
    Serial.println(" d[hh] - set demo mode (two hex digits for demo number, or omit for default)");
    Serial.println(" *[hh[hh[hh...]]] -- directly set LED colors (two hex digits per channel R/G/B, three channels per LED) -- after 1s without direct LED set commands, will default back to demo mode.");
    Serial.println(" @[c[c[c...]]] -- directly set LED colors (binary, one byte per channel R/G/B, three channels per LED, do not use the character \\n because I suck, timeout as above.)");
    Serial.println(" %[c[c[c...]]] -- directly set LED colors (binary, one byte per pixel 2 bits R / 3 bits G / 3 bits B, do not use the character \\n because I suck, timeout as above.)");
    Serial.println(" b[hh] - set brightness (two hex digits for level)");
    Serial.println(" #... -- line is ignored.");
  } else if (buf[0] == 'f') {  // framerate in ms
    if (buf[1] == '\n') {
      framerate = MS_PER_FRAME;
    } else {
      framerate = read_hex_byte(buf+1);
    }
  } else if (buf[0] == 'd') {  // go back to demo mode
    if (buf[1] == '\n') {
      mode = DEFAULT_DEMO_MODE;
    } else {
      int newmode = read_hex_byte(buf+1);
      if (newmode >= FIRST_DEMO && newmode < N_MODES) {
        Serial.print("Switching to demo mode number:");
        Serial.println(newmode);
        mode = (modes)newmode;
      } else {
        Serial.print("Bad demo mode number:");
        Serial.print(newmode);
        Serial.print("; valid modes are from ");
        Serial.print(FIRST_DEMO);
        Serial.print(" to ");
        Serial.print(N_MODES-1);
        Serial.println(".");
      }
    }
  } else if (buf[0] == '*') {  // set all lights
    mode = MANUAL;
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
    mode = MANUAL;
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
    mode = MANUAL;
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
    // doesn't change mode
    Serial.println("ignoring comment.");
  } else if (buf[0] == 'b') {
    // doesn't change mode
    int brightness = read_hex_byte(buf+1);
    Serial.print("Setting LED brightness to:");
    Serial.println(brightness);
    leds.setBrightness(brightness);
  } else {
    // doesn't change mode
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

void black() {
  leds.fill(0);
  leds.show();
}

void rgbtest() {
  for (uint32_t color = 0xFF0000; color > 0; color >>= 8) {
    leds.fill(color);
    leds.show();
    flashfor(100, 100);  // delay 200
  }
}

void seqtest() {
  // Light each strip in sequence. This helps verify the mapping of pins to
  // a physical LED installation. If strips flash out of sequence, you can
  // either re-wire, or just change the order of the pins[] array.
  for (int i=0; i<8; i++) {
    if (pins && (pins[i] < 0)) continue; // No pixels on this pin
    leds.fill(0);
    uint32_t color = leds.Color(demo_colors[i][0], demo_colors[i][1], demo_colors[i][2]);
    leds.fill(color, i * NUM_LEDS, NUM_LEDS);
    leds.show();
    flashfor(500, 100);  // delay 200
  }
}

void raindemo() {
  uint32_t now = millis(); // Get time once at start of each frame
  int frame = now * 20 / framerate;

  for(uint8_t r=0; r<8; r++) { // For each row...
    for(int p=0; p<NUM_LEDS; p++) { // For each pixel of row...
      leds.setPixelColor(r * NUM_LEDS + p, rain(now, r, p));
    }
  }
  leds.show();
}

// thanks Claude for this one
void rainbowWave() {
  uint32_t now = millis(); // Get time once at start of each frame
  int frame = now / framerate;

  for (int string = 0; string < 8; ++string) {
    for (int i = 0; i < NUM_LEDS; i++) {
      float hue = fmodf((float)(i + frame) / 30.0f, 1.0f);
      float x = 1.0f - fabsf(fmodf(hue * 6.0f, 2.0f) - 1.0f);
      unsigned char r, g, b;

      if (hue < 1.0f/6.0f)      { r = 255; g = 255 * x; b = 0; }
      else if (hue < 2.0f/6.0f) { r = 255 * x; g = 255; b = 0; }
      else if (hue < 3.0f/6.0f) { r = 0; g = 255; b = 255 * x; }
      else if (hue < 4.0f/6.0f) { r = 0; g = 255 * x; b = 255; }
      else if (hue < 5.0f/6.0f) { r = 255 * x; g = 0; b = 255; }
      else                      { r = 255; g = 0; b = 255 * x; }

      leds.setPixelColor(string * NUM_LEDS + i, r, g, b);
    }
  }
  leds.show();
}

int spherePanelOrder[] = { 2, 4, 5, 3, 1, 0, 6, 7 };

void slowRainbow() {
  uint32_t now = millis();
  int frame = now / framerate;

  for (int i = 0; i < NUM_LEDS_ALL; i++) {
    float hue = fmodf((float)(i + frame) / (180.0f * 8.0f), 1.0f);
    float x = 1.0f - fabsf(fmodf(hue * 6.0f, 2.0f) - 1.0f);
    unsigned char r, g, b;

    if (hue < 1.0f/6.0f)      { r = 255; g = 255 * x; b = 0; }
    else if (hue < 2.0f/6.0f) { r = 255 * x; g = 255; b = 0; }
    else if (hue < 3.0f/6.0f) { r = 0; g = 255; b = 255 * x; }
    else if (hue < 4.0f/6.0f) { r = 0; g = 255 * x; b = 255; }
    else if (hue < 5.0f/6.0f) { r = 255 * x; g = 0; b = 255; }
    else                      { r = 255; g = 0; b = 255 * x; }

    int string = i / NUM_LEDS;
    leds.setPixelColor(NUM_LEDS * spherePanelOrder[string] + (i % NUM_LEDS), r, g, b);
  }
  leds.show();
}

// Demo 2: Sparkle Fade
#define MAX_SPARKLES 5000
int sparkle_positions[MAX_SPARKLES];
int sparkle_brightness[MAX_SPARKLES];

void sparkleFade() {
    uint32_t now = millis(); // Get time once at start of each frame
    int frame = now / framerate;

    // Clear the strip
    leds.fill(0);

    // Update existing sparkles
    for (int i = 0; i < MAX_SPARKLES; i++) {
        if (sparkle_brightness[i] > 0) {
            sparkle_brightness[i] -= 10;
            if (sparkle_brightness[i] < 0) sparkle_brightness[i] = 0;
            leds.setPixelColor(sparkle_positions[i], sparkle_brightness[i], sparkle_brightness[i], sparkle_brightness[i]);
        }
    }

    // Add new sparkles
    if (frame % 5 == 0) {
        for (int i = 0; i < MAX_SPARKLES; i++) {
            if (sparkle_brightness[i] == 0) {
                sparkle_positions[i] = rand() % NUM_LEDS_ALL;
                sparkle_brightness[i] = 255;
                break;
            }
        }
    }
    leds.show();
}

void flashfor(int duration) {
  flashfor(duration, 0);
}

void flashfor(int duration, int pause) {
  flashfor(duration, pause, 1);
}

void flashfor(int duration, int pause, int count) {
  for (size_t i = 0; i < count; ++i) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(duration);
    digitalWrite(LED_BUILTIN, LOW);
    delay(pause);
  }
}
