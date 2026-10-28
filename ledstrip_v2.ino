#include <FastLED.h>

// =========================
// LED STRIP CONFIG
// =========================
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

// Right side strips
#define PIN_R9 2
#define PIN_R8 4
#define PIN_R6 6

// Left side strips
#define PIN_L9 3
#define PIN_L8 5
#define PIN_L6 7

// Inputs
#define BRAKE_PIN 8
#define RIGHT_PIN 9
#define LEFT_PIN 10

// Define LED strip lengths
#define LEN9 9
#define LEN8 8
#define LEN6 6

#define cometLength 4

// =========================
// BRIGHTNESS DEFINITION
// =========================
uint8_t RUN_BRIGHT   = 120;
uint8_t BRAKE_BRIGHT = 200;
uint8_t TURN_BRIGHT  = 200;

// =========================
// STRIPS DEFINITION
// =========================
CRGB r9[LEN9], r8[LEN8], r6[LEN6];
CRGB l9[LEN9], l8[LEN8], l6[LEN6];

// =========================
// INPUT FILTER
// =========================
struct Inp {
  uint8_t pin;
  bool state, last;
  unsigned long t;
};

// =========================
// SETTING INITIAL VALUES
// =========================
Inp brake = {BRAKE_PIN, false, false, 0};
Inp leftI = {LEFT_PIN, false, false, 0};
Inp rightI = {RIGHT_PIN, false, false, 0};

const int debounceMs = 30;

// =========================
// TURN STATE
// =========================
unsigned long leftHold = 0, rightHold = 0;
unsigned long leftCycle = 500, rightCycle = 500;
unsigned long leftRise = 0, rightRise = 0;

uint8_t leftStep = 0, rightStep = 0;

// =========================
// STARTUP ANIMATION STATE
// =========================
bool isStartingUp = true;
unsigned long startupStartTime = 0;
int startupStep = 0;

// =========================================================================
// GENERIC INPUT DEBOUNCER
// =========================================================================
void updateInput(Inp &input) {
  bool rawReading = (digitalRead(input.pin) == HIGH);

  if (rawReading != input.last) {
    input.t = millis();
  }

  if ((millis() - input.t) > debounceMs) {
    input.state = rawReading;
  }

  input.last = rawReading;
}

// ===========================
// CLEAR ALL LEDs (set black)
// ===========================
void clearAll() {
  fill_solid(r9, LEN9, CRGB::Black);
  fill_solid(r8, LEN8, CRGB::Black);
  fill_solid(r6, LEN6, CRGB::Black);
  fill_solid(l9, LEN9, CRGB::Black);
  fill_solid(l8, LEN8, CRGB::Black);
  fill_solid(l6, LEN6, CRGB::Black);
}

// =========================
// COMET FUNCTION
// =========================
void comet(CRGB* s, int len, int pos) {
  for(int i = 0; i < cometLength; i++){
    int p = (len - 1) - pos + i;
    
    if(p >= 0 && p < len) {
      s[p] = CRGB(200 / (i + 1), 0, 0); 
    }
  }
}

// =========================
// LAYOUT MAPPING TABLE
// =========================
const int8_t PROGMEM layoutMap[3][18] = {
  //   0   1   2   3   4   5   6   7   8  |  9  10  11  12  13  14  15  16  17   <- Virtual X
  {   0,  1,  2,  3,  4,  5,  6,  7,  8,    8,  7,  6,  5,  4,  3,  2,  1,  0 }, // Row 0 (Top)
  {  -1,  0,  1,  2,  3,  4,  5,  6,  7,    7,  6,  5,  4,  3,  2,  1,  0, -1 }, // Row 1 (Mid)
  {  -1, -1, -1,  0,  1,  2,  3,  4,  5,    5,  4,  3,  2,  1,  0, -1, -1, -1 }  // Row 2 (Bot)
};

// =========================================================================
// UNIFIED GRAPHICS MAPPING ENGINE
// =========================================================================
void setMatrixPixel(int x, int row, CRGB color) {
  if (x < 0 || x > 17 || row < 0 || row > 2) return;

  int8_t physicalIdx = pgm_read_byte(&(layoutMap[row][x]));

  if (physicalIdx == -1) return;

  if (x <= 8) {
    if (row == 0)      l9[physicalIdx] = color;
    else if (row == 1) l8[physicalIdx] = color;
    else if (row == 2) l6[physicalIdx] = color;
  } 
  else {
    if (row == 0)      r9[physicalIdx] = color;
    else if (row == 1) r8[physicalIdx] = color;
    else if (row == 2) r6[physicalIdx] = color;
  }
}

