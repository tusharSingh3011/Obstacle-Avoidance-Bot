# 🤖 Arduino Obstacle Avoiding Robot

An autonomous obstacle-avoiding robot built using **Arduino UNO**, **L298N Motor Driver**, **HC-SR04 Ultrasonic Sensor**, **Servo Motor**, and **BO Gear Motors**.

The robot continuously scans its surroundings, detects obstacles, chooses the best path, and navigates automatically without human intervention.

---

# ✨ Features

- 🚗 Autonomous navigation
- 📡 Ultrasonic obstacle detection
- 🔄 Servo-based left and right scanning
- ↩ Automatic backup before turning
- 🧭 Chooses the clearest path
- 🔁 Returns to previous location if trapped
- 🔄 Performs 180° U-turn when no path is available
- ⚙ Adjustable speed and distance parameters
- 🧠 Finite State Machine (FSM) based programming

---

# 🛠 Components Required

| Component | Quantity |
|------------|----------|
| Arduino UNO | 1 |
| L298N Motor Driver | 1 |
| HC-SR04 Ultrasonic Sensor | 1 |
| SG90 Servo Motor | 1 |
| BO Gear Motors | 2 |
| Robot Chassis | 1 |
| Wheels | 2 |
| Caster Wheel | 1 |
| Li-ion Battery Pack (7.4V–12V) | 1 |
| Power Switch | 1 |
| Jumper Wires | As Required |

---

# 🔌 Circuit Connections

## Ultrasonic Sensor

| HC-SR04 | Arduino UNO |
|----------|-------------|
| VCC | 5V |
| GND | GND |
| TRIG | D10 |
| ECHO | D11 |

---

## Servo Motor

| Servo | Arduino UNO |
|--------|-------------|
| Signal | D8 |
| VCC | 5V |
| GND | GND |

---

## L298N Motor Driver

| L298N | Arduino UNO |
|--------|-------------|
| ENA | D6 |
| IN1 | D2 |
| IN2 | D9 |
| IN3 | D4 |
| IN4 | D5 |
| ENB | D3 |

---

## Power Connections

- Battery Positive → L298N +12V
- Battery Negative → L298N GND
- Arduino GND ↔ L298N GND (Common Ground)

---

# ⚙ Pin Configuration

```cpp
ENA        -> D6
IN1        -> D2
IN2        -> D9
IN3        -> D4
IN4        -> D5
ENB        -> D3

Servo      -> D8

TRIG       -> D10
ECHO       -> D11
```

---

# 🧠 Robot Working

## 1. Move Forward

The robot continuously moves forward while monitoring the distance ahead using the HC-SR04 ultrasonic sensor.

---

## 2. Detect Obstacle

If an object is detected within the predefined obstacle distance, the robot immediately stops.

---

## 3. Move Backward

The robot reverses a short distance to create space for scanning.

---

## 4. Scan Environment

The servo rotates the ultrasonic sensor:

- Left (155°)
- Right (25°)

Distance measurements are taken in both directions.

---

## 5. Select Best Direction

The robot compares both distances.

- If left is clearer → Turn Left
- If right is clearer → Turn Right

---

## 6. Verify Path

After turning, the robot checks whether the path is actually clear.

If not:

- Undo the previous turn
- Try the opposite direction

---

## 7. Return Home

If both directions are blocked, the robot returns to its previous position.

---

## 8. Perform U-Turn

After returning, the robot performs a 180° turn and continues searching for a new path.

---

# 🔄 Finite State Machine

```text
FORWARD
   │
Obstacle Detected
   │
BACKUP
   │
SCAN
   │
TURN
   │
VERIFY
   │
 ┌───────────────┐
 │               │
Clear         Blocked
 │               │
FORWARD     Opposite Turn
                 │
          Still Blocked
                 │
          RETURN_HOME
                 │
              UTURN
                 │
             FORWARD
```

---

# ⚙ Adjustable Parameters

```cpp
RIGHT_SPEED
LEFT_SPEED
TURN_SPEED

OBSTACLE_DIST
SAFE_DIST

TURN_90
TURN_180

BACKUP_MS
FWD_STEP_MS
SCAN_SETTLE
```

These values can be modified according to your robot's speed and environment.

---

# 📂 Project Structure

```text
Obstacle-Avoiding-Robot/
│
├── Arduino_Code/
│   └── ObstacleAvoidingRobot.ino
│
├── images/
│   ├── wiring_diagram.png
│   └── robot.jpg
│
├── README.md
```

---

# 🚀 Uploading the Code

1. Install the Arduino IDE.
2. Install the **Servo** library (included with the Arduino IDE).
3. Connect the Arduino UNO to your computer.
4. In the Arduino IDE, select:
   - **Board → Arduino UNO**
   - **Port → Correct COM Port**
5. Open the project `.ino` file and click **Upload**.

---

---

# 👨‍💻 Author

**Your Name**

GitHub: https://github.com/tusharSingh3011

---

## ⭐ Support

If you found this project helpful, don't forget to **⭐ Star** this repository!
