/*******************************************************************
    Testing using a Nintendo Wii Nunchuck with the CYD

    More info on Nunchucks with Arduino: https://youtu.be/Cl9f1DUbMnc

    https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display

    Addtional Hardware Required:
      - Nunchuck adapter
        - My open source one from Oshpark: https://oshpark.com/shared_projects/RcIxSx2D
        - From Aliexpress*: https://s.click.aliexpress.com/e/_AEEtc3
      - A nintendo Nunchuck
        - Amazon.co.uk Search*: https://amzn.to/3nQrXcE
        - Amazon.com Search*: https://amzn.to/3nRJTUd
        - Aliexpress (Third Party)*: https://s.click.aliexpress.com/e/_AaQbXh

     = Affiliate Links

    Wiring:
      - Plug wire that came with CYD into the JST connector closest to the Micro SD card slot
      - Connect the wire to the adapter as follows:
          CYD  -> Adapter
          ------------------------------------------
          GND  -> - (AKA GND) - Black wire for me
          3.3V -> + (AKA 3V)  - Red wire for me
          IO22 -> d (AKA SDA) - Blue wire for me
          IO27 -> c (AKA SCL) - Yellow wire for me

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/

    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

// Make sure to copy the UserSetup.h file into the library as
// per the Github Instructions. The pins are defined in there.

// ----------------------------
// Standard Libraries
// ----------------------------

#include <Wire.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <NintendoExtensionCtrl.h>
// This library is for interfacing with the Nunchuck

// Can be installed from the library manager
// https://github.com/dmadison/NintendoExtensionCtrl

#include <TFT_eSPI.h>
// A library for interfacing with LCD displays
//
// Can be installed from the library manager (Search for "TFT_eSPI")
//https://github.com/Bodmer/TFT_eSPI

TFT_eSPI tft = TFT_eSPI();

// Controller connected to pins broken out on JTAG close to RGB LED
Nunchuk nchuk;

#define NUNCHUCK_SDA 22
#define NUNCHUCK_SCL 27

//--------------------------------
//Game Config Options:
//--------------------------------

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 320

#define WORLD_TO_PIXEL 14 //each dot on the game world will be reperesented by these many pixels.

#define DELAY_BETWEEN_FRAMES 50
#define DELAY_ON_LINE_CLEAR 100

#define NUM_PIECES_TIL_SPEED_CHANGE 20

#define PRINT_DELAY 1000

bool fillBlocks = true;

uint16_t myRED = TFT_RED;
uint16_t myGREEN = TFT_GREEN;
uint16_t myBLUE = TFT_BLUE;
uint16_t myWHITE = TFT_WHITE;
uint16_t myYELLOW = TFT_YELLOW;
uint16_t myCYAN = TFT_CYAN;
uint16_t myMAGENTA = TFT_MAGENTA;
uint16_t myBLACK = TFT_BLACK;

// [0] is empty space
// [1-7] are tetromino colours
// [8] is the colour a completed line changes to before dissapearing
// [9] is the walls of the board.
uint16_t gameColours[10] = {myBLACK, myRED, myGREEN, myBLUE, myWHITE, myYELLOW, myCYAN, myMAGENTA, myRED, myGREEN};

char tetromino[7][17];
int nFieldWidth = 12;
int nFieldHeight = 18;

int leftOffset = (DISPLAY_WIDTH - (nFieldWidth * WORLD_TO_PIXEL)) / 2;
int topOffset = DISPLAY_HEIGHT - 20 - (nFieldHeight * WORLD_TO_PIXEL);

int centerScreenX = DISPLAY_WIDTH / 2;

char *pField = nullptr;

int Rotate(int px, int py, int r)
{
  int pi = 0;
  switch (r % 4)
  {
    case 0: // 0 degrees      // 0  1  2  3
      pi = py * 4 + px;     // 4  5  6  7
      break;            // 8  9 10 11
    //12 13 14 15

    case 1: // 90 degrees     //12  8  4  0
      pi = 12 + py - (px * 4);  //13  9  5  1
      break;                      //14 10  6  2
    //15 11  7  3

    case 2: // 180 degrees      //15 14 13 12
      pi = 15 - (py * 4) - px;  //11 10  9  8
      break;                    // 7  6  5  4
    // 3  2  1  0

    case 3: // 270 degrees      // 3  7 11 15
      pi = 3 - py + (px * 4);   // 2  6 10 14
      break;                    // 1  5  9 13
  }   // 0  4  8 12

  return pi;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
  // All Field cells >0 are occupied
  for (int px = 0; px < 4; px++)
    for (int py = 0; py < 4; py++)
    {
      // Get index into piece
      int pi = Rotate(px, py, nRotation);

      // Get index into field
      int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

      // Check that test is in bounds. Note out of bounds does
      // not necessarily mean a fail, as the long vertical piece
      // can have cells that lie outside the boundary, so we'll
      // just ignore them
      if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
      {
        if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
        {
          // In Bounds so do collision check
          if (tetromino[nTetromino][pi] != L'.' && pField[fi] != 0)
            return false; // fail on first hit
        }
      }
    }

  return true;
}


void setup() {

  Serial.begin(115200);

  // Start the tft display and set it to black
  tft.init();
  tft.setRotation(0); //This is the display in portrait

  // Clear the screen before writing to it
  tft.fillScreen(TFT_BLACK);

  Wire.begin(NUNCHUCK_SDA, NUNCHUCK_SCL);
  while (!nchuk.connect()) {
    Serial.println("Nunchuk on bus #1 not detected!");
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.drawCentreString("No", 120, 10, 4);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.drawCentreString("Nunchuck", 120, 50, 4);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.drawCentreString("detected!", 120, 90, 4);
    delay(1000);
  }

  strcpy(tetromino[0], "..X...X...X...X."); // Tetronimos 4x4
  strcpy(tetromino[1], "..X..XX...X.....");
  strcpy(tetromino[2], ".....XX..XX.....");
  strcpy(tetromino[3], "..X..XX..X......");
  strcpy(tetromino[4], ".X...XX...X.....");
  strcpy(tetromino[5], ".X...X...XX.....");
  strcpy(tetromino[6], "..X...X..XX.....");

  // inits field
  restartGame();

  tft.fillScreen(TFT_BLACK);
}

bool bGameOver = false;

bool moveLeft;
bool moveRight;
bool dropDown;
bool rotatePiece;
bool cButtonPressed;

bool lastCButtonState;

bool isPaused;

int moveThreshold = 30;

int nCurrentPiece = 2;
int nCurrentRotation = 0;
int nCurrentX = (nFieldWidth / 2) - 2;
int nCurrentY = 0;

bool clearPreviousPiece = false;
int nPreviousX = -1;
int nPreviousY = -1;
int nPreviousRotation = -1;
int nPreviousPiece = -1;

bool drawLockedPiece = false;
int nLockedX = -1;
int nLockedY = -1;
int nLockedPiece = -1;
int nLockedRotation = -1;

int nSpeed = 20;
int nSpeedCount = 0;
bool bForceDown = false;
bool bRotateHold = true;

int nPieceCount = 0;
int nScore = 0;
int lastDrawnScore = -1;

int completedLinesIndex[4];
int numCompletedLines;

bool redrawWorld = false;
bool somethingMoved = true;

void getNewPiece()
{
  // Pick New Piece
  nCurrentX = (nFieldWidth / 2) - 2;
  nCurrentY = 0;

  nCurrentRotation = 0;
  nCurrentPiece = random(7);

  somethingMoved = true;
}

void gameTiming() {
  delay(DELAY_BETWEEN_FRAMES);
  nSpeedCount++;
  bForceDown = (nSpeedCount == nSpeed);
}

void clearLines() {
  if (numCompletedLines > 0)
  {
    delay(DELAY_ON_LINE_CLEAR);
    for (int i = 0; i < numCompletedLines; i ++ )
    {
      for (int px = 1; px < nFieldWidth - 1; px++)
      {
        for (int py = completedLinesIndex[i]; py > 0; py--)
          pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
        pField[px] = 0;
      }
    }
    numCompletedLines = 0;
    redrawWorld = true;
  }
}

void gameLogic() {

  somethingMoved = false;

  //Handle updating lines cleared last tick
  clearLines();

  nPreviousX = nCurrentX;
  nPreviousY = nCurrentY;
  nPreviousRotation = nCurrentRotation;
  nPreviousPiece = nCurrentPiece;

  if (moveRight && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) {
    somethingMoved = true;
    nCurrentX += 1;
  }
  if (moveLeft && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) {
    somethingMoved = true;
    nCurrentX -= 1;
  }
  if (dropDown && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
    somethingMoved = true;
    nCurrentY += 1;
  }

  if (rotatePiece)
  {
    nCurrentRotation += (bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
    somethingMoved = true;
    bRotateHold = false;
  }
  else
    bRotateHold = true;

  // Force the piece down the playfield if it's time
  if (bForceDown)
  {
    somethingMoved = true;
    // Update difficulty every 50 pieces
    nSpeedCount = 0;
    nPieceCount++;
    if (nPieceCount % NUM_PIECES_TIL_SPEED_CHANGE == 0)
      if (nSpeed >= 10) nSpeed--;

    // Test if piece can be moved down
    if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
      nCurrentY++; // It can, so do it!
    else
    {
      // It can't! Lock the piece in place
      for (int px = 0; px < 4; px++)
        for (int py = 0; py < 4; py++)
          if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != '.')
            pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

      nLockedX = nCurrentX;
      nLockedY = nCurrentY;
      nLockedRotation = nCurrentRotation;
      nLockedPiece = nCurrentPiece;
      drawLockedPiece = true;

      // Check for lines
      for (int py = 0; py < 4; py++)
        if (nCurrentY + py < nFieldHeight - 1)
        {
          bool bLine = true;
          for (int px = 1; px < nFieldWidth - 1; px++)
            bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

          if (bLine)
          {
            // Remove Line, set to =
            for (int px = 1; px < nFieldWidth - 1; px++)
              pField[(nCurrentY + py) * nFieldWidth + px] = 8;
            //vLines.push_back(nCurrentY + py);
            completedLinesIndex[numCompletedLines] = nCurrentY + py;
            numCompletedLines++;
            redrawWorld = true;
          }
        }

      nScore += 25;
      if (numCompletedLines > 0) {
        nScore += (1 << numCompletedLines) * 100;
      }


      // Pick New Piece
      getNewPiece();

      // If piece does not fit straight away, game over!
      bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
    }
  }
  clearPreviousPiece = somethingMoved;
}

void gameInput() {

  boolean success = nchuk.update();  // Get new data from the controller

  if (!success) {  // Ruh roh
    Serial.println("Controller disconnected!");
    delay(1000);
  }
  else {

    rotatePiece = nchuk.buttonZ();

    lastCButtonState = cButtonPressed;
    cButtonPressed = nchuk.buttonC();

    if (cButtonPressed && !lastCButtonState)
    {
      isPaused = !isPaused;
    }

    // Read a joystick axis (0-255, X and Y)
    int joyY = nchuk.joyY();
    int joyX = nchuk.joyX();

    moveRight = (joyX > 127 + moveThreshold);
    moveLeft = (joyX < 127 - moveThreshold);
    dropDown = (joyY < 127 - moveThreshold);

  }

}

uint16_t getFieldColour(int index, bool isDeathScreen) {
  if (isDeathScreen && pField[index] != 0) {
    return myRED;
  } else {
    return gameColours[pField[index]];
  }
}

void drawBlock(int x, int y, uint16_t colour) {
  int realX;
  int realY;
  realX = (x * WORLD_TO_PIXEL) + leftOffset;
  realY = (y * WORLD_TO_PIXEL) + topOffset;
  if (WORLD_TO_PIXEL > 1) {
    if (fillBlocks) {
      tft.fillRect(realX, realY, WORLD_TO_PIXEL, WORLD_TO_PIXEL, colour);
      tft.drawRect(realX, realY, WORLD_TO_PIXEL, WORLD_TO_PIXEL, TFT_BLACK);
    } else {
      tft.drawRect(realX, realY, WORLD_TO_PIXEL, WORLD_TO_PIXEL, colour);
    }

  } else {
    tft.drawPixel(realX, realY, colour);
  }
}

void drawPiece(int x, int y, int rotation, int piece, uint16_t colour) {
  for (int px = 0; px < 4; px++) {
    for (int py = 0; py < 4; py++) {
      if (tetromino[piece][Rotate(px, py, rotation)] != '.') {
        drawBlock((x + px), (y + py), colour);
      }
    }
  }
}

void displayLogic(bool isDeathScreen = false) {

  int realX;
  int realY;

  bool forceScoreUpdate = false;

  if (redrawWorld) {
    tft.fillScreen(TFT_BLACK);
    forceScoreUpdate = true;
    // Draw Field
    for (int x = 0; x < nFieldWidth; x++) {
      for (int y = 0; y < nFieldHeight; y++) {
        uint16_t fieldColour = getFieldColour((y * nFieldWidth + x), isDeathScreen);
        if (fieldColour != myBLACK) {
          drawBlock(x, y, fieldColour);
        }
      }
    }
    redrawWorld = false;
  }

  // Clear Previous Movement of Piece
  if (clearPreviousPiece) {
    drawPiece(nPreviousX, nPreviousY, nPreviousRotation, nPreviousPiece, TFT_BLACK);
    clearPreviousPiece = false;
  }

  // This piece previously just locked in, no point redrawing the entire field just to get this.
  if (drawLockedPiece) {
    drawPiece(nLockedX, nLockedY, nLockedRotation, nLockedPiece, gameColours[nLockedPiece + 1]);
    drawLockedPiece = false;
  }

  drawPiece(nCurrentX, nCurrentY, nCurrentRotation, nCurrentPiece, gameColours[nCurrentPiece + 1]);

  if (lastDrawnScore != nScore || forceScoreUpdate) {
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    if (isPaused) {
      tft.drawCentreString("Paws", centerScreenX, 10, 4);
    } else {
      // Display the Score

      tft.drawCentreString(String(nScore), centerScreenX, 10, 4);
      lastDrawnScore = nScore;
    }
  }


}

void restartGame() {

  redrawWorld = true;
  bGameOver = false;
  pField = new char[nFieldWidth * nFieldHeight]; // Create play field buffer
  for (int x = 0; x < nFieldWidth; x++) // Board Boundary
    for (int y = 0; y < nFieldHeight; y++)
      pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

  // Pick New Piece
  getNewPiece();

  nScore = 0;

}

void loop() {
  // put your main code here, to run repeatedly:
  if (!bGameOver)
  {
    gameInput();
    if (!isPaused)
    {
      gameTiming();
      gameLogic();
    } else {
      delay(DELAY_BETWEEN_FRAMES); //stopping pulsing of LEDS
    }
    if (somethingMoved) {
      displayLogic();
    }
  } else {
    delay(DELAY_BETWEEN_FRAMES);
    gameInput();
    displayLogic(true);
    if (cButtonPressed)
    {
      restartGame();
    }
  }
}
