# WAV Playback über I2S - Implementierung

## Übersicht

Das System verwendet jetzt **echtes I2S** (Inter-IC Sound) für die Audio-Wiedergabe über den MAX98357 Verstärker, basierend auf der funktionierenden Implementierung aus dem UDM-Packet-Radio-Internet-Gateway Projekt.

**WICHTIG:** Die Wiedergabe läuft in einem **separaten FreeRTOS Task auf Core 0**, sodass Web-Interface und WiFi (Core 1) nicht blockiert werden!

## Hardware-Konfiguration

### I2S Audio Pins (GPIO 0-4)
- **I2S_BCLK:** GPIO 3 (Bit Clock)
- **I2S_LRC:** GPIO 4 (Left/Right Clock / Word Select)
- **I2S_DOUT:** GPIO 2 (Audio Data Out)
- **I2S_SD:** GPIO 1 (Shutdown/Enable Control)
- **I2S_GAIN:** GPIO 0 (Gain Select: LOW=9dB, HIGH=15dB)

### I2S Konfiguration
- **Sample Rate:** 44.1 kHz (CD-Qualität)
- **Bit Depth:** 16-bit
- **Channels:** Mono (konvertiert Stereo automatisch)
- **Format:** Standard I2S
- **DMA Buffer:** 2 x 128 samples

## Funktionsweise

### 1. Initialisierung (beim Boot)
```cpp
void initAudio() {
    // I2S Hardware-Treiber installieren
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    
    // Control Pins
    pinMode(I2S_SD, OUTPUT);    // Amplifier Enable
    pinMode(I2S_GAIN, OUTPUT);  // Gain Control
}
```

### 2. Startup-Test
- **Kurzer 1kHz Piep (200ms)** beim Hochfahren
- Mit Fade-in/Fade-out Envelope (vermeidet Klicks)
- Bestätigt funktionierende Audio-Hardware
- Audio wird **vor** dem Display initialisiert für sofortige Verfügbarkeit

### 3. WAV-Wiedergabe

#### Architektur: Multi-Core Non-Blocking

```
Core 1 (WiFi/Web):           Core 0 (Audio):
┌─────────────────┐          ┌──────────────────┐
│ Web Server      │          │ Audio Task       │
│ WiFi Stack      │          │ (FreeRTOS)       │
│ File Upload     │          │                  │
└────────┬────────┘          └────────┬─────────┘
         │                            │
         │  playSound()               │
         ├────────────────────────────>│
         │  (non-blocking!)           │
         │                            │
         │                      ┌─────▼──────┐
         │                      │ Read WAV   │
         │                      │ Parse      │
         │                      │ Play I2S   │
         │                      └────────────┘
         │  User can continue          │
         │  browsing web!              │
         └─────────────────────────────┘
            Both cores run parallel
```

#### Unterstützte Formate
- **Audio Codec:** PCM (unkomprimiert)
- **Bit Depth:** 16-bit
- **Sample Rate:** beliebig (44.1kHz, 22.05kHz, 8kHz etc.)
- **Channels:** Mono oder Stereo (wird zu Mono gemischt)

#### Ablauf
1. **Playback Request (Core 1):**
   - `playSound()` wird aufgerufen (z.B. von Web API)
   - Setzt `audioPlaybackRequested = true` via Mutex
   - Kehrt **sofort zurück** (non-blocking!)
   - Web-Interface bleibt responsiv

2. **Background Playback (Core 0):**
   - Audio-Task erkennt Request
   - Ruft `playWavFileBlocking()` auf
   - **Header-Parsing:**
     - Prüft RIFF/WAVE Signatur
     - Liest Audio-Parameter (Sample Rate, Channels, Bit Depth)
     - Validiert Format (nur PCM 16-bit)

3. **Chunk-basierte Wiedergabe:**
   - Liest 512 Samples gleichzeitig
   - Wendet Lautstärke an (0-100%)
   - Konvertiert Stereo → Mono bei Bedarf
   - Schreibt via I2S DMA an MAX98357
   - **Yielded regelmäßig** für Scheduler (alle 4096 Samples)

4. **Stopp-Mechanismus:**
   - `stopSound()` setzt `audioStopRequested = true`
   - Audio-Task prüft Flag in jedem Chunk
   - Bricht Wiedergabe sofort ab
   - Löscht I2S DMA-Buffer
   - Deaktiviert Verstärker

## Lautstärke-Steuerung

### Software-Lautstärke
```cpp
config.volume = 50;  // 0-100%
// Angewendet in playSound() auf jeden Sample
buffer[i] = (int16_t)(buffer[i] * config.volume / 100);
```

### Hardware-Gain (MAX98357)
```cpp
setGain(false);  // LOW  = 9dB  (Standard)
setGain(true);   // HIGH = 15dB (+6dB mehr)
```

## Web-API Endpunkte

### WAV hochladen
```
POST /api/upload
Content-Type: multipart/form-data
```

### WAV abspielen
```
POST /api/play
Content-Type: application/json
{"filename": "sound.wav"}
```

### Wiedergabe stoppen
```
POST /api/stop
```

### Gain einstellen
```
POST /api/gain
Content-Type: application/json
{"gain": true}  // true=15dB, false=9dB
```

