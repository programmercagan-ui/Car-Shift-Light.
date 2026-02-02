#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <BluetoothSerial.h>
#include <ELMduino.h>
#include <FastLED.h>

/* ================= AYARLAR & PINLER ================= */
#define LED_PIN     5
#define NUM_LEDS    8
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define BRIGHTNESS  220
#define EEPROM_SIZE 32

// ELM327 MAC ADRESİN
uint8_t address[6] = {0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33};

/* ================= LED ================= */
CRGB leds[NUM_LEDS];
const int centerLeft  = 3;
const int centerRight = 4;

/* ================= MOD ================= */
#define MODE_GUNLUK 0
#define MODE_CADDE  1

int currentMode;
int rpmStart, rpmGreenEnd, rpmYellowEnd, rpmRedEnd;
int ledBrightness = BRIGHTNESS;
int strobeDelay   = 50;

/* ================= WIFI ================= */
WebServer server(80);
const char* ssid     = "SHIFT_LIGHT";
const char* password = "12345678";

/* ================= OBD ================= */
BluetoothSerial SerialBT;
ELM327 elm327;
bool obdConnected = false;

/* ================= EFEKT ================= */
unsigned long lastStrobeTime = 0;
bool strobeState = false;

unsigned long lastChaseTime = 0;
int chaseIndex = 0;
uint8_t hue = 0;

/* ================= HTML ================= */
String page() {
  String s = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  s += "<style>body{background:#111;color:#fff;text-align:center;font-family:Arial}";
  s += "button,input{font-size:18px;margin:8px;padding:10px;width:220px}</style></head><body>";

  s += "<h2>SHIFT LIGHT</h2>";
  s += "<p>Durum: " + String(obdConnected ? "OBD BAGLI" : "BAGLANTI YOK") + "</p>";

  s += "<form action='/mode'>";
  s += "<button name='m' value='0'>GUNLUK MOD</button>";
  s += "<button name='m' value='1'>CADDE MODU</button></form>";

  s += "<h3>RPM AYAR</h3><form action='/set'>";
  s += "<input name='s' placeholder='RPM BASLANGIC (" + String(rpmStart) + ")'><br>";
  s += "<input name='g' placeholder='YESIL SON (" + String(rpmGreenEnd) + ")'><br>";
  s += "<input name='y' placeholder='SARI SON (" + String(rpmYellowEnd) + ")'><br>";
  s += "<input name='r' placeholder='KIRMIZI SON (" + String(rpmRedEnd) + ")'><br>";
  s += "<button>Kaydet</button></form>";

  s += "<h3>LED</h3><form action='/led'>";
  s += "<input name='b' value='" + String(ledBrightness) + "' placeholder='Parlaklik'><br>";
  s += "<input name='t' value='" + String(strobeDelay) + "' placeholder='Strobe Hiz (ms)'><br>";
  s += "<button>Kaydet</button></form>";

  s += "</body></html>";
  return s;
}

/* ================= LED FONKS ================= */
void drawCenter(int p, CRGB c) {
  for (int i = 0; i < p; i++) {
    if (centerLeft - i >= 0) leds[centerLeft - i] = c;
    if (centerRight + i < NUM_LEDS) leds[centerRight + i] = c;
  }
}

/* ---------- NON BLOCKING STROBE ---------- */
void shiftStrobe() {
  unsigned long now = millis();
  if (now - lastStrobeTime >= strobeDelay) {
    lastStrobeTime = now;
    strobeState = !strobeState;
    fill_solid(leds, NUM_LEDS, strobeState ? CRGB::Red : CRGB::White);
    FastLED.show();
  }
}

