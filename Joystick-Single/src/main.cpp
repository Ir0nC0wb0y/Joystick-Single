#include <Arduino.h>
#include <Joystick.h>

// Pin Declarations
#define PIN_JOYSTICK_AXIS_X A0
#define PIN_JOYSTICK_AXIS_Y A1
#define PIN_JOYSTICK_AXIS_Z A2
#define PIN_JOYSTICK_BUTTON 21
#define PIN_BUTTON_1         2
#define PIN_BUTTON_2         3
#define PIN_BUTTON_3         4
#define PIN_BUTTON_4         5
#define PIN_BUTTON_5         6
#define PIN_BUTTON_6         7
#define PIN_MODE            A9

// Joystick State Variables
int  joy_axis_x = 0;
int  joy_axis_y = 0;
int  joy_axis_z = 0;
bool joy_button = 0;
bool button_1   = 0;
bool button_2   = 0;
bool button_3   = 0;
bool button_4   = 0;
bool button_5   = 0;
bool button_6   = 0;
int  mode_joy   = 0;

// Mode Data
  int mode = -1;
  #define MODE_HIGH 895
  #define MODE_LOW  128
  // Modes
    // 1 (II)     - Steering/Accel/Brake
    // 2 (Center) - X, Y, Z Axes
    // 3 (I)      - Rx, Ry, Rz Axes

// Axis Calibration
int center_x = 0;
int center_y = 0;
int center_z = 0;
int calibration_times = 256;
#define DEADZONE 30 // +/- tolerance from center
#define JOYSTICK_AXIS_MAX  400
#define JOYSTICK_AXIS_MIN -400
#define DIRECTION_X  1
#define DIRECTION_Y -1
#define DIRECTION_Z  1

// Joystick Lib Variables
Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID, // Report ID
  JOYSTICK_TYPE_MULTI_AXIS,   // Joystick Type
  7,                          // Button Count (1-6 + Joystick)
  0,                          // Hatswitch Count
  true,                       // Include X-Axis
  true,                       // Include Y-Axis 
  true,                       // Include Z-Axis
  true,                       // Include Rx-Axis
  true,                       // Include Ry-Axis
  true,                       // Include Rz-Axis
  false,                      // Include Rudder
  false,                      // Include Throttle
  true,                       // Include Accelerator
  true,                       // Include Brake
  true);                      // Include Steering



int Joystick_Read(int read_pin, int center_adj, int direction) {
  // Collect Analog
  int raw = analogRead(read_pin);

  // Check if within deadzone
  int out = 0;
  if (abs(raw-center_adj) > DEADZONE) {
    // Compute Range
    if (raw > center_adj) {
      out = map(raw, center_adj + DEADZONE, 1023, 0, JOYSTICK_AXIS_MAX);
      //Serial.println("Above Deadzone");
    } else {
      out = map(raw, 0, center_adj - DEADZONE, JOYSTICK_AXIS_MIN, 0);
      //Serial.println("Below Deadzone");
    }
  }
  out = out * direction;

  return out;
}

void ComputeMode() {
  // Read Mode Switch
  mode_joy = analogRead(PIN_MODE);

  // Set Mode state
  if (mode_joy < MODE_LOW) {
    mode = 3;
  } else if (mode_joy > MODE_HIGH) {
    mode = 1;
  } else {
    mode = 2;
  }

  
  switch (mode) {
    case 1:
      // set Steering to X axis
      Joystick.setSteering(joy_axis_x);
      if (joy_axis_y > 0) {
        // if Y > 0 set Throttle
        Joystick.setAccelerator(joy_axis_y);
        Joystick.setBrake(0);
      } else if (joy_axis_y < 0) {
        // if Y < 0 set Brake
        Joystick.setAccelerator(0);
        Joystick.setBrake(abs(joy_axis_y));
      } else {
        Joystick.setAccelerator(0);
        Joystick.setBrake(0);
      }
      
      
      // Set X, Y, Z to zero
      Joystick.setXAxis(0);
      Joystick.setYAxis(0);
      Joystick.setZAxis(0);
      // set Rx, Ry, Rz to zero
      Joystick.setRxAxis(0);
      Joystick.setRyAxis(0);
      Joystick.setRzAxis(0);
      break;

    case 2:
      // Set X, Y, Z
      Joystick.setXAxis(joy_axis_x);
      Joystick.setYAxis(joy_axis_y);
      Joystick.setZAxis(joy_axis_z);
      // set Steering, Throttle, Brake to zero
      Joystick.setSteering(0);
      Joystick.setThrottle(0);
      Joystick.setBrake(0);
      // set Rx, Ry, Rz to zero
      Joystick.setRxAxis(0);
      Joystick.setRyAxis(0);
      Joystick.setRzAxis(0);
      break;

    case 3:
      // set Rx, Ry, Rz
      Joystick.setRxAxis(joy_axis_x);
      Joystick.setRyAxis(joy_axis_y);
      Joystick.setRzAxis(joy_axis_z);
      // set Steering, Throttle, Brake to zero
      Joystick.setSteering(0);
      Joystick.setThrottle(0);
      Joystick.setBrake(0);
      // Set X, Y, Z to zero
      Joystick.setXAxis(0);
      Joystick.setYAxis(0);
      Joystick.setZAxis(0);
      break;
    
    default:
      break;
  }

}

