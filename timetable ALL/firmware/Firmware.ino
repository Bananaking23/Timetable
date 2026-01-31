#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>
#include <ArduinoBLE.h>

/* ================= PIN DEFINITIONS =================
   CHANGE THESE TO MATCH YOUR PCB
*/

// ---- E-PAPER ----
#define EPD_CS     1
#define EPD_DC     10
#define EPD_RST    7
#define EPD_BUSY   4

#define SPI_SCK    8
#define SPI_MOSI   1

// ---- BUTTONS (ACTIVE LOW) ----
#define BTN_UP     2
#define BTN_OK     3
#define BTN_DOWN   6

// ---- STATUS LED ----
#define LED_PIN    4


/* ================= DISPLAY =================
   Change driver if your panel is different
*/
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
  GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);


/* ================= BLE UART ================= */
BLEService uartService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");

BLECharacteristic rxChar(
  "6E400002-B5A3-F393-E0A9-E50E24DCCA9E",
  BLEWrite,
  64
);

BLECharacteristic txChar(
  "6E400003-B5A3-F393-E0A9-E50E24DCCA9E",
  BLENotify,
  64
);


/* ================= DATA ================= */
int page = 0;
const int MAX_PAGES = 5;

String timetable[MAX_PAGES][4] = {
  {"Mon", "Math", "Physics", "CS"},
  {"Tue", "English", "Chem", "PE"},
  {"Wed", "Bio", "Math", "Free"},
  {"Thu", "CS", "Physics", "Art"},
  {"Fri", "Math", "English", "Free"}
};


/* ================= SETUP ================= */
void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  SPI.setSCK(SPI_SCK);
  SPI.setMOSI(SPI_MOSI);
  SPI.begin();

  display.init(115200);
  display.setRotation(1);

  BLE.begin();
  BLE.setLocalName("Timetable");
  BLE.setAdvertisedService(uartService);

  uartService.addCharacteristic(rxChar);
  uartService.addCharacteristic(txChar);
  BLE.addService(uartService);

  rxChar.setEventHandler(BLEWritten, onBLEReceive);
  BLE.advertise();

  drawScreen();
}


/* ================= LOOP ================= */
void loop() {
  BLE.poll();

  if (buttonPressed(BTN_UP)) {
    page--;
    if (page < 0) page = 0;
    drawScreen();
  }

  if (buttonPressed(BTN_DOWN)) {
    page++;
    if (page >= MAX_PAGES) page = MAX_PAGES - 1;
    drawScreen();
  }

  if (buttonPressed(BTN_OK)) {
    blinkLED(2);
    drawScreen();
  }

  delay(10);
}


/* ================= BLE RECEIVE ================= */
void onBLEReceive(BLEDevice central, BLECharacteristic characteristic) {
  String data = characteristic.value();

  int last = 0;
  int part = 0;

  for (int i = 0; i <= data.length(); i++) {
    if (data[i] == ',' || i == data.length()) {
      if (part < 4) {
        timetable[page][part] = data.substring(last, i);
      }
      part++;
      last = i + 1;
    }
  }

  txChar.writeValue("OK");
  drawScreen();
}


/* ================= DISPLAY ================= */
void drawScreen() {
  display.setFullWindow();
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    display.setTextSize(2);
    display.setCursor(10, 20);
    display.print("Timetable");

    display.setTextSize(1);
    display.setCursor(10, 40);
    display.print("Day: ");
    display.print(timetable[page][0]);

    display.setTextSize(2);
    int y = 70;
    for (int i = 1; i < 4; i++) {
      display.setCursor(10, y);
      display.print("- ");
      display.print(timetable[page][i]);
      y += 30;
    }

  } while (display.nextPage());
}


/* ================= HELPERS ================= */
bool buttonPressed(int pin) {
  if (digitalRead(pin) == LOW) {
    delay(25);
    if (digitalRead(pin) == LOW) {
      while (digitalRead(pin) == LOW);
      return true;
    }
  }
  return false;
}

void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(80);
    digitalWrite(LED_PIN, HIGH);
    delay(80);
  }
}
