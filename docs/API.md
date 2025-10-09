# API Documentation

## Overview
The Floppy Disk Audio Simulator provides a comprehensive RESTful API for remote control and integration with other systems. All API endpoints return JSON responses and support CORS for web applications.

## Base URL
```
http://[device-ip]/api/
```

Default device IP addresses:
- **AP Mode**: `192.168.4.1`
- **Station Mode**: Assigned by router DHCP

## Authentication
Currently, no authentication is required. Future versions may include API key authentication for security.

## Response Format
All API responses follow this standard format:

### Success Response
```json
{
  "status": "success",
  "data": {
    // Response data here
  },
  "timestamp": "2025-10-09T12:34:56Z"
}
```

### Error Response
```json
{
  "status": "error",
  "error": {
    "code": "ERROR_CODE",
    "message": "Human readable error message"
  },
  "timestamp": "2025-10-09T12:34:56Z"
}
```

## Endpoints

### System Status

#### GET /api/status
Get current system status and playback information.

**Response:**
```json
{
  "status": "success",
  "data": {
    "playing": true,
    "currentSound": "floppy_seek.wav",
    "volume": 75,
    "uptime": 3600,
    "freeMemory": 234567,
    "wifiConnected": true,
    "nfcEnabled": true,
    "soundCount": 12
  }
}
```

**Fields:**
- `playing` (boolean): Whether audio is currently playing
- `currentSound` (string): Filename of current/last played sound
- `volume` (integer): Current volume level (0-100)
- `uptime` (integer): System uptime in seconds
- `freeMemory` (integer): Free heap memory in bytes
- `wifiConnected` (boolean): WiFi connection status
- `nfcEnabled` (boolean): NFC reader status
- `soundCount` (integer): Number of sound files available

#### GET /api/info
Get detailed system information.

**Response:**
```json
{
  "status": "success",
  "data": {
    "deviceName": "Floppy Disk Audio Simulator",
    "version": "1.0.0",
    "chipModel": "ESP32-C3",
    "cpuFreq": 160,
    "flashSize": 4194304,
    "freeFlash": 2097152,
    "macAddress": "AA:BB:CC:DD:EE:FF",
    "ipAddress": "192.168.4.1",
    "apClients": 2
  }
}
```

### Configuration Management

#### GET /api/config
Get current device configuration.

**Response:**
```json
{
  "status": "success",
  "data": {
    "volume": 75,
    "randomPlay": true,
    "nfcEnabled": true,
    "ssid": "FloppyDisk_AP",
    "autoPlay": false,
    "playbackMode": "random",
    "defaultSound": "floppy_seek.wav"
  }
}
```

#### POST /api/config
Update device configuration.

**Request Body:**
```json
{
  "volume": 80,
  "randomPlay": false,
  "nfcEnabled": true,
  "autoPlay": true
}
```

**Response:**
```json
{
  "status": "success",
  "data": {
    "updated": ["volume", "randomPlay", "autoPlay"],
    "message": "Configuration updated successfully"
  }
}
```

**Supported Configuration Keys:**
- `volume` (0-100): Audio volume level
- `randomPlay` (boolean): Enable random playback mode
- `nfcEnabled` (boolean): Enable NFC card triggering
- `autoPlay` (boolean): Auto-play on startup
- `defaultSound` (string): Default sound file to play

### Audio Playback Control

#### POST /api/play
Play a specific sound file.

**Request Body:**
```json
{
  "file": "floppy_seek.wav"
}
```

**Response:**
```json
{
  "status": "success",
  "data": {
    "playing": "floppy_seek.wav",
    "duration": 2.5,
    "message": "Playback started"
  }
}
```

#### POST /api/play-random
Play a random sound file from the collection.

**Request Body:** *(Optional)*
```json
{
  "exclude": ["error_sound.wav"]
}
```

**Response:**
```json
{
  "status": "success",
  "data": {
    "playing": "floppy_motor.wav",
    "selected": "random",
    "duration": 3.2
  }
}
```

#### POST /api/stop
Stop current audio playback.

**Response:**
```json
{
  "status": "success",
  "data": {
    "message": "Playback stopped",
    "wasPlaying": "floppy_seek.wav"
  }
}
```

#### POST /api/pause
Pause current audio playback.

**Response:**
```json
{
  "status": "success",
  "data": {
    "message": "Playback paused",
    "position": 1.25
  }
}
```

#### POST /api/resume
Resume paused audio playback.

**Response:**
```json
{
  "status": "success",
  "data": {
    "message": "Playback resumed",
    "position": 1.25
  }
}
```

### File Management

#### GET /api/sounds
List all available sound files.

**Query Parameters:**
- `format` (optional): Response format (`list` or `detailed`)
- `sort` (optional): Sort order (`name`, `size`, `date`)