// =========================================================================
// STARTUP ANIMATION
// =========================================================================
void startupAnim() {
  unsigned long now = millis();
  
  // Smooth, deliberate pacing (75ms per step)
  if (now - startupStartTime >= 75) {
    startupStartTime = now;
    startupStep++;
  }

  FastLED.clear();

  uint8_t runB = 60; // Running lights dim baseline brightness

  // -------------------------------------------------------------------------
  // STEP 0: Bottom Outside Tip Lights Up Bright & Dims
  // -------------------------------------------------------------------------
  if (startupStep == 0) {
    setMatrixPixel(3, 2, CRGB(255, 0, 0));
    setMatrixPixel(14, 2, CRGB(255, 0, 0));
  }

  // -------------------------------------------------------------------------
  // STEP 1: Bottom Dims, Middle Outside Tip Lights Up Bright
  // -------------------------------------------------------------------------
  else if (startupStep == 1) {
    setMatrixPixel(3, 2, CRGB(runB, 0, 0));
    setMatrixPixel(14, 2, CRGB(runB, 0, 0));
    setMatrixPixel(1, 1, CRGB(255, 0, 0));
    setMatrixPixel(16, 1, CRGB(255, 0, 0));
  }

  // -------------------------------------------------------------------------
  // STEP 2: Middle Dims, Top Outside Tip Lights Up Bright
  // -------------------------------------------------------------------------
  else if (startupStep == 2) {
    setMatrixPixel(3, 2, CRGB(runB, 0, 0));
    setMatrixPixel(14, 2, CRGB(runB, 0, 0));
    setMatrixPixel(1, 1, CRGB(runB, 0, 0));
    setMatrixPixel(16, 1, CRGB(runB, 0, 0));
    setMatrixPixel(0, 0, CRGB(255, 0, 0));
    setMatrixPixel(17, 0, CRGB(255, 0, 0));
  }

  // -------------------------------------------------------------------------
  // STEPS 3 to 11: Top Row Outside-to-Inside Sweep
  // -------------------------------------------------------------------------
  else if (startupStep >= 3 && startupStep <= 11) {
    int headPos = startupStep - 3; // 0 to 8 (moving outside to inside)

    setMatrixPixel(3, 2, CRGB(runB, 0, 0));
    setMatrixPixel(14, 2, CRGB(runB, 0, 0));
    setMatrixPixel(1, 1, CRGB(runB, 0, 0));
    setMatrixPixel(16, 1, CRGB(255, 0, 0));

    for (int c = 0; c <= 8; c++) {
      if (c < headPos - cometLength) {
        if (c < 4) {
          setMatrixPixel(c, 0, CRGB(runB, 0, 0));
          setMatrixPixel(17 - c, 0, CRGB(runB, 0, 0));
        }
      } else if (c >= headPos - cometLength && c <= headPos && c >= 0) {
        int distFromHead = headPos - c;
        uint8_t bri = 255 / (distFromHead + 1);
        if (bri < runB) bri = runB;
        setMatrixPixel(c, 0, CRGB(bri, 0, 0));
        setMatrixPixel(17 - c, 0, CRGB(bri, 0, 0));
      }
    }
  }

  // -------------------------------------------------------------------------
  // STEPS 12+: Inside-to-Outside Comet Sweep Across All Rows with Blending
  // -------------------------------------------------------------------------
  else if (startupStep >= 12) {
    int sweepPos = startupStep - 12; // Distance from center (0 to 8+)

    // Once the comet and its tail have fully exited, hand off cleanly to main loop without blinking
    if (sweepPos > 8 + cometLength) {
      // Solid fill matching the exact running mode layout
      for(int i = 0; i < 4; i++) {
        l9[i] = CRGB(runB, 0, 0); 
        r9[i] = CRGB(runB, 0, 0); 
      }
      for(int i = 0; i < 3; i++) {
        l8[i] = CRGB(runB, 0, 0); 
        r8[i] = CRGB(runB, 0, 0); 
      }
      l6[0] = CRGB(runB, 0, 0); 
      r6[0] = CRGB(runB, 0, 0); 

      FastLED.show();
      isStartingUp = false;
      return;
    }

    // Iterate across all positions from center (p = 0) to outer edge (p = 8)
    for (int p = 0; p <= 8; p++) {
      int xLeft = 8 - p;   
      int xRight = 9 + p;  

      for (int row = 0; row < 3; row++) {
        bool isValid = false;
        if (row == 0 && xLeft >= 0 && xLeft <= 8) isValid = true;
        if (row == 1 && xLeft >= 1 && xLeft <= 8) isValid = true;
        if (row == 2 && xLeft >= 3 && xLeft <= 8) isValid = true;

        if (!isValid) continue;

        // 1. Identify final running light positions
        bool isFinalRunningLight = false;
        if (row == 0 && xLeft <= 3) isFinalRunningLight = true;              // Top row running lights ($x \le 3$)
        else if (row == 1 && xLeft >= 1 && xLeft <= 3) isFinalRunningLight = true; // Middle row running lights ($x = 1..3$)
        else if (row == 2 && xLeft == 3) isFinalRunningLight = true;         // Bottom row running light ($x = 3$)

        // 2. Define baseline behavior:
        // - AHEAD of the comet (p > sweepPos): Keep the initial full arrow visible (entire top row x <= 8, middle x = 1, bottom x = 3).
        // - BEHIND or AT the comet (p <= sweepPos): Transition to final running light state (only true running lights stay lit after comet tail passes).
        uint8_t baselineBri = 0;
        if (p > sweepPos) {
          if ((row == 0 && xLeft <= 8) || 
              (row == 1 && xLeft == 1) || 
              (row == 2 && xLeft == 3)) {
            baselineBri = runB;
          }
        } else {
          if (isFinalRunningLight && p <= sweepPos - cometLength) {
            baselineBri = runB;
          }
        }

        // 3. Calculate comet brightness based on distance from head
        int distFromHead = sweepPos - p;
        uint8_t cometBri = 0;
        if (distFromHead >= 0 && distFromHead < cometLength) {
          cometBri = 230 / (distFromHead + 1);
        }

        // 4. Blend: Pixel takes the brighter of the comet or the baseline
        uint8_t finalBri = (cometBri > baselineBri) ? cometBri : baselineBri;

        if (finalBri > 0) {
          setMatrixPixel(xLeft, row, CRGB(finalBri, 0, 0));
          setMatrixPixel(xRight, row, CRGB(finalBri, 0, 0));
        }
      }
    }
  }
}

