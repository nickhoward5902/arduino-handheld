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

class ComponentDot {
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

class SpriteO {
  public:
    ComponentDot PO1;
    ComponentDot PO2;
    ComponentDot PO3;
    ComponentDot PO4;
    ComponentDot PO5;
    ComponentDot PO6;
    ComponentDot PO7;
    ComponentDot PO8;
    ComponentDot PO9;
    ComponentDot POA;
    ComponentDot POB;
    ComponentDot POC;
    int x = 0;
    int y = 0; //coordinates of lower left corner of sprite
    void setPosition(int x1, int y1) {
      x = x1;
      y = y1;
      /**
      x 1 2 x
      3 4 5 6
      7 8 9 A
      x B C x
      **/
      PO1.setPosition((x+1), (y+3));
      PO2.setPosition((x+2), (y+3));
      PO3.setPosition(x, (y+2));
      PO4.setPosition((x+1), (y+2));
      PO5.setPosition((x+2), (y+2));
      PO6.setPosition((x+3), (y+2));
      PO7.setPosition(x, (y+1));
      PO8.setPosition((x+1), (y+1));
      PO9.setPosition((x+2), (y+1));
      POA.setPosition((x+3), (y+1));
      POB.setPosition((x+1), y);
      POC.setPosition((x+2), y);
      if (debug == true) {
        serialBuffer();
      }
    }
    void spriteRight() {
      PO6.dotRight();
      POA.dotRight();
      PO2.dotRight();
      PO5.dotRight();
      PO9.dotRight();
      POC.dotRight();
      PO1.dotRight();
      PO4.dotRight();
      PO8.dotRight();
      POB.dotRight();
      PO3.dotRight();
      PO7.dotRight();
    }
    void spriteLeft() {
      PO7.dotLeft();
      PO3.dotLeft();
      POB.dotLeft();
      PO8.dotLeft();
      PO4.dotLeft();
      PO1.dotLeft();
      POC.dotLeft();
      PO9.dotLeft();
      PO5.dotLeft();
      PO2.dotLeft();
      POA.dotLeft();
      PO6.dotLeft();
    }
    void spriteUp() {
      PO1.dotUp();
      PO2.dotUp();
      PO3.dotUp();
      PO4.dotUp();
      PO5.dotUp();
      PO6.dotUp();
      PO7.dotUp();
      PO8.dotUp();
      PO9.dotUp();
      POA.dotUp();
      POB.dotUp();
      POC.dotUp();
    }
    void spriteDown() {
      POC.dotDown();
      POB.dotDown();
      POA.dotDown();
      PO9.dotDown();
      PO8.dotDown();
      PO7.dotDown();
      PO6.dotDown();
      PO5.dotDown();
      PO4.dotDown();
      PO3.dotDown();
      PO2.dotDown();
      PO1.dotDown();
    }
};

class SOControl{
  public:
    void moveLeft(SpriteO SO) {
      int x = SO.x - 1;
      int y = SO.y;
      SO.spriteLeft();
      if (debug == true) {
        serialBuffer();
      }
    }
    void moveRight(SpriteO SO) {
      int x = SO.x + 1;
      int y = SO.y;
      SO.spriteRight();
      if (debug == true) {
        serialBuffer();
      }
    }
    void moveUp(SpriteO SO) {
      int x = SO.x;
      int y = SO.y + 1;
      SO.spriteUp();
      if (debug == true) {
        serialBuffer();
      }
    }
    void moveDown(SpriteO SO) {
      int x = SO.x;
      int y = SO.y - 1;
      SO.spriteDown();
      if (debug == true) {
        serialBuffer();
      }
    }
};

class SpriteX {
  public:
    ComponentDot PX1;
    ComponentDot PX2;
    ComponentDot PX3;
    ComponentDot PX4;
    ComponentDot PX5;
    ComponentDot PX6;
    ComponentDot PX7;
    ComponentDot PX8;
    int x = 0;
    int y = 0; //coordinates of lower left corner of sprite
    void setPosition(int x1, int y1) {
      x = x1;
      y = y1;
      /**
      1 x x 2
      x 3 4 x
      x 5 6 x
      7 x x 8
      **/
      PX1.setPosition(x, (y+3));
      PX2.setPosition((x+3), (y+3));
      PX3.setPosition((x+1), (y+2));
      PX4.setPosition((x+2), (y+2));
      PX5.setPosition((x+1), (y+1));
      PX6.setPosition((x+2), (y+1));
      PX7.setPosition(x, y);
      PX8.setPosition((x+3), y);
      if (debug == true) {
        serialBuffer();
      }
    }
    void spriteRight() {
      PX2.dotRight();
      PX8.dotRight();
      PX4.dotRight();
      PX6.dotRight();
      PX3.dotRight();
      PX5.dotRight();
      PX1.dotRight();
      PX7.dotRight();
    }
    void spriteLeft() {
      PX7.dotLeft();
      PX1.dotLeft();
      PX5.dotLeft();
      PX3.dotLeft();
      PX6.dotLeft();
      PX4.dotLeft();
      PX8.dotLeft();
      PX2.dotLeft();
    }
    void spriteUp() {
      PX1.dotUp();
      PX2.dotUp();
      PX3.dotUp();
      PX4.dotUp();
      PX5.dotUp();
      PX6.dotUp();
      PX7.dotUp();
      PX8.dotUp();
    }
    void spriteDown() {
      PX8.dotDown();
      PX7.dotDown();
      PX6.dotDown();
      PX5.dotDown();
      PX4.dotDown();
      PX3.dotDown();
      PX2.dotDown();
      PX1.dotDown();
    }
};

class SXControl{
  public:
    void moveLeft(SpriteX SX) {
      int x = SX.x - 1;
      int y = SX.y;
      SX.spriteLeft();
      if (debug == true) {
        serialBuffer();
      }
    }
    void moveRight(SpriteX SX) {
      int x = SX.x + 1;
      int y = SX.y;
      SX.spriteRight();
      if (debug == true) {
        serialBuffer();
      }
    }
    void moveUp(SpriteX SX) {
      int x = SX.x;
      int y = SX.y + 1;
      SX.spriteUp();
      if (debug == true) {
        serialBuffer();
      }
    }
    void moveDown(SpriteX SX) {
      int x = SX.x;
      int y = SX.y - 1;
      SX.spriteDown();
      if (debug == true) {
        serialBuffer();
      }
    }
};

/**
    Avoid use of LedControl functions over custom functions, or buffer functions over display functions; this will not update the frame buffer and may cause some logic issues
*/

SpriteX SX0;
SpriteO SO0;
SXControl SXX0;
SOControl SOX0;

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
  SO0.setPosition(6,6);
  SX0.setPosition(22,6);
}

void loop() {
  if (debug == true) {
        Serial.println("loop()");
      }
  int randC = random(2,7); // double bias for x-axis as it is twice the size of the y-axis
  switch (randC) {
    case 1:
      SOX0.moveUp(SO0);
      break;
    case 2:
    case 3:
      SOX0.moveLeft(SO0);
      break;
    case 4:
    case 5:
      SOX0.moveRight(SO0);
      break;
    case 6:
      SOX0.moveDown(SO0);
      break;
  }
  int randX = random(2,7);
  switch (randX) {
    case 1:
      SXX0.moveUp(SX0);
      break;
    case 2:
    case 3:
      SXX0.moveLeft(SX0);
      break;
    case 4:
    case 5:
      SXX0.moveRight(SX0);
      break;
    case 6:
      SXX0.moveDown(SX0);
      break;
  }
  delay(500);
}