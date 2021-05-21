#include <LedControl.h>
#include <hardwareSerial.h>

// d1 - 16x32
// display.setLed(display, row, column, on)
// d, x, y coordinates start from 0, not 1 (0d-7d, 0x-31x, 0y-15y)

const bool debug = false; //set as true to return serial images through the COM port; completely annihilates performance and slows everything to a crawl
const unsigned int movementDelay = 50; //delay in ms between movements in control models
//movement starts producing trails below 10ms, processing limit seems to be around 0.4ms for a single point

//Disclaimer - debug output works fine with the Arduino IDE's serial monitor, but is heavily corrupted and garbled if used with VSCode's serial monitor

byte displaycheck [16][4] {
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

byte border [16][4] {
  {B11111111, B11111111, B11111111, B11111111},
  {B10000000, B00000000, B00000000, B00000001},
  {B10000000, B00000000, B00000000, B00000001},
  {B10000000, B00000000, B00000000, B00000001},
  {B10000000, B00000000, B00000000, B00000001},
  {B10000000, B00000000, B00000000, B00000001},
  {B10000000, B00000000, B00000000, B00000001},
  {B10000000, B00000000, B00000000, B00000001},
  {B10000000, B00000000, B00000000, B00000001},
  {B10000000, B00000000, B00000000, B00000001},
  {B10000000, B00000000, B00000000, B00000001},
  {B10000000, B00000000, B00000000, B00000001},
  {B10000000, B00000000, B00000000, B00000001},
  {B10000000, B00000000, B00000000, B00000001},
  {B10000000, B00000000, B00000000, B00000001},
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
LedControl D0 = LedControl(4, 5, 3, 8); // DIN, CLK, CS, number of MAX72XXs

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
  if ((x >= 0) and (y >= 0)) {
    F0.setPoint(x, 15 - y, isOn);
  }
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

class SpriteDot {
  public:
    ComponentDot P1;
    int x = 0;
    int y = 0; //coordinates of lower left corner of sprite
    void setPosition(int x1, int y1) {
      x = x1;
      y = y1;
      /**
        1
      **/
      P1.setPosition(x, y);
      if (debug == true) {
        serialBuffer();
      }
    }
    void spriteRight() {
      P1.dotRight();
    }
    void spriteLeft() {
      P1.dotLeft();
    }
    void spriteUp() {
      P1.dotUp();
    }
    void spriteDown() {
      P1.dotDown();
    }
};

class SpriteDotControl {
  private:
    SpriteDot CONTROL;
  public:
    void initialise(SpriteDot SD, int x, int y) {
      CONTROL = SD;
      bool isOccupied = F0.getPoint(x, y);
      if (isOccupied == true) {
        // Do nothing
        Serial.println("Wall detected [initialisation]");
      }
      else {
        CONTROL.setPosition(x, y);
      }
    }
    void moveLeft() {
      int x = CONTROL.x - 1;
      int y =  CONTROL.y;
      bool isOccupied = F0.getPoint(x, y);
      if (isOccupied == true) {
        // Do nothing
        Serial.println("Wall detected [left]");
      }
      else {
        CONTROL.spriteLeft();
      }
      if (debug == true) {
        serialBuffer();
      }
      delay(movementDelay);
    }
    void moveRight() {
      int x = CONTROL.x + 1;
      int y = CONTROL.y;
      bool isOccupied = F0.getPoint(x, y);
      if (isOccupied == true) {
        // Do nothing
        Serial.println("Wall detected [right]");
      }
      else {
        CONTROL.spriteRight();
      }
      if (debug == true) {
        serialBuffer();
      }
      delay(movementDelay);
    }
    void moveUp() {
      int x = CONTROL.x;
      int y = CONTROL.y + 1;
      bool isOccupied = F0.getPoint(x, y);
      if (isOccupied == true) {
        // Do nothing
        Serial.println("Wall detected [above]");
      }
      else {
        CONTROL.spriteUp();
      }
      if (debug == true) {
        serialBuffer();
      }
      delay(movementDelay);
    }
    void moveDown() {
      int x = CONTROL.x;
      int y = CONTROL.y - 1;
      if (F0.getPoint(x, y) == true) {
        // Do nothing
        Serial.println("Wall detected [below]");
      }
      else {
        CONTROL.spriteDown();
      }
      if (debug == true) {
        serialBuffer();
      }
      delay(movementDelay);
    }
};

/**
    Avoid use of LedControl functions over custom functions, or buffer functions over display functions; this will not update the frame buffer and may cause some logic issues
*/

// definition of controllable sprite S0 and control class SX0
SpriteDot S0;
SpriteDotControl SX0;

void setup() {
  Serial.begin(115200); // serial channel for debugging (high baud rate to avoid significant slowdown)
  if (debug == true) {
    Serial.println("setup()");
  }
  // INSERT SETUP CODE HERE
  F0.initBuffer(); // initialises the frame buffer
  displayReset(); // resets/wakes the MAX7219 chips and the connected displays
  displayFrame(displaycheck); // displays the 32107654 test screen to ensure that all 8 LED matrices are working correctly
  delay(1000);
  displayReset();
  displayFrame(border); // displays a 1-pixel border around the edges of the display. The displayFrame function can be used with any 16x4 byte array, consisting of 512 bits or 64 bytes, one byte for each row, one bit for each pixel
  SX0.initialise(S0, 2, 2); // initialises SX0 as a controller for sprite S0 at coordinates (2,2)
}

void loop() {
  if (debug == true) {
    Serial.println("loop()");
  }
  // INSERT LOOP CODE HERE
  // demonstration of basic xy movement (sprite goes 16 right, 16 left, 8 up, 8 down, returning to its original position)
  for (int i = 0; i < 16; i++) {
    SX0.moveRight;
  }
  for (int i = 0; i < 16; i++) {
    SX0.moveLeft;
  }
  for (int i = 0; i < 8; i++) {
    SX0.moveUp;
  }
  for (int i = 0; i < 8; i++) {
    SX0.moveDown;
  }
  /**
  demonstration of further image display functions
  **/
  displayReset(); // completely clears the display, including the sprite, which will disappear until called upon again, which will cause it to reappear at the position given
  flowFill(true,10); // flow fills the display with a delay of 10ms between pixels
  fill(false); // clears the display without completely resetting the chips
  pixelToggle(0, 0, true);
  bool dotState = F0.getPoint(0,0);
  Serial.println(dotState); // sets the (0,0) pixel on, and checks if the corresponding change has taken place within the frame buffer; if it has been carries out successfully, this statement should output "true" or "1" to the serial monitor through the COM port
}