**Response:**
```json
{
  "status": "success",
  "data": {
    "files": [
      {
        "name": "floppy_seek.wav",
        "size": 125440,
        "duration": 2.5,
        "format": "WAV",
        "sampleRate": 44100,
        "lastModified": "2025-10-09T10:30:00Z"
      },
      {
        "name": "floppy_motor.wav", 
        "size": 201600,
        "duration": 4.0,
        "format": "WAV",
        "sampleRate": 44100,
        "lastModified": "2025-10-09T11:15:00Z"
      }
    ],
    "totalCount": 2,
    "totalSize": 327040
  }
}
```

#### POST /api/upload
Upload a new sound file.

**Request:** Multipart form data with file
```
Content-Type: multipart/form-data
file: [binary file data]
```

**Response:**
```json
{
  "status": "success",
  "data": {
    "filename": "new_sound.wav",
    "size": 156800,
    "message": "File uploaded successfully"
  }
}
```

#### DELETE /api/sounds/{filename}
Delete a specific sound file.

**Response:**
```json
{
  "status": "success",
  "data": {
    "filename": "old_sound.wav",
    "message": "File deleted successfully"
  }
}
```

#### GET /api/sounds/{filename}
Download a specific sound file.

**Response:** Binary file data with appropriate headers
```
Content-Type: audio/wav
Content-Disposition: attachment; filename="floppy_seek.wav"
```

### Network Management

#### GET /api/wifi/scan
Scan for available WiFi networks.

**Response:**
```json
{
  "status": "success",
  "data": {
    "networks": [
      {
        "ssid": "HomeNetwork",
        "rssi": -45,
        "channel": 6,
        "security": "WPA2",
        "hidden": false
      },
      {
        "ssid": "OfficeWiFi",
        "rssi": -67,
        "channel": 11,
        "security": "WPA3",
        "hidden": false
      }
    ],
    "scanTime": "2025-10-09T12:34:56Z"
  }
}
```

#### POST /api/wifi/connect
Connect to a WiFi network.

**Request Body:**
```json
{
  "ssid": "HomeNetwork",
  "password": "secretpassword"
}
```

**Response:**
```json
{
  "status": "success",
  "data": {
    "ssid": "HomeNetwork",
    "connected": true,
    "ipAddress": "192.168.1.100",
    "message": "Connected successfully"
  }
}
```

#### POST /api/wifi/disconnect
Disconnect from current WiFi network.

**Response:**
```json
{
  "status": "success",
  "data": {
    "message": "Disconnected from WiFi",
    "apMode": true
  }
}
```

### Hardware Control

#### GET /api/hardware/nfc
Get NFC reader status and detected cards.

**Response:**
```json
{
  "status": "success",
  "data": {
    "enabled": true,
    "readerConnected": true,
    "cardPresent": false,
    "lastCard": {
      "uid": "04:A1:B2:C3",
      "type": "MIFARE Classic",
      "timestamp": "2025-10-09T12:30:00Z"
    }
  }
}
```

#### POST /api/hardware/nfc/enable
Enable or disable NFC functionality.

**Request Body:**
```json
{
  "enabled": true
}
```

**Response:**
```json
{
  "status": "success",
  "data": {
    "nfcEnabled": true,
    "message": "NFC reader enabled"
  }
}
```

#### GET /api/hardware/display
Get display status and control.

**Response:**
```json
{
  "status": "success",
  "data": {
    "enabled": true,
    "brightness": 255,
    "currentScreen": "status",
    "animationEnabled": true
  }
}
```

#### POST /api/hardware/display
Control display settings.

**Request Body:**
```json
{
  "brightness": 200,
  "animationEnabled": false,
  "screen": "custom",
  "message": "Hello World!"
}
```

**Response:**
```json
{
  "status": "success",
  "data": {
    "brightness": 200,
    "message": "Display updated"
  }
}
```

### System Management

#### POST /api/system/restart
Restart the device.

**Response:**
```json
{
  "status": "success",
  "data": {
    "message": "System restarting in 3 seconds"
  }
}
```

#### POST /api/system/factory-reset
Reset device to factory defaults.

**Request Body:**
```json
{
  "confirm": true,
  "keepSounds": false
}
```

**Response:**
```json
{
  "status": "success",
  "data": {
    "message": "Factory reset initiated",
    "restartIn": 5
  }
}
```

#### GET /api/system/logs
Get system log entries.

**Query Parameters:**
- `level` (optional): Log level filter (`debug`, `info`, `warn`, `error`)
- `limit` (optional): Maximum number of entries (default: 100)

**Response:**
```json
{
  "status": "success",  
  "data": {
    "logs": [
      {
        "timestamp": "2025-10-09T12:34:56Z",
        "level": "info",
        "message": "Audio playback started",
        "component": "audio"
      },
      {
        "timestamp": "2025-10-09T12:34:55Z",
        "level": "debug",
        "message": "NFC card detected",
        "component": "nfc"
      }
    ],
    "totalCount": 245,
    "filtered": 2
  }
}
```

