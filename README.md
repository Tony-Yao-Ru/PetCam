# 📷 Raspberry Pi MJPEG WebCam Streamer

This project turns your Raspberry Pi into a live MJPEG video streaming server using a standard USB webcam. It also provides a simple web interface to view the stream and capture snapshots directly in the browser.

## 🔧 Features
- Real-time MJPEG stream from a USB webcam.
- Simple, responsive HTML interface.
- One-click image capture and download.
- Built with C (server), HTML/JS (frontend), and V4L2 (camera input).

## 🧰 Requirements
- Raspberry Pi (tested on Raspberry Pi 4 with Ubuntu Desktop 24.04.2 LTS)
- USB webcam compatible with V4L2
- gcc, libv4l-dev

## 🚀 Getting Started

### 1. Install Dependencies on Rawspberry Pi
```bash!
sudo apt update
sudo apt install libv4l-dev
```

### 2. Check Webcam
```bash!
v4l2-ctl --list-devices
```

### 3. Compile the Server
```bash!
gcc server.c -o webcam-server -lpthread
```

### 4. Run the Server
```bash!
./webcam-server
```

### 5. Check the web
Visit ***http://<YOUR_PI_IP>:8080/***
