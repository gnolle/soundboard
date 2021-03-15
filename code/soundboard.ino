#include <WS2812.h>
#include "src/matrix/Matrix.h"
#include "src/button/Button.h"
#include "src/rotary/Rotary.h"
#include "src/musicplayer/Adafruit_VS1053.h"
#include <SPI.h>
#include <SdFat.h>
#include <inttypes.h>

#define BREAKOUT_RESET  9
#define BREAKOUT_CS     10
#define BREAKOUT_DCS    8
#define CARDCS          4
#define DREQ            3

SdFat sd;

byte rows[] = {8, 9, 10, 11};
byte rowCount = sizeof(rows) / sizeof(rows[0]);

byte cols[] = {0, 1, 2, 3, 4, 5, 6, 7};
byte colCount = sizeof(cols) / sizeof(cols[0]);

Adafruit_VS1053_FilePlayer musicPlayer =
  Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

WS2812 LED(1);

byte volumeValue = 20;

cRGB value;

Rotary volume = Rotary(A0, A1);

byte currentPageIndex = 0;
byte numberOfPages = 0;

struct PAGE {
  char dirPath[30];
  char color[7];
};

PAGE currentPage;

void onButtonPress(byte number) {
  if (musicPlayer.playingMusic) {
    musicPlayer.stopPlaying();
    while (musicPlayer.playingMusic) {
      delay(1);
    }
  }

  File root;
  root.open(currentPage.dirPath);

  File entry;
  while (entry.openNext(&root, O_RDONLY)) {

    char trackName[40];
    entry.getName(trackName, sizeof(trackName));
    char trackPrefix[3];
    sprintf(trackPrefix, "%d_", number + 1);

    if (strncmp(trackName, trackPrefix, strlen(trackPrefix)) == 0) {
      char pathToTrack[strlen(currentPage.dirPath) + strlen(trackName) + 1];
      sprintf(pathToTrack, "%s/%s", currentPage.dirPath, trackName);
      musicPlayer.startPlayingFile(pathToTrack);
      entry.close();
      break;
    }

    entry.close();
  }

  root.close();
}

void knobPressed() {
  currentPageIndex = (currentPageIndex + 1) % numberOfPages;
  loadCurrentPage();
}

Matrix matrix(rows, rowCount, cols, colCount, onButtonPress);

Button button(2, knobPressed);

void pciSetup(byte pin) {
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));
  PCIFR  |= bit (digitalPinToPCICRbit(pin));
  PCICR  |= bit (digitalPinToPCICRbit(pin));
}

void setPageColor() {
  sscanf(currentPage.color, "%2" SCNx8 "%2" SCNx8 "%2" SCNx8, &value.r, &value.g, &value.b);
  LED.set_crgb_at(0, value);
  LED.sync();
}

void setup() {
  musicPlayer.begin();
  sd.begin(CARDCS);

  pciSetup(A0);
  pciSetup(A1);

  volume.begin();

  musicPlayer.setVolume(volumeValue, volumeValue);
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);

  matrix.init();
  LED.setOutput(5);

  countPages();
  loadCurrentPage();
}

void countPages() {
  File root;
  root.open("/");
  File entry;
  while (entry.openNext(&root, O_RDONLY)) {
    if (entry.isDir() && !entry.isHidden()) {
      numberOfPages++;
    }
    entry.close();
  }
  root.close();
}

void loadCurrentPage() {
  char pagePrefix[3];
  sprintf(pagePrefix, "%d_", currentPageIndex + 1);
  File root;
  root.open("/");
  File entry;
  while (entry.openNext(&root, O_RDONLY)) {
    if (entry.isDir() && !entry.isHidden()) {
      char pageName[30];
      entry.getName(pageName, sizeof(pageName));
      if (strncmp(pageName, pagePrefix, strlen(pagePrefix)) == 0) {
        char dirPath[sizeof(pageName) + 1];
        sprintf(dirPath, "/%s", pageName);
        strcpy(currentPage.dirPath, dirPath);

        char colorFilePath[sizeof(dirPath) + sizeof("/color.txt")];
        sprintf(colorFilePath, "%s/color.txt", dirPath);
        File colorFile;
        colorFile.open(colorFilePath);

        char color[7];
        colorFile.readBytes(color, 6);
        color[6] = '\0';
        strcpy(currentPage.color, color);
        colorFile.close();
        setPageColor();
      }
    }
    entry.close();
  }
  root.close();
}

void loop() {
  matrix.scan();
  button.scan();
}

ISR (PCINT1_vect) {
  unsigned char result = volume.process();
  if (result == DIR_CW) {
    if (volumeValue < 254) volumeValue += 2;
    musicPlayer.setVolume(volumeValue, volumeValue);
  }
  if (result == DIR_CCW) {
    if (volumeValue > 1) volumeValue -= 2;
    musicPlayer.setVolume(volumeValue, volumeValue);
  }

}