/* ================= MODLAR ================= */
void gunluk(int rpm) {
  FastLED.clear();

  if (rpm < rpmStart) { FastLED.show(); return; }

  if (rpm < rpmGreenEnd) {
    drawCenter(map(rpm, rpmStart, rpmGreenEnd, 1, 2), CRGB::Green);
    FastLED.show();
  }
  else if (rpm < rpmYellowEnd) {
    drawCenter(map(rpm, rpmGreenEnd, rpmYellowEnd, 2, 4), CRGB::Yellow);
    FastLED.show();
  }
  else if (rpm < rpmRedEnd) {
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
  }
  else {
    shiftStrobe();
  }
}

void cadde(int rpm) {

  if (rpm < 800) {
    FastLED.clear();
    FastLED.show();
    return;
  }

  if (rpm >= rpmRedEnd) {
    shiftStrobe();
    return;
  }

  if (millis() - lastChaseTime > (rpm < 3000 ? 120 : 60)) {
    lastChaseTime = millis();

    fadeToBlackBy(leds, NUM_LEDS, 100);
    leds[chaseIndex] = CHSV(hue++, 255, 255);
    chaseIndex = (chaseIndex + 1) % NUM_LEDS;
    FastLED.show();
  }
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  EEPROM.get(0, currentMode);
  EEPROM.get(4, rpmStart);
  EEPROM.get(8, rpmGreenEnd);
  EEPROM.get(12, rpmYellowEnd);
  EEPROM.get(16, rpmRedEnd);
  EEPROM.get(20, ledBrightness);
  EEPROM.get(24, strobeDelay);

  if (rpmStart <= 0) {
    currentMode = MODE_GUNLUK;
    rpmStart = 2200;
    rpmGreenEnd = 3000;
    rpmYellowEnd = 3300;
    rpmRedEnd = 3600;
    ledBrightness = BRIGHTNESS;
    strobeDelay = 50;
  }

  WiFi.softAP(ssid, password);

  server.on("/", [](){ server.send(200, "text/html", page()); });

  server.on("/mode", [](){
    currentMode = server.arg("m").toInt();
    EEPROM.put(0, currentMode); EEPROM.commit();
    server.send(200, "text/html", page());
  });

  server.on("/set", [](){
    if(server.arg("s")!="") rpmStart = server.arg("s").toInt();
    if(server.arg("g")!="") rpmGreenEnd = server.arg("g").toInt();
    if(server.arg("y")!="") rpmYellowEnd = server.arg("y").toInt();
    if(server.arg("r")!="") rpmRedEnd = server.arg("r").toInt();

    EEPROM.put(4, rpmStart);
    EEPROM.put(8, rpmGreenEnd);
    EEPROM.put(12, rpmYellowEnd);
    EEPROM.put(16, rpmRedEnd);
    EEPROM.commit();
    server.send(200, "text/html", page());
  });

  server.on("/led", [](){
    if(server.arg("b")!="") {
      ledBrightness = constrain(server.arg("b").toInt(), 0, 255);
      FastLED.setBrightness(ledBrightness);
      EEPROM.put(20, ledBrightness);
    }
    if(server.arg("t")!="") {
      strobeDelay = constrain(server.arg("t").toInt(), 5, 500);
      EEPROM.put(24, strobeDelay);
    }
    EEPROM.commit();
    server.send(200, "text/html", page());
  });

  server.begin();

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(ledBrightness);

  /* 🔵 BAĞLANIYOR */
  leds[0] = CRGB::Blue;
  FastLED.show();

  SerialBT.begin("ESP32_SHIFT_LIGHT", true);

  if (SerialBT.connect(address) && elm327.begin(SerialBT, true, 2000)) {
    obdConnected = true;
    leds[0] = CRGB::Green;   // 🟢 BAĞLANDI
  } else {
    leds[0] = CRGB::Red;     // 🔴 HATA
  }

  FastLED.show();
}

/* ================= LOOP ================= */
void loop() {
  server.handleClient();

  if (!obdConnected) return;

  int rpm = elm327.rpm();
  if (elm327.nb_rx_state != ELM_SUCCESS) return;

  if (currentMode == MODE_GUNLUK) gunluk(rpm);
  else cadde(rpm);
}
