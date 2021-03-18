#include <LedControl.h>

// d1 - 16x32
// display.setLed(display, row, column, on)
// d, x, y coordinates start from 0, not 1 (0d-7d, 0x-31x, 0y-15y)

const bool debug = false;

const byte displaycheck [16][4] {
  {B00000000, B00000000, B00000000, B00000000},
  {B01111100, B01111100, B00001000, B00111100},
  {B00000010, B00000010, B00011000, B01000110},
  {B01111100, B00111100, B00001000, B01011010},
  {B00000010, B01000000, B00001000, B01100010},
  {B01111100, B01111110, B00111110, B00111100},
  {B00000000, B00000000, B00000000, B00000000},
  {B00000000, B00000000, B00000000, B00000000},
  {B00000000, B00000000, B00000000, B00000000},
  {B01111110, B00111110, B01111110, B00011000},
  {B00000100, B01000000, B01000000, B00101000},
  {B00111110, B01111100, B00111100, B01001000},
  {B00010000, B01000010, B00000010, B01111110},
  {B00100000, B00111100, B01111100, B00001000},
  {B00000000, B00000000, B00000000, B00000000},
  {B00000000, B00000000, B00000000, B00000000}
};

const byte border [16][4] {
  {B11111111, B11111111, B11111111, B11111111},
  {B10000000, B00000001, B00000000, B00000001},
  {B10000000, B00000000, B10000000, B00000001},
  {B10000000, B00000001, B00000000, B00000001},
  {B10000000, B00000000, B10000000, B00000001},
  {B10000000, B00000001, B00000000, B00000001},
  {B10000000, B00000000, B10000000, B00000001},
  {B10000000, B00000001, B00000000, B00000001},
  {B10000000, B00000000, B10000000, B00000001},
  {B10000000, B00000001, B00000000, B00000001},
  {B10000000, B00000000, B10000000, B00000001},
  {B10000000, B00000001, B00000000, B00000001},
  {B10000000, B00000000, B10000000, B00000001},
  {B10000000, B00000001, B00000000, B00000001},
  {B10000000, B00000000, B10000000, B00000001},
  {B11111111, B11111111, B11111111, B11111111}
};

/**
   _________________
   | 3 | 2 | 1 | 0 |
   _________________
   | 7 | 6 | 5 | 4 |
   _________________
*/

class FrameBuffer {
  public:
    bool buff[16][32];
    void initBuffer() { // sets or resets the frame buffer to empty
      for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 32; x++) {
          buff[y][x] = false;
        }
      }
    }
    void fillBuffer() { // sets or resets the frame buffer to empty
      for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 32; x++) {
          buff[y][x] = true;
        }
      }
    }
    void togglePoint(int x, int y) {
      buff[15 - y][x] = !buff[y][x];
    }
    bool getPoint(int x, int y) {
      return buff[y][x];
    }
    void setPoint(int x, int y, bool z) {
      buff[15 - y][x] = z;
    }
    void setByte(int c, int y, byte b) {
      int x;
      while (c > 3) { // overflow protection
        c -= 4;
      }
      if (c == 0) { // adjusts byte x coordinate to bit x coordinate
        x = 0;
      }
      else if (c == 1) {
        x = 8;
      }
      else if (c == 2) {
        x = 16;
      }
      else if (c == 3) {
        x = 24;
      }
      for (int i = 0; i < 8; i++) {
        if (bitRead(b, 7 - i) == true) {
          buff[15 - y][x + i] = true;
        }
        else {
          buff[15 - y][x + i] = false;
        }
      }
    }
    byte getByte(int c, int y) {
      byte b = 0;
      int x;
      while (c > 3) { // overflow protection
        c -= 4;
      }
      if (c == 0) { // adjusts byte x coordinate to bit x coordinate
        x = 0;
      }
      else if (c == 1) {
        x = 8;
      }
      else if (c == 2) {
        x = 16;
      }
      else if (c == 3) {
        x = 24;
      }
      if (buff[y][x] == true) { // converts boolean bits to byte value
        b += 128;
      }
      if (buff[y][x + 1] == true) {
        b += 64;
      }
      if (buff[y][x + 2] == true) {
        b += 32;
      }
      if (buff[y][x + 3] == true) {
        b += 16;
      }
      if (buff[y][x + 4] == true) {
        b += 8;
      }
      if (buff[y][x + 5] == true) {
        b += 4;
      }
      if (buff[y][x + 6] == true) {
        b += 2;
      }
      if (buff[y][x + 7] == true) {
        b += 1;
      }
      return b;
    }
    void getRowSerial(int y) {
      String serialOut = "";
      for (int i = 0; i < 32; i++) {
        if (buff[15 - y][i] == false) {
          serialOut += "▒";
        }
        else {
          serialOut += "█";
        }
      }
      Serial.println(serialOut);
    }
};