## WebSocket API

For real-time updates, the device also provides WebSocket connections:

### Connection
```
ws://[device-ip]/ws
```

### Message Format
```json
{
  "type": "status_update",
  "data": {
    "playing": true,
    "currentSound": "floppy_seek.wav",
    "timestamp": "2025-10-09T12:34:56Z"
  }
}
```

### Message Types
- `status_update`: Playback status changes
- `nfc_card`: NFC card detected
- `volume_change`: Volume level changed
- `file_uploaded`: New file uploaded
- `file_deleted`: File deleted
- `config_changed`: Configuration updated

## Error Codes

| Code | Description |
|------|-------------|
| `INVALID_REQUEST` | Malformed request body or parameters |
| `FILE_NOT_FOUND` | Requested sound file does not exist |
| `UPLOAD_FAILED` | File upload failed (size, format, or space) |
| `PLAYBACK_ERROR` | Audio playback failed to start |
| `CONFIG_ERROR` | Invalid configuration parameter |
| `NETWORK_ERROR` | WiFi connection or network issue |
| `HARDWARE_ERROR` | Hardware component not responding |
| `STORAGE_FULL` | Insufficient storage space |
| `INVALID_FORMAT` | Unsupported file format |
| `SYSTEM_ERROR` | Internal system error |

## Rate Limiting

API endpoints are rate-limited to prevent abuse:
- **General endpoints**: 60 requests per minute
- **Upload endpoint**: 10 requests per minute
- **System endpoints**: 5 requests per minute

Exceeded rate limits return HTTP 429 with:
```json
{
  "status": "error",
  "error": {
    "code": "RATE_LIMIT_EXCEEDED",
    "message": "Too many requests, try again later",
    "retryAfter": 60
  }
}
```

## SDK Examples

### JavaScript/Node.js
```javascript
class FloppySimulatorAPI {
  constructor(baseUrl = 'http://192.168.4.1') {
    this.baseUrl = baseUrl;
  }
  
  async playSound(filename) {
    const response = await fetch(`${this.baseUrl}/api/play`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ file: filename })
    });
    return response.json();
  }
  
  async getStatus() {
    const response = await fetch(`${this.baseUrl}/api/status`);
    return response.json();
  }
  
  async uploadFile(file) {
    const formData = new FormData();
    formData.append('file', file);
    
    const response = await fetch(`${this.baseUrl}/api/upload`, {
      method: 'POST',
      body: formData
    });
    return response.json();
  }
}

// Usage
const api = new FloppySimulatorAPI();
await api.playSound('floppy_seek.wav');
```

### Python
```python
import requests
import json

class FloppySimulatorAPI:
    def __init__(self, base_url='http://192.168.4.1'):
        self.base_url = base_url
    
    def play_sound(self, filename):
        response = requests.post(
            f'{self.base_url}/api/play',
            json={'file': filename}
        )
        return response.json()
    
    def get_status(self):
        response = requests.get(f'{self.base_url}/api/status')
        return response.json()
    
    def upload_file(self, file_path):
        with open(file_path, 'rb') as f:
            files = {'file': f}
            response = requests.post(
                f'{self.base_url}/api/upload',
                files=files
            )
        return response.json()

# Usage
api = FloppySimulatorAPI()
status = api.get_status()
api.play_sound('floppy_seek.wav')
```

### curl Examples
```bash
# Get status
curl http://192.168.4.1/api/status

# Play specific sound
curl -X POST http://192.168.4.1/api/play \
  -H "Content-Type: application/json" \
  -d '{"file":"floppy_seek.wav"}'

# Upload file
curl -X POST http://192.168.4.1/api/upload \
  -F "file=@mysound.wav"

# Update volume
curl -X POST http://192.168.4.1/api/config \
  -H "Content-Type: application/json" \
  -d '{"volume":75}'
```

## Integration Examples

### Home Assistant
```yaml
# configuration.yaml
rest_command:
  floppy_play_random:
    url: "http://192.168.4.1/api/play-random"
    method: POST
  
  floppy_stop:
    url: "http://192.168.4.1/api/stop"
    method: POST

sensor:
  - platform: rest
    resource: http://192.168.4.1/api/status
    name: "Floppy Simulator Status"
    value_template: "{{ value_json.data.playing }}"
```

### Node-RED
```json
[
  {
    "id": "floppy_trigger",
    "type": "http request",
    "method": "POST",
    "url": "http://192.168.4.1/api/play-random",
    "name": "Trigger Floppy Sound"
  }
]
```

---

*This API documentation is part of the Floppy Disk Audio Simulator project. For more information, see README.md*