// =========================
// SETUP
// =========================
void setup() {
  pinMode(BRAKE_PIN, INPUT);
  pinMode(LEFT_PIN, INPUT);
  pinMode(RIGHT_PIN, INPUT);

  FastLED.addLeds<LED_TYPE, PIN_R9, COLOR_ORDER>(r9, LEN9);
  FastLED.addLeds<LED_TYPE, PIN_R8, COLOR_ORDER>(r8, LEN8);
  FastLED.addLeds<LED_TYPE, PIN_R6, COLOR_ORDER>(r6, LEN6);

  FastLED.addLeds<LED_TYPE, PIN_L9, COLOR_ORDER>(l9, LEN9);
  FastLED.addLeds<LED_TYPE, PIN_L8, COLOR_ORDER>(l8, LEN8);
  FastLED.addLeds<LED_TYPE, PIN_L6, COLOR_ORDER>(l6, LEN6);

  clearAll();
  FastLED.show();
}

// =========================
// LOOP
// =========================
void loop() {
  if (isStartingUp) {
    startupAnim();
    FastLED.show();
    return;
  }

  updateInput(brake);
  updateInput(leftI);
  updateInput(rightI);

  unsigned long now = millis();

  static unsigned long lastLeftRiseTime = 0;
  static unsigned long lastRightRiseTime = 0;

  static bool leftLatch = false;
  static bool rightLatch = false;

  if (!leftI.state && leftI.last) {
    lastLeftRiseTime = now;
    leftStep = 0; 
  }
  if (leftI.state) {
    leftLatch = true;
    leftHold = now + 950; 
  }
  if (now > leftHold) {
    leftLatch = false;
  }

  if (!rightI.state && rightI.last) {
    lastRightRiseTime = now;
    rightStep = 0; 
  }
  if (rightI.state) {
    rightLatch = true;
    rightHold = now + 950;
  }
  if (now > rightHold) {
    rightLatch = false;
  }

  bool isHazard = false;
  if ((now - lastLeftRiseTime < 850) && (now - lastRightRiseTime < 850)) {
    isHazard = true;
  }

  clearAll(); 

  // LEFT SIDE TAILLIGHT (Brake has absolute priority)
  if (brake.state) {
    fill_solid(l9, LEN9, CRGB(255, 0, 0));
    fill_solid(l8, LEN8, CRGB(255, 0, 0));
    fill_solid(l6, LEN6, CRGB(255, 0, 0));
  } 
  else if (isHazard || leftLatch) {
    comet(l9, LEN9, leftStep);
    comet(l8, LEN8, leftStep);
    comet(l6, LEN6, leftStep);
  } 
  else {
    for(int i = 0; i < 4; i++) l9[i] = CRGB(60, 0, 0); 
    for(int i = 0; i < 3; i++) l8[i] = CRGB(60, 0, 0); 
    l6[0] = CRGB(60, 0, 0); 
  }

  // RIGHT SIDE TAILLIGHT (Brake has absolute priority)
  if (brake.state) {
    fill_solid(r9, LEN9, CRGB(255, 0, 0));
    fill_solid(r8, LEN8, CRGB(255, 0, 0));
    fill_solid(r6, LEN6, CRGB(255, 0, 0));
  } 
  else if (isHazard || rightLatch) {
    comet(r9, LEN9, rightStep);
    comet(r8, LEN8, rightStep);
    comet(r6, LEN6, rightStep);
  } 
  else {
    for(int i = 0; i < 4; i++) r9[i] = CRGB(60, 0, 0); 
    for(int i = 0; i < 3; i++) r8[i] = CRGB(60, 0, 0); 
    r6[0] = CRGB(60, 0, 0); 
  }

  static unsigned long lastStepTime = 0;
  int maxSteps = 8 + cometLength;
  
  if (now - lastStepTime >= 40) {
    lastStepTime = now;

    if (leftLatch || isHazard) {
      if (leftStep < maxSteps) leftStep++; 
    } else {
      leftStep = 0;
    }

    if (rightLatch || isHazard) {
      if (rightStep < maxSteps) rightStep++;
    } else {
      rightStep = 0;
    }
  }

  FastLED.show();
}
