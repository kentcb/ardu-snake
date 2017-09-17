#include "Arduboy.h"
#include "timer.h"

enum GAME_STATE {
  TITLE,
  PLAYING,
  OVER
};

Arduboy arduboy;
ArduboyTunes arduboyTunes;
GAME_STATE gameState;

const int screenWidth = 128;
const int screenHeight = 64;
const int tileSize = 4;
const int tileWidth = screenWidth / tileSize;
const int tileHeight = screenHeight / tileSize;
const int maxSegments = 32;

int frame;
int dx;
int dy;
int segmentsX[maxSegments];
int segmentsY[maxSegments];
int segmentCount;
int bonusX;
int bonusY;
int bonusCount;

void setup() {
  arduboy.beginNoLogo();
  arduboy.setFrameRate(60);
  arduboy.initRandomSeed();

  startGame();
}

void startGame() {
  frame = 0;
  dx = 1;
  dy = 0;
  bonusCount = 0;

  for (int i = 0; i < maxSegments; ++i) {
    segmentsX[i] = 0;
    segmentsY[i] = 0;
  }

  segmentCount = 2;
  segmentsX[0] = tileWidth / 2;
  segmentsY[0] = tileHeight / 2;
  segmentsX[1] = segmentsX[0] - 5;
  segmentsY[1] = segmentsY[0];

  bonusX = -1;
  bonusY = -1;

  gameState = PLAYING;
}

void endGame() {
  frame = 0;
  gameState = OVER;
}

void assert(bool condition, char* message) {
  if (!condition) {
    while (true) {
      arduboy.clear();
      arduboy.drawChar(0, 0, 'A', 1, 0, 2);
      drawString(message, 0, 25, 7);      
      arduboy.display();
    }
  }
}

void drawString(char* string, int x, int y, float kerning, int size) {
  char* output = string;
  float xPos = x;
  
  while (*output != NULL) {
    arduboy.drawChar((int)xPos, y, *output, 1, 0, size);
    xPos += kerning;
    ++output;
  }
}

void drawString(char* string, int x, int y, float kerning) {
  drawString(string, x, y, kerning, 1);
}

void addInflectionPointAfterHead() {
  assert(segmentCount <= maxSegments, "addInflectionPointAfterHead");
  
  for (int i = segmentCount; i >= 1; --i) {
    segmentsX[i] = segmentsX[i - 1];
    segmentsY[i] = segmentsY[i - 1];
  }

  segmentsX[1] = segmentsX[0];
  segmentsY[1] = segmentsY[0];

  ++segmentCount;
}

bool isWall(int tileX, int tileY) {
  return tileX == 0 || tileY == 0 || tileX == (tileWidth - 1) || tileY == (tileHeight - 1);
}

bool isBonus(int tileX, int tileY) {
  return bonusX != -1 && bonusX == tileX && bonusY == tileY;
}

bool isSnake(int tileX, int tileY) {
  return arduboy.getPixel(tileX * tileSize, tileY * tileSize) == 1;
}

void chooseRandomBonus() {
  int tileX = rand() % (tileWidth - 2);
  int tileY = rand() % (tileHeight - 2);

  if (!isSnake(tileX, tileY)) {
    bonusX = tileX + 1;
    bonusY = tileY + 1;
  }
}

void removeInflectionPointBeforeTail() {
  assert(segmentCount > 2, "removeInflectionPointBeforeTail");
  
  --segmentCount;
  segmentsX[segmentCount - 1] = segmentsX[segmentCount];
  segmentsY[segmentCount - 1] = segmentsY[segmentCount];
}

void drawWithSin(char ch, int angle, int amplitude, int x, int y) {
  float yOffset = sin(angle);
  int yAdjust = (int)(yOffset * amplitude);

  arduboy.drawChar(x, y + yAdjust, ch, 1, 0, 2);
}

void titleLoop() {
  const int speed = 10;
  const int yWiggle = 14;
  const int halfYWiggle = yWiggle / 2;

  int startAngle = frame / speed;
  const int angleAdjust = 15;
  int angle = startAngle;
  int x = (screenWidth / 2) - (5 * 14 / 2);

  drawWithSin('S', angle, 8, x, screenHeight / 2);
  angle += angleAdjust;
  x += 14;
  drawWithSin('N', angle, 8, x, screenHeight / 2);
  angle += angleAdjust;
  x += 14;
  drawWithSin('A', angle, 8, x, screenHeight / 2);
  angle += angleAdjust;
  x += 14;
  drawWithSin('K', angle, 8, x, screenHeight / 2);
  angle += angleAdjust;
  x += 14;
  drawWithSin('E', angle, 8, x, screenHeight / 2);
  
  ++frame;
}