FrameBuffer F0;
LedControl D0 = LedControl(12, 13, 11, 8); // DIN, CLK, CS, number of MAX72XXs

void serialBuffer() {
  Serial.println("==FRAME==");
  for (int i = 0; i < 16; i++) {
    F0.getRowSerial(i);
  }
  Serial.println("==END==");
}

void displayReset() {
  F0.initBuffer();
  for (int i = 0; i < 8; i++) {
    D0.shutdown(i, false); // wakes each display from low-power mode
    D0.setIntensity(i, 0); // sets display brightness (0-15)
    D0.clearDisplay(i); // sets all leds to off
  }
  if (debug == true) {
        serialBuffer();
      }
}

void pixelToggle(int x, int y, bool isOn) { // converts a 32x16y value into 4dx2dy8x8y
  int d;
  F0.setPoint(x, 15 - y, isOn);
  if (x > 7) {
    d = 6;
    x -= 8;
    if (x > 7) {
      d = 5;
      x -= 8;
      if (x > 7) {
        d = 4;
        x -= 8;
      }
    }
  }
  else {
    d = 7;
  }
  if (y > 7) {
    d -= 4;
    y -= 8;
  }
  D0.setLed(d, 7 - y, x, isOn); // adjustment for horizontal by inverting coordinates
}

void fill(bool isOn) {
  if (isOn == true) {
    F0.fillBuffer();
    for (int y = 0; y < 16; y++) {
      for (int d = 0; d < 8; d++) {
        D0.setRow(d, y, B11111111);
      }
    }
  }
  else {
    F0.initBuffer();
    for (int d = 0; d < 8; d++) {
      D0.clearDisplay(d);
    }
  }
  if (debug == true) {
        serialBuffer();
      }
}

void flowFill(bool isOn, int delayTime) {
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 32; x++) {
      pixelToggle(x, y, isOn);
      delay(delayTime);
    }
  }
  if (debug == true) {
        serialBuffer();
      }
}

void displayFrame(byte frame [16][4]) {
  for (int r = 0; r < 16; r++) {
    for (int c = 0; c < 4; c++) {
      byte data = frame[r][c];
      F0.setByte(c, r, data);
      if (r < 8) {
        D0.setRow(3 - c, r, data);
      }
      else if (r > 7) {
        int d = c + 4;
        int q = r - 8;
        D0.setRow(11 - d, q, data);
      }
    }
  }
  if (debug == true) {
        serialBuffer();
      }
}

class PlayerDot {
  public:
    int x = -1;
    int y = -1;
    int xPrev = 0;
    int yPrev = 0;
    void setPosition(int x1, int y1) {
      xPrev = x;
      yPrev = y;
      x = x1;
      y = y1;
      while ((x > 31) or (x < 0) or (y > 15) or (y < 0)) { //overflow protection
        if (x > 31) {
          x -= 32;
        }
        if (x < 0) {
          x += 32;
        }
        if (y > 15) {
          y -= 16;
        }
        if (y < 0) {
          y += 16;
        }
      }
      pixelToggle(xPrev, yPrev, false);
      pixelToggle(x, y, true);
    }
    void dotLeft() {
      xPrev = x;
      yPrev = y;
      x -= 1;
      if (x == -1) {
        x = 31;
      }
      pixelToggle(xPrev, yPrev, false);
      pixelToggle(x, y, true);
    }
    void dotRight() {
      xPrev = x; //saves old value of x for turning off
      yPrev = y;
      x += 1; //adds to create new value of x
      if (x == 32) {
        x = 0; //overflow correction
      }
      pixelToggle(xPrev, yPrev, false); //turns off old pixel
      pixelToggle(x, y, true); //turns on new pixel
    }
    void dotUp() {
      xPrev = x;
      yPrev = y;
      y += 1;
      if (y == 16) {
        y = 0;
      }
      pixelToggle(xPrev, yPrev, false);
      pixelToggle(x, y, true);
    }
    void dotDown() {
      xPrev = x;
      yPrev = y;
      y -= 1;
      if (y == -1) {
        y = 15;
      }
      pixelToggle(x, y, true);
      pixelToggle(xPrev, yPrev, false);
    }
};

