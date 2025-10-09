# 🔊 Gain Control Implementation

## ✅ Funktionen implementiert:

### 1. Hardware-Steuerung
- **Pin**: GPIO 0 steuert MAX98357 GAIN Pin
- **LOW (0V)**: 9dB Verstärkung (leiser)
- **HIGH (3.3V)**: 15dB Verstärkung (lauter)

### 2. Web-Interface
- **Gain-Sektion** in der Weboberfläche
- Zeigt aktuellen Gain-Wert an (9dB oder 15dB)
- **Toggle-Button** zum Umschalten zwischen LOW/HIGH
- Automatische Anzeigen-Aktualisierung

### 3. Persistente Speicherung
- Gain-Setting wird in ESP32 Preferences gespeichert
- Einstellung überlebt Neustarts
- Laden beim Boot-Vorgang

### 4. API-Endpunkte
- **GET /api/status**: Liefert aktuellen Gain-Status
- **POST /api/gain**: Schaltet Gain um (Toggle)

## 🔧 Verwendung:

### Hardware-Anschluss:
```
ESP32-C3 GPIO 0  →  MAX98357 GAIN Pin
```

### Web-Interface:
1. Öffne http://192.168.4.1
2. Scrolle zu "Audio Gain" Sektion
3. Klicke "Switch to HIGH (15dB)" für höhere Verstärkung
4. Klicke "Switch to LOW (9dB)" für niedrigere Verstärkung

### Automatische Funktionen:
- Beim Systemstart: Gain wird aus gespeicherter Konfiguration geladen
- Bei jedem Umschalten: Neue Einstellung wird automatisch gespeichert
- Verstärker-Status wird auf Display und Web-Interface angezeigt

## 📊 Gain-Unterschied:
- **9dB (LOW)**: Normale Lautstärke, stromsparend
- **15dB (HIGH)**: +6dB lauter = doppelt so laut wahrgenommen
- **Pin-Steuerung**: Einfache digitale HIGH/LOW Schaltung

## 🎛️ Integration:
- Gain-Steuerung ist vollständig in das bestehende Web-Interface integriert
- Funktioniert zusammen mit Volume-Control (Software) und Gain (Hardware)
- Beide Einstellungen werden persistent gespeichert