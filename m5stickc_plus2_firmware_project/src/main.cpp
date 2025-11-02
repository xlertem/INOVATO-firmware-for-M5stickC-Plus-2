// main.cpp
// Simplified working framework for M5StickC Plus2 (Arduino/PlatformIO)
#include <M5Unified.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <IRremote.h>
#include <SPIFFS.h>
#include <time.h>

const char* ap_ssid = "MyDevice_AP";
const char* ap_pass = "12345678";
const int IR_SEND_PIN = 19;
const int TFT_W = 135;
const int TFT_H = 240;

enum Screen { SCREEN_MAIN, SCREEN_WIFI, SCREEN_BLUETOOTH, SCREEN_SETTINGS, SCREEN_IR };
Screen currentScreen = SCREEN_MAIN;

unsigned long lastClockUpdate = 0;
const unsigned long CLOCK_INTERVAL = 1000;

IRsend irsend(IR_SEND_PIN);

float readBatteryPercent(){
  // Use M5.Axp.BatteryPercent() when available in your M5 library; fallback stub:
  #ifdef ARDUINO_M5Stick_C_PLUS
    // some M5 libraries expose power API; adjust if needed
  #endif
  return 100.0; // placeholder until replaced by actual power API or ADC reading
}

String formattedTime(){
  time_t now;
  time(&now);
  struct tm* tm_info = localtime(&now);
  char buf[32];
  strftime(buf, sizeof(buf), "%H:%M:%S", tm_info);
  return String(buf);
}

void drawStatusBar(){
  auto &lcd = M5.Display;
  lcd.fillRect(0, 0, TFT_W, 20, 0); // black
  float batt = readBatteryPercent();
  String bp = String((int)batt) + "%";
  lcd.setTextSize(1);
  lcd.setCursor(4, 4);
  lcd.setTextColor(0xFFFF);
  lcd.print(bp);
  lcd.setCursor(80,4);
  lcd.print("WiFi");
  lcd.setCursor(110,4);
  lcd.print(formattedTime());
}

void drawMainScreen(){
  auto &lcd = M5.Display;
  lcd.fillScreen(0);
  drawStatusBar();
  lcd.setTextSize(2);
  lcd.setCursor(8,36);
  lcd.setTextColor(0xFFFF);
  lcd.print("Главный экран");
  int barX = 8, barY = 80, barW = 119, barH = 20;
  float batt = readBatteryPercent();
  lcd.drawRect(barX, barY, barW, barH, 0xFFFF);
  int fillW = (int)((batt/100.0) * (barW-2));
  lcd.fillRect(barX+1, barY+1, fillW, barH-2, 0x07E0); // green-ish
  lcd.setTextSize(1);
  lcd.setCursor(8,120);
  lcd.printf("BTN TOP: Menu  BTN BTM: Back");
}

void drawWifiScreen(){
  auto &lcd = M5.Display;
  lcd.fillScreen(0x39E7); // dark grey approx
  drawStatusBar();
  lcd.setTextSize(2);
  lcd.setCursor(8,36);
  lcd.setTextColor(0xFFFF);
  lcd.print("WiFi меню");
  lcd.setTextSize(1);
  lcd.setCursor(8,80);
  lcd.print("1) Connect to WiFi");
  lcd.setCursor(8,100);
  lcd.print("2) Start AP");
}

void drawBluetoothScreen(){
  auto &lcd = M5.Display;
  lcd.fillScreen(0);
  drawStatusBar();
  lcd.setTextSize(2);
  lcd.setCursor(8,36);
  lcd.setTextColor(0xFFFF);
  lcd.print("Bluetooth");
  lcd.setTextSize(1);
  lcd.setCursor(8,80);
  lcd.print("BLE: Advertising...");
}

void drawSettingsScreen(){
  auto &lcd = M5.Display;
  lcd.fillScreen(0x39E7);
  drawStatusBar();
  lcd.setTextSize(2);
  lcd.setCursor(8,36);
  lcd.setTextColor(0xFFFF);
  lcd.print("Настройки");
  lcd.setTextSize(1);
  lcd.setCursor(8,80);
  lcd.print("Brightness / Sound / Config");
}

void drawIRScreen(){
  auto &lcd = M5.Display;
  lcd.fillScreen(0);
  drawStatusBar();
  lcd.setTextSize(2);
  lcd.setCursor(8,36);
  lcd.setTextColor(0xFFFF);
  lcd.print("IR Remote");
  lcd.setTextSize(1);
  lcd.setCursor(8,80);
  lcd.print("Press top to send IR code");
}

void animateScreenTransition(Screen toScreen){
  // Simple transition: just set current and redraw. Complex fades may be slow on device.
  currentScreen = toScreen;
  switch(currentScreen){
    case SCREEN_MAIN: drawMainScreen(); break;
    case SCREEN_WIFI: drawWifiScreen(); break;
    case SCREEN_BLUETOOTH: drawBluetoothScreen(); break;
    case SCREEN_SETTINGS: drawSettingsScreen(); break;
    case SCREEN_IR: drawIRScreen(); break;
  }
}

void startWiFiAP(){
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);
}

void startBLE(){
  BLEDevice::init("M5StickC_Plus2");
}

void sendIRExample(){
  // send a NEC code example
  irsend.sendNEC(0x20DF10EF, 32);
}

void handleButtons(){
  if (M5.BtnA.wasPressed()){
    if (currentScreen == SCREEN_MAIN) animateScreenTransition(SCREEN_WIFI);
    else animateScreenTransition(SCREEN_MAIN);
  }
  if (M5.BtnB.wasPressed()){
    if (currentScreen == SCREEN_WIFI) animateScreenTransition(SCREEN_BLUETOOTH);
    else animateScreenTransition(SCREEN_MAIN);
  }
  if (M5.BtnC.wasPressed()){
    animateScreenTransition(SCREEN_SETTINGS);
  }
}

void setup(){
  Serial.begin(115200);
  auto cfg = M5.config();
  cfg.clear(); // use defaults
  M5.begin(cfg);
  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS Mount Failed");
  }
  irsend.begin();
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  drawMainScreen();
}

void loop(){
  M5.update();
  if (millis() - lastClockUpdate > CLOCK_INTERVAL){
    lastClockUpdate = millis();
    drawStatusBar();
  }
  handleButtons();
  if (currentScreen == SCREEN_IR && M5.BtnA.wasPressed()){
    sendIRExample();
  }
  delay(10);
}