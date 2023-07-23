// Adafruit_ImageReader test for Adafruit ST7735 TFT Breakout for Arduino.
// Demonstrates loading images from SD card or flash memory to the screen,
// to RAM, and how to query image file dimensions.
// Requires three BMP files in root directory of SD card:
// parrot.bmp, miniwoof.bmp and wales.bmp.
// As written, this uses the microcontroller's SPI interface for the screen
// (not 'bitbang') and must be wired to specific pins (e.g. for Arduino Uno,
// MOSI = pin 11, MISO = 12, SCK = 13). Other pins are configurable below.

#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ST7735.h>      // Hardware-specific library
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions

// Comment out the next line to load from SPI/QSPI flash instead of SD card:
#define USE_SD_CARD

// TFT display and SD card share the hardware SPI interface, using
// 'select' pins for each to identify the active device on the bus.

#define SD_CS    A1 // SD card select pin
#define TFT_CS   A5 // TFT chip select pin (TFT_CS on the display)
#define TFT_RST  A4 // TFT reset pin (RST on the display)
#define TFT_DC   A6 // TFT data/command pin (D/C on the display)
#define TFT_MOSI 8  // Data out
#define TFT_SCLK 9  // Clock out
#define TFT_MISO 10 // Data out

#if defined(USE_SD_CARD)
  SdFat                SD;         // SD card filesystem
  Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
#else
  // SPI or QSPI flash filesystem (i.e. CIRCUITPY drive)
  #if defined(__SAMD51__) || defined(NRF52840_XXAA)
    Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS,
      PIN_QSPI_IO0, PIN_QSPI_IO1, PIN_QSPI_IO2, PIN_QSPI_IO3);
  #else
    #if (SPI_INTERFACES_COUNT == 1)
      Adafruit_FlashTransport_SPI flashTransport(SS, &SPI);
    #else
      Adafruit_FlashTransport_SPI flashTransport(SS1, &SPI1);
    #endif
  #endif
  Adafruit_SPIFlash    flash(&flashTransport);
  FatVolume        filesys;
  Adafruit_ImageReader reader(filesys); // Image-reader, pass in flash filesys
#endif

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 160, // BMP image dimensions
                     height = 128;


void setup(void) {
  pinMode(SD_CS, OUTPUT);
  ImageReturnCode stat; // Status from image-reading functions

  Serial.begin(9600);
#if !defined(ESP32)
  while(!Serial);       // Wait for Serial Monitor before continuing
#endif
  pinMode(A1, OUTPUT);
  digitalWrite(A1, HIGH);
  tft.initR(INITR_BLACKTAB); // Initialize screen

  // The Adafruit_ImageReader constructor call (above, before setup())
  // accepts an uninitialized SdFat or FatVolume object. This MUST
  // BE INITIALIZED before using any of the image reader functions!
  Serial.print(F("Initializing filesystem..."));
#if defined(USE_SD_CARD)
  // SD card is pretty straightforward, a single call...
  if(!SD.begin(SD_CS, SD_SCK_MHZ(10))) { // Breakouts require 10 MHz limit due to longer wires
    Serial.println(F("SD begin() failed"));
    for(;;); // Fatal error, do not continue
  }
#else
  // SPI or QSPI flash requires two steps, one to access the bare flash
  // memory itself, then the second to access the filesystem within...
  if(!flash.begin()) {
    Serial.println(F("flash begin() failed"));
    for(;;);
  }
  if(!filesys.begin(&flash)) {
    Serial.println(F("filesys begin() failed"));
    for(;;);
  }
#endif
  Serial.println(F("OK!"));

  // Fill screen blue. Not a required step, this just shows that we're
  // successfully communicating with the screen.
  tft.fillScreen(ST7735_BLUE);

  // Load full-screen BMP file 'parrot.bmp' at position (0,0) (top left).
  // Notice the 'reader' object performs this, with 'tft' as an argument.
  Serial.print(F("Loading parrot.bmp to screen..."));
  stat = reader.drawBMP("parrot.bmp", tft, 160, 128);
  reader.printStatus(stat);   // How'd we do?

  // Query the dimensions of image 'miniwoof.bmp' WITHOUT loading to screen:

  if(stat == IMAGE_SUCCESS) { // If it worked, print image size...
    Serial.print(F("Image dimensions: "));
    Serial.print(width);
    Serial.write('x');
    Serial.println(height);
  }

}

void loop() {

}