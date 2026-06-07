# 🫀 PPG Heart Rate Monitor — MAX30102 + Arduino Uno

> **Perancangan Sensor Biomedis Photoplethysmography (PPG)**  
> Program Studi Teknik Komputer · Fakultas Ilmu Komputer · Universitas Brawijaya · 2025/2026

[![Arduino](https://img.shields.io/badge/Platform-Arduino%20Uno-00979D?style=flat-square&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Sensor](https://img.shields.io/badge/Sensor-MAX30102-red?style=flat-square)](https://datasheets.maximintegrated.com/en/ds/MAX30102.pdf)
[![Display](https://img.shields.io/badge/Display-OLED%20SSD1306-blue?style=flat-square)](#)
[![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)](LICENSE)
[![MAPE](https://img.shields.io/badge/MAPE-0.838%25-brightgreen?style=flat-square)](#hasil-pengujian)

---

## 📋 Deskripsi Proyek

Sistem pemantau detak jantung berbasis PPG (*Photoplethysmography*) menggunakan sensor **MAX30102** yang terhubung ke **Arduino Uno**, dengan hasil pengukuran ditampilkan secara *real-time* pada layar **OLED SSD1306 128×32**.

Sensor bekerja dengan memancarkan cahaya **merah (660 nm)** dan **inframerah (940 nm)** ke jaringan kulit pada ujung jari. Perubahan intensitas pantulan akibat fluktuasi volume darah direkam sebagai sinyal PPG, kemudian diproses untuk menghitung **BPM (*Beats Per Minute*)**.

### ✨ Fitur Utama
- Deteksi detak jantung (*Heart Rate*) secara *real-time*
- Tampilan animasi ikon jantung (kecil ↔ besar) sinkron dengan setiap detak
- Rata-rata BPM rolling dari 4 sampel terakhir untuk nilai yang stabil
- Deteksi otomatis keberadaan jari (threshold IR > 7000)
- Akurasi tinggi: **MAE = 0.833 BPM**, **MAPE = 0.838%**
- Sistem *standalone* — tidak membutuhkan koneksi ke PC atau smartphone

---

## 🔧 Komponen Hardware

| Komponen | Spesifikasi | Fungsi |
|---|---|---|
| **Arduino Uno R3** | ATmega328P, 16 MHz, 5V | Mikrokontroler utama |
| **Sensor MAX30102** | LED Merah 660nm + IR 940nm, ADC 18-bit, I²C | Deteksi PPG |
| **OLED SSD1306** | 128×32 px, I²C | Tampilan hasil BPM |
| **Breadboard** | 400/830 lubang | Perakitan prototipe |
| **Kabel Jumper** | Male-to-Male / Male-to-Female | Koneksi antar komponen |

---

## 🔌 Wiring / Koneksi Pin

```
MAX30102          Arduino Uno
─────────────────────────────
VCC       →       3.3V
GND       →       GND
SDA       →       A4 (SDA)
SCL       →       A5 (SCL)

OLED SSD1306      Arduino Uno
─────────────────────────────
VCC       →       5V (atau 3.3V)
GND       →       GND
SDA       →       A4 (SDA)
SCL       →       A5 (SCL)
```

> ⚠️ **Penting:** MAX30102 menggunakan logika 3.3V. Hubungkan ke pin **3.3V** Arduino Uno, bukan 5V.

---

## 💻 Software & Library

| Library | Versi | Sumber |
|---|---|---|
| `Wire.h` | Built-in | Arduino Core |
| `Adafruit_GFX.h` | ≥1.11 | [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library) |
| `Adafruit_SSD1306.h` | ≥2.5 | [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) |
| `MAX30105.h` | ≥1.1 | [SparkFun MAX3010x Library](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library) |
| `heartRate.h` | Built-in (SparkFun) | SparkFun MAX3010x Library |

### Instalasi Library (Arduino IDE)

1. Buka **Arduino IDE** → `Sketch` → `Include Library` → `Manage Libraries`
2. Cari dan install:
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
   - `SparkFun MAX3010x Pulse and Proximity Sensor Library`

---

## 🚀 Cara Penggunaan

1. Clone repo ini:
   ```bash
   git clone https://github.com/[username]/ppg-heart-rate-monitor.git
   ```
2. Buka file `src/ppg_heart_rate.ino` di **Arduino IDE**
3. Install semua library yang dibutuhkan (lihat bagian di atas)
4. Pilih board: `Tools` → `Board` → **Arduino Uno**
5. Pilih port COM yang sesuai
6. Klik **Upload** ⬆️
7. Letakkan ujung jari pada sensor MAX30102
8. Tunggu ±5 detik untuk pembacaan BPM stabil, ±12 detik untuk BPM rata-rata stabil

---

## ⚙️ Alur Kerja Sistem

```
┌─────────────┐     Cahaya IR/Merah      ┌──────────────┐
│  MAX30102   │ ──────────────────────── │  Ujung Jari  │
│   Sensor    │ ←── Pantulan cahaya ──── │  (Jaringan)  │
└──────┬──────┘                          └──────────────┘
       │ I²C (A4/A5)
       ▼
┌─────────────────────────────────────────┐
│             Arduino Uno                 │
│  1. Baca nilai IR dari MAX30102         │
│  2. Deteksi puncak gelombang (checkForBeat) │
│  3. Hitung interval antar denyut (Δt)  │
│  4. BPM = 60 / (Δt / 1000)             │
│  5. Rolling average dari 4 sampel      │
└──────┬──────────────────────────────────┘
       │ I²C (A4/A5)
       ▼
┌─────────────┐
│ OLED 128×32 │  →  Tampilkan BPM + animasi ikon jantung
└─────────────┘
```

---

## 📊 Hasil Pengujian

Pengujian dilakukan dalam 2 kondisi: **duduk normal** dan **setelah 20x push-up**, pada 3 subjek berbeda. Nilai prototipe dibandingkan dengan **pulse oximeter komersial** sebagai referensi.

### Perbandingan BPM Rata-rata vs Oximeter

| Subjek | Kondisi | Prototype (rerata) | Oximeter | Selisih |
|---|---|:---:|:---:|:---:|
| Arbi | Normal | 97 | 97 | 0 |
| Arbi | Post push-up | 102 | 102 | 0 |
| Maulana | Normal | 92 | 92 | 0 |
| Maulana | Post push-up | 117 | 115 | 2 |
| Radhi | Normal | 84 | 82 | 2 |
| Radhi | Post push-up | 97 | 96 | 1 |

### Metrik Akurasi

| Metrik | Nilai | Interpretasi |
|---|:---:|---|
| **MAE** | **0.833 BPM** | < 1 BPM → sangat akurat |
| **MAPE** | **0.838%** | < 5% threshold → akurat |

> 📌 Error BPM instan yang lebih tinggi (3–21%) pada kondisi post push-up disebabkan oleh perubahan irama jantung yang mendadak, membuat deteksi puncak kurang stabil. Namun nilai **BPM rata-rata** tetap sangat akurat (error < 2.3%).

---

## 📁 Struktur Repositori

```
ppg-heart-rate-monitor/
├── src/
│   └── ppg_heart_rate.ino      # Kode program utama Arduino
├── docs/
│   ├── laporan_akhir.pdf        # Laporan lengkap proyek
│   └── wiring_diagram.png       # Diagram koneksi hardware
├── results/
│   └── measurement_data.csv     # Data hasil pengujian BPM
├── README.md
└── LICENSE
```

---

## 👥 Tim Pengembang

| Nama | NIM | Kontribusi |
|---|---|---|
| **Arbi Yusuf Ramanda** | 235150300111022 | Riset desain wiring hardware, kode program |
| **Ahmad Maulana Maky** | 235150307111038 | Wiring hardware, perhitungan error pengujian |
| **Muhammad Radhi Rasyidi** | 235150307111041 | Wiring hardware, kode program |

**Dosen Pengajar:** Edita Rosana Widasari, S.T., M.T., M.Eng., Ph.D.

---

## 📚 Referensi

- Maxim Integrated. (2021). *MAX30102: High-Sensitivity Pulse Oximeter and Heart-Rate Sensor for Wearable Health.* Datasheet Rev. 7.
- SparkFun Electronics. (2016). *MAX30105/MAX30102 Breakout Board Hookup Guide.*
- Microchip Technology. (2018). *ATmega328P – 8-bit AVR Microcontroller.* Rev. 7810D.
- Mulyani, S., et al. (2024). Prototype PPG Secara Real-time sebagai Pendeteksi Dini Gangguan Detak Jantung. *JTECE, 06(02)*, 125–138.

---

## 📄 Lisensi

Proyek ini dilisensikan di bawah [MIT License](LICENSE).