void gameLoop() {
  if (bonusX == -1) {
    chooseRandomBonus();
  }

  arduboy.fillRect(0, 0, tileSize, screenHeight, 1);
  arduboy.fillRect(0, screenHeight - tileSize, screenWidth, tileSize, 1);
  arduboy.fillRect(screenWidth - tileSize, 0, tileSize, screenHeight, 1);
  arduboy.fillRect(0, 0, screenWidth, tileSize, 1);

  for (int i = 0; i < segmentCount - 1; ++i) {
    int x1 = segmentsX[i];
    int y1 = segmentsY[i];
    int x2 = segmentsX[i + 1];
    int y2 = segmentsY[i + 1];
    int deltaX = x2 - x1;
    int deltaY = y2 - y1;

    if (deltaX < 0) {
      int tempX = x1;
      x1 = x2;
      x2 = tempX;

      deltaX *= -1;
    }
    
    if (deltaY < 0) {
      int tempY = y1;
      y1 = y2;
      y2 = tempY;

      deltaY *= -1;
    }

    deltaX += 1;
    deltaY += 1;
    
    arduboy.fillRect(x1 * tileSize, y1 * tileSize, deltaX * tileSize, deltaY * tileSize, 1);
  }

  if (bonusX != -1) {
    arduboy.fillRect(bonusX * tileSize, bonusY * tileSize, tileSize, tileSize, 1);
  }

  if ((frame % 15) == 0) {
    if (arduboy.pressed(LEFT_BUTTON) && dx != -1) {
      dx = -1;
      dy = 0;
      addInflectionPointAfterHead();
    } else if (arduboy.pressed(RIGHT_BUTTON) && dx != 1) {
      dx = 1;
      dy = 0;
      addInflectionPointAfterHead();
    } else if (arduboy.pressed(UP_BUTTON) && dy != -1) {
      dy = -1;
      dx = 0;
      addInflectionPointAfterHead();
    } else if (arduboy.pressed(DOWN_BUTTON) && dy != 1) {
      dy = 1;
      dx = 0;
      addInflectionPointAfterHead();
    }

    segmentsX[0] += dx;
    segmentsY[0] += dy;
    
    if (isBonus(segmentsX[0], segmentsY[0])) {
      bonusX = -1;
      bonusY = -1;
      ++bonusCount;
    } else if (isWall(segmentsX[0], segmentsY[0]) || isSnake(segmentsX[0], segmentsY[0])) {
      endGame();
    } else {
      int lastInflexionX = segmentsX[segmentCount - 1 - 1];
      int lastInflexionY = segmentsY[segmentCount - 1 - 1];
  
      int tailX = segmentsX[segmentCount - 1];
      int tailY = segmentsY[segmentCount - 1];
  
      int deltaTailX = 0;
      int deltaTailY = 0;
  
      if (tailX < lastInflexionX) {
        deltaTailX = 1;
      } else if (tailX > lastInflexionX) {
        deltaTailX = -1;
      }
  
      if (tailY < lastInflexionY) {
        deltaTailY = 1;
      } else if (tailY > lastInflexionY) {
        deltaTailY = -1;
      }
  
      // adjust tail
      segmentsX[segmentCount - 1] += deltaTailX;
      segmentsY[segmentCount - 1] += deltaTailY;
  
      if (segmentsX[segmentCount - 1] == segmentsX[segmentCount - 2] && segmentsY[segmentCount - 1] == segmentsY[segmentCount - 2]) {
        removeInflectionPointBeforeTail();
      }
    }
  }
  
  ++frame;
}

int getHorizontalCenterFor(char* string, int kerning) {
  return (screenWidth / 2) - (strlen(string) * kerning / 2);
}

void gameOverLoop() {
  int y = frame / 10 - 10;

  if (y > 25) {
    y = 25;
  }

  char* gameOver = "GAME OVER";
  
  drawString(gameOver, getHorizontalCenterFor(gameOver, 10), y, 10);

  char buffer[11];
  buffer[0] = 'S';
  buffer[1] = 'c';
  buffer[2] = 'o';
  buffer[3] = 'r';
  buffer[4] = 'e';
  buffer[5] = ':';
  itoa(bonusCount, buffer + 6, 10);
  
  drawString(buffer, getHorizontalCenterFor(buffer, 7), 45, 7);
  
  if (arduboy.pressed(A_BUTTON)) {
    startGame();
  }

  ++frame;
}

void loop() {
  arduboy.clear();

  switch (gameState) {
    case TITLE:
      titleLoop();
      break;
    case PLAYING:
      gameLoop();
      break;
    case OVER:
      gameOverLoop();
      break;
  }

  arduboy.display();
}
