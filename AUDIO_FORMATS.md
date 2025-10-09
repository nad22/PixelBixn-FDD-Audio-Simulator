# 🎵 Audio-Formate Guide

## ✅ Unterstützte Formate

### 1. **MP3** (Empfohlen für Floppy-Sounds)
```
- Dateiendung: .mp3
- Kompression: Gut (Standard-Referenz)
- Qualität: Gut bei 64-128kbps
- Universell unterstützt
- Hardware-Decoder im ESP32
- Beispiel: 3 Sekunden Floppy-Sound ≈ 15-25KB
```

### 2. **WAV** (Für unkomprimierte Sounds)
```
- Dateiendung: .wav
- Kompression: Keine (unkomprimiert)
- Qualität: Perfekt
- Schnelle Verarbeitung
- Beispiel: 3 Sekunden Floppy-Sound ≈ 250KB
```

## 🎯 **Empfohlene Einstellungen für Floppy-Disk Sounds:**

### MP3 (Beste Wahl für ESP32):
```
Bitrate: 64-128 kbps
Sample Rate: 22.05 kHz oder 16 kHz
Channels: Mono
Dauer: 1-5 Sekunden
→ Dateigröße: 10-30KB pro Sound
```

### WAV (Für beste Qualität):
```
Sample Rate: 16 kHz oder 22.05 kHz (nicht 44.1kHz)
Bit Depth: 16-bit
Channels: Mono
Dauer: 1-3 Sekunden
→ Dateigröße: 32-100KB pro Sound
```

## 📊 **Speicher-Vergleich (3 Sekunden Floppy-Sound):**

| Format | Dateigröße | Anzahl Sounds (3MB) |
|--------|------------|-------------------|
| MP3 (64kbps)  | ~15KB      | ~200 Sounds       |
| MP3 (128kbps) | ~24KB      | ~125 Sounds       |
| WAV (16kHz)   | ~96KB      | ~31 Sounds        |
| WAV (22kHz)   | ~132KB     | ~23 Sounds        |

## 🔧 **Audio-Konvertierung:**

### Mit FFmpeg (kostenlos):
```bash
# WAV zu MP3 (optimiert für ESP32)
ffmpeg -i floppy_sound.wav -c:a mp3 -b:a 64k -ar 22050 -ac 1 floppy_sound.mp3

# WAV zu MP3 (höhere Qualität)
ffmpeg -i floppy_sound.wav -c:a mp3 -b:a 128k -ar 22050 -ac 1 floppy_sound.mp3

# WAV optimieren (reduzierte Sample Rate)
ffmpeg -i floppy_sound.wav -ar 16000 -ac 1 floppy_optimized.wav
```

### Online-Konverter:
- cloudconvert.com
- convertio.co
- online-audio-converter.com

## 💡 **Tipps für optimale Floppy-Sounds:**

1. **Mono statt Stereo**: Floppy-Geräusche sind mono → 50% Platzersparnis
2. **Niedrige Sample Rate**: 16-22kHz reicht für Maschinengeräusche
3. **Kurze Loops**: 1-3 Sekunden, dann Loop in Software
4. **Noise Shaping**: Entferne Stille am Anfang/Ende
5. **Normalisierung**: Einheitliche Lautstärke

## 🎮 **Typische Floppy-Disk Sounds:**

```
floppy_seek.opus     →  Kopf-Bewegung (zirp-zirp)
floppy_read.opus     →  Lese-Vorgang (rattern)
floppy_write.opus    →  Schreib-Vorgang (klick-rattern)
floppy_insert.opus   →  Diskette einlegen (klack)
floppy_eject.opus    →  Diskette auswerfen (chunk)
floppy_error.opus    →  Fehler-Sound (piep-piep)
floppy_motor.opus    →  Motor-Start (whirr)
```

## ⚡ **Performance-Hinweise:**

- **MP3**: Hardware-Dekodierung im ESP32, sehr effizient
- **WAV**: Keine Dekodierung nötig, aber viel Speicher
- **Lower Sample Rates**: 16-22kHz reichen für Maschinengeräusche
- **Mono**: Floppy-Sounds sind mono → 50% Speicher sparen

**Empfehlung**: Verwende **MP3 mit 64-128 kbps** für besten Kompromiss zwischen Dateigröße und Qualität!