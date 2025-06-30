# 🤖📷 Raspberry Pi MJPEG WebCam Streamer + Motor Controller

This project turns your Raspberry Pi into a live **MJPEG video streaming server** with web-based **robot movement control**.  
It streams camera feed using V4L2 and lets you control **two L298N motor modules** directly from a browser.

---

## 🔧 Features

- 🎥 Real-time MJPEG stream from a USB webcam.
- 🕹️ Robot control via browser:
  - Move Forward
  - Move Backward
  - Turn Left
  - Turn Right
- 📸 One-click snapshot download.
- 📦 Simple, responsive HTML interface.
- 💡 Written in C using `libv4l2`, `libgpiod`, and raw socket programming.

---

## 🧰 Requirements

- **Hardware**
  - Raspberry Pi (tested on Raspberry Pi 4)
  - USB webcam (V4L2-compatible)
  - 2 × L298N motor driver modules
  - Jumper wires
  - 6–12V external power for motors
  - Shared GND between Pi and motor modules

- **Software**
  - Ubuntu Desktop 24.04.2 LTS (or similar)
  - `gcc`, `libv4l-dev`, `libgpiod-dev`

---

## 🔌 GPIO Wiring

### 🟢 Front Motor Module (Steering)
| L298N Pin | Raspberry Pi GPIO | Physical Pin |
|-----------|-------------------|---------------|
| IN1       | GPIO 17           | Pin 11        |
| IN2       | GPIO 22           | Pin 15        |
| IN3       | GPIO 23           | Pin 16        |
| IN4       | GPIO 24           | Pin 18        |

### 🔵 Back Motor Module (Drive)
| L298N Pin | Raspberry Pi GPIO | Physical Pin |
|-----------|-------------------|---------------|
| IN1       | GPIO 5            | Pin 29        |
| IN2       | GPIO 6            | Pin 31        |
| IN3       | GPIO 13           | Pin 33        |
| IN4       | GPIO 19           | Pin 35        |

> ⚠️ **Power the motors separately**, not through the Pi’s 5V. Make sure the Pi and the motor modules share **GND**.

---

## 🚀 Getting Started

### 1. Install Dependencies
```bash
sudo apt update
sudo apt install gcc libv4l-dev libgpiod-dev
```

### 2. Check Webcam
```bash!
v4l2-ctl --list-devices
```

### 3. Compile the Server
```bash!
gcc server.c motor_control.c -o webcam-server -lpthread
```

### 4. Run the Server
```bash!
./webcam-server
```

### 5. Check the web
Visit ***http://<YOUR_PI_IP>:8080/***