void joystick_Calibrate() {
  unsigned long perf_cal = micros();
  
  int cal_x = analogRead(PIN_JOYSTICK_AXIS_X);
  int cal_y = analogRead(PIN_JOYSTICK_AXIS_Y);
  int cal_z = analogRead(PIN_JOYSTICK_AXIS_Z);
  
  int i = 1;
  while (i < calibration_times) {
    cal_x = (analogRead(PIN_JOYSTICK_AXIS_X) + cal_x) / 2;
    cal_y = (analogRead(PIN_JOYSTICK_AXIS_Y) + cal_x) / 2;
    cal_z = (analogRead(PIN_JOYSTICK_AXIS_Z) + cal_x) / 2;
    i++;
  }
  Serial.print(cal_x); Serial.print(", ");
  Serial.print(cal_y); Serial.print(", ");
  Serial.print(cal_z); Serial.print(", ");
  Serial.print(i); Serial.print(", ");
  Serial.println();
  
  center_x = cal_x;
  center_y = cal_y;
  center_z = cal_z;

  perf_cal = micros() - perf_cal;

  /*
  Serial.print("New joystick calibration: ");
    Serial.print("X="); Serial.print(center_x);
    Serial.print(", Y="); Serial.print(center_y);
    Serial.print(", Z="); Serial.print(center_z);
    Serial.print(" and took "); Serial.print((double)perf_cal / 1000000.0); Serial.print("s");
    Serial.println();
  */
}

void setup() {
  //Serial.begin(9600);
  delay(5000);
  //Serial.println("Starting Test Script");

  // Setup Pins
  pinMode(PIN_BUTTON_1,INPUT_PULLUP);
  pinMode(PIN_BUTTON_2,INPUT_PULLUP);
  pinMode(PIN_BUTTON_3,INPUT_PULLUP);
  pinMode(PIN_BUTTON_4,INPUT_PULLUP);
  pinMode(PIN_BUTTON_5,INPUT_PULLUP);
  pinMode(PIN_BUTTON_6,INPUT_PULLUP);
  pinMode(PIN_JOYSTICK_BUTTON,INPUT_PULLUP);

  // Calibrate Joystick ADC reads
  joystick_Calibrate();
  //delay(5000);

  // Setup Joystick
  Joystick.setXAxisRange(JOYSTICK_AXIS_MIN,JOYSTICK_AXIS_MAX);
  Joystick.setYAxisRange(JOYSTICK_AXIS_MIN,JOYSTICK_AXIS_MAX);
  Joystick.setZAxisRange(JOYSTICK_AXIS_MIN,JOYSTICK_AXIS_MAX);
  Joystick.setRxAxisRange(JOYSTICK_AXIS_MIN,JOYSTICK_AXIS_MAX);
  Joystick.setRyAxisRange(JOYSTICK_AXIS_MIN,JOYSTICK_AXIS_MAX);
  Joystick.setRzAxisRange(JOYSTICK_AXIS_MIN,JOYSTICK_AXIS_MAX);
  Joystick.setSteeringRange(JOYSTICK_AXIS_MIN,JOYSTICK_AXIS_MAX);
  Joystick.setAcceleratorRange(0, JOYSTICK_AXIS_MAX);
  Joystick.setBrakeRange(0,-JOYSTICK_AXIS_MIN);

  Joystick.begin(false);
  
}

void loop() {
  //unsigned long perf_read = micros();
  // Read Momentary Buttons
  button_1 = !digitalRead(PIN_BUTTON_1);
  Joystick.setButton(1,button_1);
  button_2 = !digitalRead(PIN_BUTTON_2);
  Joystick.setButton(2,button_2);
  button_3 = !digitalRead(PIN_BUTTON_3);
  Joystick.setButton(3,button_3);
  button_4 = !digitalRead(PIN_BUTTON_4);
  Joystick.setButton(4,button_4);
  button_5 = !digitalRead(PIN_BUTTON_5);
  Joystick.setButton(5,button_5);
  button_6 = !digitalRead(PIN_BUTTON_6);
  Joystick.setButton(6,button_6);
  joy_button = !digitalRead(PIN_JOYSTICK_BUTTON);
  Joystick.setButton(0,joy_button);

  // Read Joystick
  joy_axis_x = Joystick_Read(PIN_JOYSTICK_AXIS_X, center_x, DIRECTION_X);
  joy_axis_y = Joystick_Read(PIN_JOYSTICK_AXIS_Y, center_y, DIRECTION_Y);
  joy_axis_z = Joystick_Read(PIN_JOYSTICK_AXIS_Z, center_z, DIRECTION_Z);
  ComputeMode();

  Joystick.sendState();

  //perf_read = micros() - perf_read;

  /*
  // Display values
  Serial.print(button_1); //Serial.print(", ");
  Serial.print(button_2); //Serial.print(", ");
  Serial.print(button_3); //Serial.print(", ");
  Serial.print(button_4); //Serial.print(", ");
  Serial.print(button_5); //Serial.print(", ");
  Serial.print(button_6); Serial.print(", ");
  Serial.print(joy_axis_x); Serial.print(", ");
  Serial.print(joy_axis_y); Serial.print(", ");
  Serial.print(joy_axis_z); Serial.print(", ");
  Serial.print(joy_button); Serial.print(", ");
  Serial.print(mode); Serial.print(", ");
  Serial.print(perf_read);
  Serial.println();
  */

}
