#include <Servo.h>

Servo myservo;  // create servo object to control a servo

int servoPin = 0;
int angle;

void setup()
{
  myservo.attach(9);
}

void loop()
{
  setServoAngle(0);
  delay(3000);
  setServoAngle(90);
  delay(3000);
  setServoAngle(180);
  delay(3000);
}

void setServoAngle(int angle)
{
  const int SERVO_LOW = 36;
  const int SERVO_HIGH = 142;

  angle = servoMap(angle, 0, 180, SERVO_LOW, SERVO_HIGH);     // scale it to use it with the servo (value between 0 and 180)
  myservo.write(angle);                  // sets the servo position according to the scaled value
}

float servoMap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
