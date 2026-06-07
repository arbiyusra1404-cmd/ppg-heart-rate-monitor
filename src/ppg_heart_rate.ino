/*
 * ============================================================
 *  PPG Heart Rate Monitor
 *  MAX30102 + OLED SSD1306 (128x32) + Arduino Uno
 * ============================================================
 *  Menggunakan algoritma deteksi detak jantung dari SparkFun
 *  (PBA - Pan-Tompkins Based Algorithm) via library MAX30105.
 *
 *  Fitur:
 *  - Deteksi BPM real-time dari sensor optik MAX30102
 *  - Rolling average BPM dari 4 sampel terakhir
 *  - Animasi ikon jantung sinkron dengan setiap detak
 *  - Deteksi otomatis keberadaan jari (IR threshold > 7000)
 *
 *  Wiring:
 *  MAX30102 VCC  → Arduino 3.3V
 *  MAX30102 GND  → Arduino GND
 *  MAX30102 SDA  → Arduino A4
 *  MAX30102 SCL  → Arduino A5
 *  OLED VCC      → Arduino 5V
 *  OLED GND      → Arduino GND
 *  OLED SDA      → Arduino A4
 *  OLED SCL      → Arduino A5
 *
 *  Library yang dibutuhkan:
 *  - Adafruit GFX Library
 *  - Adafruit SSD1306
 *  - SparkFun MAX3010x Pulse and Proximity Sensor Library
 * ============================================================
 *  Tim Pengembang:
 *  - Arbi Yusuf Ramanda     (235150300111022)
 *  - Ahmad Maulana Maky     (235150307111038)
 *  - Muhammad Radhi Rasyidi (235150307111041)
 *
 *  Teknik Komputer, FILKOM, Universitas Brawijaya — 2025/2026
 * ============================================================
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "heartRate.h"

// ── Konfigurasi OLED ─────────────────────────────────────────
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  32
#define OLED_RESET     -1   // Tidak menggunakan pin reset terpisah
#define OLED_I2C_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ── Konfigurasi Sensor ────────────────────────────────────────
MAX30105 particleSensor;

// ── Parameter BPM ─────────────────────────────────────────────
const byte RATE_SIZE = 4;   // Jumlah sampel untuk rolling average
byte  rates[RATE_SIZE];     // Buffer BPM
byte  rateSpot   = 0;
long  lastBeat   = 0;       // Timestamp detak terakhir (ms)

float beatsPerMinute;       // BPM instan
int   beatAvg;              // BPM rata-rata (rolling)

// ── Bitmap Ikon Jantung Kecil (24x21) ─────────────────────────
static const unsigned char PROGMEM logo2_bmp[] = {
  0x03, 0xC0, 0xF0, 0x06, 0x71, 0x8C, 0x0C, 0x1B, 0x06, 0x18, 0x0E,
  0x02, 0x10, 0x0C, 0x03, 0x10,
  0x04, 0x01, 0x10, 0x04, 0x01, 0x10, 0x40, 0x01, 0x10, 0x40, 0x01,
  0x10, 0xC0, 0x03, 0x08, 0x88,
  0x02, 0x08, 0xB8, 0x04, 0xFF, 0x37, 0x08, 0x01, 0x30, 0x18, 0x01,
  0x90, 0x30, 0x00, 0xC0, 0x60,
  0x00, 0x60, 0xC0, 0x00, 0x31, 0x80, 0x00, 0x1B, 0x00, 0x00, 0x0E,
  0x00, 0x00, 0x04, 0x00,
};

// ── Bitmap Ikon Jantung Besar (32x32) ─────────────────────────
static const unsigned char PROGMEM logo3_bmp[] = {
  0x01, 0xF0, 0x0F, 0x80, 0x06, 0x1C, 0x38, 0x60, 0x18, 0x06, 0x60,
  0x18, 0x10, 0x01, 0x80, 0x08,
  0x20, 0x01, 0x80, 0x04, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00,
  0x02, 0xC0, 0x00, 0x08, 0x03,
  0x80, 0x00, 0x08, 0x01, 0x80, 0x00, 0x18, 0x01, 0x80, 0x00, 0x1C,
  0x01, 0x80, 0x00, 0x14, 0x00,
  0x80, 0x00, 0x14, 0x00, 0x80, 0x00, 0x14, 0x00, 0x40, 0x10, 0x12,
  0x00, 0x40, 0x10, 0x12, 0x00,
  0x7E, 0x1F, 0x23, 0xFE, 0x03, 0x31, 0xA0, 0x04, 0x01, 0xA0, 0xA0,
  0x0C, 0x00, 0xA0, 0xA0, 0x08,
  0x00, 0x60, 0xE0, 0x10, 0x00, 0x20, 0x60, 0x20, 0x06, 0x00, 0x40,
  0x60, 0x03, 0x00, 0x40, 0xC0,
  0x01, 0x80, 0x01, 0x80, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x60, 0x06,
  0x00, 0x00, 0x30, 0x0C, 0x00,
  0x00, 0x08, 0x10, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0x03, 0xC0,
  0x00, 0x00, 0x01, 0x80, 0x00
};

// ── Helper: Tampilkan pesan tunggu ─────────────────────────────
void showWaitMessage() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15, 10);
  display.println("Place your finger");
  display.display();
}

// ── Setup ──────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println(F("PPG Heart Rate Monitor — Initializing..."));

  // Inisialisasi OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println(F("[ERROR] OLED gagal diinisialisasi! Cek koneksi."));
    while (1);
  }
  display.display();
  delay(1000);
  display.clearDisplay();

  // Inisialisasi MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("[ERROR] MAX30102 tidak terdeteksi! Cek kabel/power."));
    while (1);
  }

  // Konfigurasi sensor
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);  // LED merah (rendah) — indikator aktif
  particleSensor.setPulseAmplitudeGreen(0);   // Matikan LED hijau (tidak digunakan)

  Serial.println(F("[OK] Sensor siap. Letakkan jari pada sensor..."));
  showWaitMessage();
}

// ── Main Loop ──────────────────────────────────────────────────
void loop() {
  long irValue = particleSensor.getIR();

  if (irValue > 7000) {
    // ── Jari terdeteksi ─────────────────────────────────────
    if (checkForBeat(irValue) == true) {
      // Hitung BPM dari interval antar detak
      long delta      = millis() - lastBeat;
      lastBeat        = millis();
      beatsPerMinute  = 60.0 / (delta / 1000.0);

      // Simpan ke buffer jika dalam range fisiologis (20–254 BPM)
      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;

        // Hitung rolling average
        beatAvg = 0;
        for (byte x = 0; x < RATE_SIZE; x++) beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }

      // Tampilkan ikon jantung BESAR saat detak terdeteksi
      display.clearDisplay();
      display.drawBitmap(0, 0, logo3_bmp, 32, 32, WHITE);
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(50, 0);
      display.println(F("BPM"));
      display.setCursor(50, 18);
      display.println(beatAvg);
      display.display();

    } else {
      // Tidak ada detak baru → tampilkan ikon jantung KECIL
      display.clearDisplay();
      display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE);
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(50, 0);
      display.println(F("BPM"));
      display.setCursor(50, 18);
      display.println(beatAvg);
      display.display();
    }

  } else {
    // ── Tidak ada jari ───────────────────────────────────────
    beatAvg = 0;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(30, 5);
    display.println(F("Please Place"));
    display.setCursor(30, 15);
    display.println(F("your finger"));
    display.display();
  }

  // Debug output ke Serial Monitor
  Serial.print(F("IR="));
  Serial.print(irValue);
  Serial.print(F(", BPM="));
  Serial.print(beatsPerMinute);
  Serial.print(F(", Avg BPM="));
  Serial.println(beatAvg);
}
