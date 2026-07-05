#include <Servo.h>
Servo servo;

// ─── Pins ──────────────────────────────────────────
#define ENA        6
#define IN1        2
#define IN2        9
#define IN3        4
#define IN4        5
#define ENB        3
#define TRIG       10
#define ECHO       11
#define SERVO_PIN  8

// ─── Tuning ────────────────────────────────────────
#define RIGHT_SPEED     175
#define LEFT_SPEED      175
#define TURN_SPEED      155
#define OBSTACLE_DIST   28     // detection threshold (cm)
#define TURN_TIME       500    // ms per 90 degree turn
#define BACKUP_TIME     550    // ms backup
#define SCAN_SETTLE     650    // ms servo settle

// ─── State Machine ─────────────────────────────────
enum State {
  FORWARD,
  AVOID
};
State state = FORWARD;

// ─── Tracking ──────────────────────────────────────
int  lastTurnDir   = 0;   // 1=left 2=right
int  sameObstCount = 0;   // same obstacle hit counter
int  avoidAttempts = 0;   // total avoid attempts

// ───────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  pinMode(ENA,  OUTPUT); pinMode(IN1, OUTPUT);
  pinMode(IN2,  OUTPUT); pinMode(IN3, OUTPUT);
  pinMode(IN4,  OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);

  motorStop();
  servo.attach(SERVO_PIN);
  servo.write(90);
  delay(800);

  // Warm up sensor
  for (int i = 0; i < 5; i++) { getDistance(); delay(100); }
  Serial.println("=== Robot Ready ===");
}

// ───────────────────────────────────────────────────
void loop() {
  switch (state) {

    case FORWARD: {
      int dist = getDistance();
      Serial.print("[FWD] "); Serial.println(dist);

      if (dist > 0 && dist < OBSTACLE_DIST) {
        motorStop();
        delay(150);
        sameObstCount = 0;
        avoidAttempts = 0;
        state = AVOID;
      } else {
        motorForward();
        delay(50);
      }
      break;
    }

    case AVOID: {
      avoidAttempts++;
      Serial.print("\n[AVOID] attempt #"); Serial.println(avoidAttempts);

      // ── Step 1: Backup first, always ─────────────
      // Moving back creates physical space so
      // scans reflect the TRUE open space around robot
      Serial.println("[BACKUP]");
      motorBackward();
      delay(BACKUP_TIME + (sameObstCount * 150)); // back up MORE each retry
      motorStop();
      delay(250);

      // ── Step 2: Scan all 3 directions ────────────
      int f = scanDirection(90);
      int l = scanDirection(155);
      int r = scanDirection(25);
      servo.write(90);
      delay(300);

      Serial.print("  F="); Serial.print(f);
      Serial.print("  L="); Serial.print(l);
      Serial.print("  R="); Serial.println(r);

      // ── Step 3: All blocked → U-turn ─────────────
      if (f < OBSTACLE_DIST &&
          l < OBSTACLE_DIST &&
          r < OBSTACLE_DIST) {

        sameObstCount++;
        Serial.print("[ALL BLOCKED] count="); Serial.println(sameObstCount);

        if (sameObstCount >= 3) {
          // Fully escape: back up a lot then U-turn
          Serial.println("[ESCAPE] Full U-turn");
          motorBackward();
          delay(800);
          motorStop();
          delay(200);
          motorTurnRight();
          delay(TURN_TIME * 2);   // 180 degrees
          motorStop();
          delay(300);
          sameObstCount = 0;
          lastTurnDir   = 0;
          avoidAttempts = 0;
        }
        // Stay in AVOID to retry with more backup
        break;
      }

      // ── Step 4: Pick direction smartly ───────────
      // Score each direction
      int scoreL = l;
      int scoreR = r;

      // Penalize last turn direction to break loops
      if (lastTurnDir == 1) scoreL = scoreL * 0.6;  // penalize left
      if (lastTurnDir == 2) scoreR = scoreR * 0.6;  // penalize right

      // If front is clearly open, just go forward
      if (f > OBSTACLE_DIST * 2) {
        Serial.println("[DECISION] Front wide open, go forward");
        sameObstCount = 0;
        lastTurnDir   = 0;
        avoidAttempts = 0;
        state = FORWARD;
        break;
      }

      // Pick highest scored direction
      bool goLeft;
      if (scoreL >= scoreR && l > OBSTACLE_DIST) {
        goLeft = true;
      } else if (r > OBSTACLE_DIST) {
        goLeft = false;
      } else {
        // Only front semi-clear
        Serial.println("[DECISION] Only front, nudge forward");
        motorForward();
        delay(300);
        motorStop();
        state = FORWARD;
        break;
      }

      // ── Step 5: Execute turn ─────────────────────
      if (goLeft) {
        Serial.println("[TURN] Left");
        motorTurnLeft();
        lastTurnDir = 1;
      } else {
        Serial.println("[TURN] Right");
        motorTurnRight();
        lastTurnDir = 2;
      }
      delay(TURN_TIME);
      motorStop();
      delay(250);

      // ── Step 6: Move forward a bit after turn ─────
      // KEY FIX: drive forward briefly so robot physically
      // clears the obstacle before next scan
      Serial.println("[CLEAR] Moving past obstacle...");
      motorForward();
      delay(350);
      motorStop();
      delay(200);

      // ── Step 7: Verify new path ───────────────────
      int checkDist = getDistance();
      Serial.print("[VERIFY] dist="); Serial.println(checkDist);

      if (checkDist > OBSTACLE_DIST) {
        Serial.println("[OK] Path clear, resuming forward");
        sameObstCount = 0;
        avoidAttempts = 0;
        lastTurnDir   = 0;
        state = FORWARD;
      } else {
        // Still blocked — increment and retry
        sameObstCount++;
        Serial.print("[RETRY] same obstacle count=");
        Serial.println(sameObstCount);
        // Stay in AVOID, will backup more next iteration
      }
      break;
    }
  }
}

// ─── Scan Direction ────────────────────────────────
int scanDirection(int angle) {
  servo.write(angle);
  delay(SCAN_SETTLE);
  return getDistance();
}

// ─── Median of 5 Ultrasonic Readings ──────────────
long getDistance() {
  int r[5];
  for (int i = 0; i < 5; i++) {
    digitalWrite(TRIG, LOW);  delayMicroseconds(2);
    digitalWrite(TRIG, HIGH); delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    long d = pulseIn(ECHO, HIGH, 25000);
    r[i] = (d == 0) ? 300 : d * 0.034 / 2;
    delay(15);
  }
  // Sort → median
  for (int i = 0; i < 4; i++)
    for (int j = i+1; j < 5; j++)
      if (r[i] > r[j]) { int t=r[i]; r[i]=r[j]; r[j]=t; }
  return r[2];
}

// ─── Motor Functions ───────────────────────────────
void motorForward() {
  analogWrite(ENA, RIGHT_SPEED); analogWrite(ENB, LEFT_SPEED);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}
void motorBackward() {
  analogWrite(ENA, RIGHT_SPEED); analogWrite(ENB, LEFT_SPEED);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}
void motorTurnLeft() {
  analogWrite(ENA, TURN_SPEED); analogWrite(ENB, TURN_SPEED);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}
void motorTurnRight() {
  analogWrite(ENA, TURN_SPEED); analogWrite(ENB, TURN_SPEED);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}
void motorStop() {
  analogWrite(ENA, 0); analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}