class PlayerSprite {
  public:
    int x = 0;
    int y = 0; //coordinates of bottom left corner of sprite
    PlayerDot P1;
    PlayerDot P2;
    PlayerDot P3;
    PlayerDot P4;
    PlayerDot P5;
    PlayerDot P6;
    PlayerDot P7;
    PlayerDot P8;
    PlayerDot DotArrayH[8] = {P2, P8, P4, P6, P3, P5, P1, P7};
    PlayerDot DotArrayV[8] = {P7, P8, P5, P6, P3, P4, P1, P2};
    void setPosition(int x1, int y1) {
      x = x1;
      y = y1;
      P1.setPosition(x, y);
      P2.setPosition((x + 3), y);
      P3.setPosition((x + 1), (y + 1));
      P4.setPosition((x + 2), (y + 1));
      P5.setPosition((x + 1), (y + 2));
      P6.setPosition((x + 2), (y + 2));
      P7.setPosition(x, (y + 3));
      P8.setPosition((x + 3), (y + 3));
      /**
         7xx8
         x56x
         x34x
         1xx2
      */
      if (debug == true) {
        serialBuffer();
      }
    }
    void spriteRight() {
      P2.dotRight();
      P8.dotRight();
      P4.dotRight();
      P6.dotRight();
      P3.dotRight();
      P5.dotRight();
      P1.dotRight();
      P7.dotRight();
    }
    void spriteLeft() {
      //non functional iteration
      /**for (int i = 0; i < 8; i++) {
        PlayerDot s = DotArrayH[i];
        s.dotLeft();
        }*/
      P7.dotLeft();
      P1.dotLeft();
      P5.dotLeft();
      P3.dotLeft();
      P6.dotLeft();
      P4.dotLeft();
      P8.dotLeft();
      P2.dotLeft();
    }
    void spriteUp() {
      P7.dotUp();
      P8.dotUp();
      P5.dotUp();
      P6.dotUp();
      P3.dotUp();
      P4.dotUp();
      P1.dotUp();
      P2.dotUp();
    }
    void spriteDown() {
      P2.dotDown();
      P1.dotDown();
      P4.dotDown();
      P3.dotDown();
      P6.dotDown();
      P5.dotDown();
      P8.dotDown();
      P7.dotDown();
    }
};

PlayerSprite S0;

class PlayerControl{
  public:
    void moveLeft() {
      int x = S0.x - 1;
      int y = S0.y;
      S0.spriteLeft();
      if (debug == true) {
        serialBuffer();
      }
    }
    void moveRight() {
      int x = S0.x + 1;
      int y = S0.y;
      S0.spriteRight();
      if (debug == true) {
        serialBuffer();
      }
    }
    void moveUp() {
      int x = S0.x;
      int y = S0.y + 1;
      S0.spriteUp();
      if (debug == true) {
        serialBuffer();
      }
    }
    void moveDown() {
      int x = S0.x;
      int y = S0.y - 1;
      S0.spriteDown();
      if (debug == true) {
        serialBuffer();
      }
    }
};

/**
    Avoid use of LedControl functions over custom functions, or buffer functions over display functions; this will not update the frame buffer and may cause some logic issues
*/

PlayerControl X0;

void setup() {
  if (debug == true) {
    Serial.begin(115200); // serial channel for debugging (high baud rate to avoid significant slowdown)
    Serial.println("setup()");
  }
  F0.initBuffer();
  displayReset();
  displayFrame(displaycheck);
  delay(1000);
  displayReset();
  S0.setPosition(14,6);
}

void loop() {
  if (debug == true) {
        Serial.println("loop()");
      }
  int rand = random(1,7);
  switch (rand) {
    case 1:
      X0.moveUp();
      break;
    case 2 || 3:
      X0.moveLeft();
      break;
    case 4 || 5:
      X0.moveRight();
      break;
    case 6:
      X0.moveDown();
      break;
  }
  delay(100);
}