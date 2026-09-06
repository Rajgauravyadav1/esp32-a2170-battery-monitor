#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

/*
 * ESP32 A2170 Battery Monitor
 * ---------------------------
 * This project started with a simple laptop battery pack and a question:
 * what's actually talking to it over those two tiny data lines?
 *
 * The A2170 fuel gauge IC has no public datasheet — it shows up quietly
 * inside Lapcare and Apexe battery packs, grouped alongside known chips
 * like the BQ2084, BQ3060, and BQ8050. That family clue was the only lead,
 * so this code was built on the assumption that it speaks the standard
 * Smart Battery Data Specification (SBS) over SMBus.
 *
 * What follows is the result of reverse-engineering that conversation —
 * reading voltage, current, temperature, and charge data straight off
 * the pack, while carefully separating out the registers that are still
 * unconfirmed guesses.
 *
 * Documented and shared as part of the Techhi journey (YouTube @techhi1),
 * for anyone else trying to make sense of the same silent little chip.
 *
 * GitHub: https://github.com/Rajgauravyadav1/esp32-a2170-battery-monitor
 * Found something new about the A2170? Fork it, test it, open a PR.
 */

#define SDA_PIN 21
#define SCL_PIN 22
#define BMS_ADDR 0x0B
#define NUM_CELLS 3   // adjust to match your actual pack (3S ~ 12V nominal)

const char* ap_ssid = "BatteryMonitor";
const char* ap_password = "12345678";

WebServer server(80);

bool readWordSMBus(uint8_t cmd, uint16_t *value) {
  Wire.beginTransmission(BMS_ADDR);
  Wire.write(cmd);
  uint8_t err = Wire.endTransmission(false);
  if (err != 0) return false;

  uint8_t got = Wire.requestFrom(BMS_ADDR, (uint8_t)2);
  if (got != 2) return false;

  uint8_t lsb = Wire.read();
  uint8_t msb = Wire.read();
  *value = (msb << 8) | lsb;
  return true;
}

void handleRoot() {
  uint16_t raw;
  bool ok;

  String page = "<html><head><meta http-equiv='refresh' content='2'>";
  page += "<style>body{font-family:Arial;text-align:center;} h1{color:#2c3e50;} .data{font-size:18px;} .err{color:#c0392b;} .note{color:#888;font-size:12px;}</style>";
  page += "</head><body>";
  page += "<h1>A2170 SMBus Battery Monitor</h1><div class='data'>";

  ok = readWordSMBus(0x09, &raw); // Voltage, mV
  page += "<p><b>Pack Voltage:</b> " + (ok ? String(raw / 1000.0, 2) + " V" : String("<span class='err'>read error</span>")) + "</p>";

  ok = readWordSMBus(0x0A, &raw); // Current, signed mA
  page += "<p><b>Pack Current:</b> " + (ok ? String(((int16_t)raw) / 1000.0, 3) + " A" : String("<span class='err'>read error</span>")) + "</p>";

  ok = readWordSMBus(0x0D, &raw); // RelativeStateOfCharge, %
  page += "<p><b>SOC:</b> " + (ok ? String(raw) + " %" : String("<span class='err'>read error</span>")) + "</p>";

  ok = readWordSMBus(0x08, &raw); // Temperature, 0.1K
  page += "<p><b>Temperature:</b> " + (ok ? String((raw / 10.0) - 273.15, 1) + " &deg;C" : String("<span class='err'>read error</span>")) + "</p>";

  page += "<h2>Cell Voltages:</h2>";
  for (int i = 0; i < NUM_CELLS; i++) {
    ok = readWordSMBus(0x3F + i, &raw);
    page += "Cell " + String(i + 1) + ": " + (ok ? String(raw / 1000.0, 3) + " V<br>" : String("<span class='err'>read error</span><br>"));
  }

  ok = readWordSMBus(0x46, &raw);
  page += "<p><b>FETs:</b> ";
  if (ok) {
    page += "DSG=" + String((raw & 0x0001) ? "ON" : "OFF");
    page += " CHG=" + String((raw & 0x0002) ? "ON" : "OFF");
  } else {
    page += "<span class='err'>read error</span>";
  }
  page += "</p>";

  page += "<p class='note'>Cell voltage &amp; FET status use guessed manufacturer registers — confirm against A2170 docs if available</p>";
  page += "</div></body></html>";

  server.send(200, "text/html", page);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nStarting A2170 SMBus Battery Monitor...");

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  WiFi.softAP(ap_ssid, ap_password);
  Serial.print("AP Started. Connect to Wi-Fi: ");
  Serial.println(ap_ssid);
  Serial.print("Access Dashboard at: http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
}