### WAV löschen
```
POST /api/delete
Content-Type: application/json
{"filename": "sound.wav"}
```

## Unterschied zu vorheriger Implementierung

### Version 1 (PWM - nicht funktionsfähig)
- ❌ Nur GPIO 2 mit PWM-Signal
- ❌ Keine echten I2S Clocks (BCLK, LRC)
- ❌ MAX98357 konnte kein Audio ausgeben
- ❌ Nur Simulation basierend auf Datei-Größe

### Version 2 (I2S Blocking - langsam)
- ✅ 3 I2S Signale: BCLK, LRC, DIN
- ✅ ESP32 I2S Hardware-Treiber (`driver/i2s.h`)
- ✅ Echte WAV-Wiedergabe
- ❌ Blockierte gesamtes System während Wiedergabe
- ❌ Web-Interface extrem langsam
- ❌ Playback-Start verzögert

### Version 3 (I2S FreeRTOS Task - aktuell) ⭐
- ✅ 3 I2S Signale: BCLK, LRC, DIN
- ✅ ESP32 I2S Hardware-Treiber (`driver/i2s.h`)
- ✅ Echte WAV-Wiedergabe mit Header-Parsing
- ✅ **Non-blocking:** Audio läuft auf Core 0
- ✅ **Web responsive:** WiFi/Web läuft auf Core 1
- ✅ **Sofortiger Start:** playSound() kehrt sofort zurück
- ✅ Thread-safe mit Mutex
- ✅ Automatische Sample-Rate Anpassung
- ✅ Stereo → Mono Konvertierung
- ✅ Software-Lautstärke + Hardware-Gain
- ✅ DMA-basiert für flüssige Wiedergabe
- ✅ Task Yielding für Scheduler Fairness

## FreeRTOS Task Details

### Audio Playback Task
```cpp
TaskHandle_t audioTaskHandle;
Priority: 1 (niedrig, gut für Audio)
Stack: 8192 bytes (8KB)
Core: 0 (WiFi/Web auf Core 1)
```

### Thread-Safety
```cpp
SemaphoreHandle_t audioMutex;  // Mutex für thread-safe Zugriff
```

### Kommunikation zwischen Cores
```cpp
volatile bool audioPlaybackRequested;  // Core 1 → Core 0
volatile bool audioStopRequested;      // Core 1 → Core 0
String audioFilenameToPlay;            // Protected by mutex
```

## Speicherverbrauch

- **RAM:** 12.5% (41,036 / 327,680 bytes)
- **Flash:** 30.0% (943,746 / 3,145,728 bytes)
- **Audio Buffer:** 1024 bytes (512 samples × 16-bit)

## Stromsparmodus

Der MAX98357 Verstärker wird automatisch deaktiviert:
- Nach `stopSound()` Aufruf
- Wenn keine Wiedergabe aktiv ist
- Spart Strom und reduziert Rauschen

## Debugging

Serial Monitor zeigt:
```
Initializing audio (I2S)...
I2S Audio initialized:
  I2S_BCLK: GPIO 3
  I2S_LRC:  GPIO 4
  I2S_DOUT: GPIO 2
  I2S_SD:   GPIO 1
  I2S_GAIN: GPIO 0
  Sample Rate: 44100 Hz
Audio playback task created on Core 0  ← FreeRTOS Task läuft
Audio ready - playing startup beep...
Startup beep complete - audio system ready!
Audio playback task started                ← Task bereit
```

Bei WAV-Wiedergabe:
```
Playback requested: sound.wav (non-blocking)  ← Sofortige Rückkehr!
Playing WAV file: /sound.wav (123456 bytes)
WAV Format: 1, Channels: 2, SampleRate: 44100, BitsPerSample: 16
WAV playback complete - played 61728 samples
```

Bei Stopp:
```
Audio stop requested
Playback stopped by user
```

## Bekannte Limitierungen

1. **Nur PCM-Format:** Keine MP3/OGG/FLAC Unterstützung
2. **Nur 16-bit:** 8-bit oder 24-bit WAV nicht unterstützt
3. **Ein Audio-File gleichzeitig:** Kein Mixing mehrerer Streams
4. **RAM-begrenzt:** 512-Sample Chunks (aber kein Problem für große Dateien)

## Performance

### Speicherverbrauch
- **RAM:** 12.5% (41,068 / 327,680 bytes)
- **Flash:** 30.0% (944,590 / 3,145,728 bytes)
- **Audio Buffer:** 1024 bytes (512 samples × 16-bit)
- **Task Stack:** 8192 bytes (8KB für Audio-Task)

### Reaktionszeiten
- **playSound() Return:** < 1ms (non-blocking!)
- **Playback Start:** ~50-100ms (File-Open + Header-Parse)
- **Web-Interface:** Voll responsiv während Wiedergabe
- **Stop Response:** < 50ms (nächster Chunk-Check)

## Nächste Schritte (Optional)

- [ ] Non-blocking Wiedergabe mit FreeRTOS Task
- [ ] Playlist-Funktion
- [ ] Fade-in/Fade-out bei Play/Stop
- [ ] Equalizer (Bass/Treble)
- [ ] Sample-Rate Resampling für optimale Qualität
