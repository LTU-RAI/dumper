#include <Servo.h>

// ********************* Pin assignments ********************* //
const int STEERING_PIN = 9;
const int MOTOR_PIN    = 10;
const int DUMP_PIN = 11;  // Connect your dump servo here

// Limit switches
const int UP_SWITCH_PIN   = 3; // D3
const int DOWN_SWITCH_PIN = 4; // D4

// Servo objects
Servo steering, motor, dump;

// ********************* Limits ********************* //
// ESC (Electronic Speed Controller) Limits
const int ESC_MIN     = 1000;  // Full reverse
const int ESC_NEUTRAL = 1500;  // Neutral
const int ESC_MAX     = 2000;  // Full forward

// Steering limits
const int STEER_MIN   = 45;    // Full left
const int STEER_NEUT  = 90;    // Center
const int STEER_MAX   = 135;   // Full right

// Safe PWM values for continuous motor
const int DUMP_STOP  = 1500; // neutral
const int DUMP_UP    = 1200; // moves up slowly
const int DUMP_DOWN  = 1800; // moves down slowly

// Dumper states
enum DumperState {DUMPER_STOP, DUMPER_UP, DUMPER_DOWN};
DumperState dumperState = DUMPER_STOP;

// Optional: Watchdog timeout for failsafe
unsigned long lastCommandTime = 0;
const unsigned long TIMEOUT_MS = 2000; // 2 seconds

void setup() {
  Serial.begin(9600);  // Match ROS subscriber baud
  steering.attach(STEERING_PIN);
  motor.attach(MOTOR_PIN);
  dump.attach(DUMP_PIN);

  stopAndCenter();

  pinMode(UP_SWITCH_PIN, INPUT);
  pinMode(DOWN_SWITCH_PIN, INPUT);
  
  Serial.println("✅ Arduino ready and waiting for PWM commands...");
}

void loop() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim(); // remove whitespace

    // Expecting format: "S:90,T:100"
    int sIndex = line.indexOf("S:");
    int tIndex = line.indexOf("T:");
    int dIndex = line.indexOf("D:");

    if (sIndex != -1 && tIndex != -1 && dIndex!=-1) {
      int steeringVal = line.substring(sIndex + 2, line.indexOf(',', sIndex)).toInt();
      int throttleVal = line.substring(tIndex + 2, line.indexOf(',', tIndex)).toInt();
      int dumperVal   = line.substring(dIndex + 2).toInt();

      // Clamp values just in case
      steeringVal = constrain(steeringVal, STEER_MIN, STEER_MAX);
      throttleVal = constrain(throttleVal, 70, 110); // matches subscriber mapping
      
      // Map throttle from 70–110 to ESC microseconds (1000–2000)
      int escPWM = map(throttleVal, 70, 110, ESC_MIN, ESC_MAX);

      // Send commands to hardware
      steering.write(steeringVal);
      motor.writeMicroseconds(escPWM);
            
      // UP
      if (dumperVal == 1.0) {
        while (digitalRead(UP_SWITCH_PIN) == 0){
          dump.writeMicroseconds(DUMP_UP);
          delay(20);
        } 
          dump.writeMicroseconds(DUMP_STOP);
          Serial.println("Reached top limit, stopped.");
          return;
      }
    
      // DOWN
      else if (dumperVal == -1.0 ) {
        while (digitalRead(DOWN_SWITCH_PIN) == 0) {
          dump.writeMicroseconds(DUMP_DOWN);
          delay(20);
        } 
          dump.writeMicroseconds(DUMP_STOP);
          Serial.println("Reached bottom limit, stopped.");
          return;
      }
    
      // STOP
      else if (dumperVal == 0.0 ){
        dump.writeMicroseconds(DUMP_STOP);
      }

      lastCommandTime = millis();

      // Debug print
      Serial.print("Steering PWM: "); Serial.print(steeringVal);
      Serial.print(" | Throttle PWM: "); Serial.println(escPWM);
      Serial.print(" | Dumper cmd: "); Serial.println(dumperVal);
    }
  }

  // Failsafe: Stop motor if no command received for a while
  if (millis() - lastCommandTime > TIMEOUT_MS) {
    stopAndCenter();
  }

  
}

// ********************* Helper Functions ********************* //
void stopAndCenter() {
  steering.write(STEER_NEUT);
  motor.writeMicroseconds(ESC_NEUTRAL);
  dump.writeMicroseconds(DUMP_STOP);
}
