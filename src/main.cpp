#include <Arduino.h>

/*==========GENERAL INFO==========
MCU used - Arduino Nano
I2C pins RESERVED - A4 SDA, A5 SCL
Motor driver used - Olimex BB-VNH3SP30
|---------------------------------------------------------------------------|
|        The hardware connections to BOTH drivers are as follows:           |
|--------------------------------|------------------------------------------|
|  Connector at Olimexino-328    |      Connector at BB-VNH3SP30            |
|--------------------------------|------------------------------------------|
|    Not connected!              |          CTRL<1>,  VIN                   |
|    Power<3>, 5V         	     |          CTRL<2>,  +5V                   |
|    Power<4>, GND               |          CTRL<3>,  GND                   |
|    Digital<2>                  |          CTRL<4>,  INA                   | motor direction switch a
|    Digital<4>                  |          CTRL<5>,  INB                   | motor direction switch b
|    Digital<3>                  |          CTRL<6>,  PWM                   | motor speed pin
|    Digital<12>                 |          CTRL<7>,  ENA/DIAGA             | MUST BE SET TO INPUT
|    Digital<13>                 |          CTRL<8>,  ENB/DIAGB             | MUST BE SET TO INPUT
|--------------------------------|------------------------------------------|
RC used - Flysky FS-GT3B

*/

//========================================FUNCTION DECLARATIONS========================================



/*==========PINOUT==========
|D13/SCK - diagb/INPUT, grey    |D12/MISO     - diaga/INPUT, pink
|3V3     - unused               |D11/PWM/MOSI - unused
|AREF    - unused               |D10/PWM      - unused
|A0      - unused               |D9/PWM       - unused
|A1      - unused               |D8           - rc_button_pin/INPUT, ch3, grey
|A2      - unused               |D7           - rc_throttle_pin/INPUT, ch2, pink
|A3      - unused               |D6           - rc_steering_pin/INPUT, ch1, purple
|A4/SDA  - RESERVED             |D5/PWM       - unused
|A5/SCL  - RESERVED             |D4           - inb/OUTPUT, for switching motor direction, blue
|A6      - unused               |D3/PWM       - motor_speed_pin/OUTPUT, purple
|A7      - unused               |D2           - ina/OUTPUT, for switching motor direction, yellow
|5V      - 5V to bus            |GND          - GND to bus
|RST     - unused               |RST          - unused
|GND     - GND to bus           |RX           - unused
|VIN     - unused               |TX           - unused
*/


//==========RC INPUTS==========
int rc_steering_pin = 6; //channel 1, PWM "analog" input
int rc_throttle_pin = 7; //channel 2, PWM "analog" input
int rc_button_pin = 8;   //channel 3, PWM "analog" input, toggle action, state is RESET when the RC TX is turned off

//==========MOTOR DRIVER OUTPUTS==========
int diaga = 12; //diagnostic from motor driver
int diagb = 13; //diagnostic from motor driver
int motor_speed_pin = 3; //writes PWM value for motor speed control
int ina = 2; //together with inb controls motor direction
int inb = 4; //together with ina controls motor direction

//==========VARIABLES AND FLAGS==========
int rc_steering_inval = 0;
int rc_throttle_inval = 0;
int rc_button_inval = 0;

int rc_steering_mapval = 0;
int rc_throttle_mapval = 0;

bool rc_button_state = 0;
bool motors_forward = false;
bool motors_reverse = false;
bool motors_stop = false;

//========================================SETUP========================================
void setup() 
{
Serial.begin(115200);

pinMode(rc_steering_pin, INPUT);
pinMode(rc_throttle_pin, INPUT);
pinMode(rc_button_pin, INPUT);

pinMode(diaga, INPUT);
pinMode(diagb, INPUT);
pinMode(motor_speed_pin, OUTPUT);
pinMode(ina, OUTPUT);
pinMode(inb, OUTPUT);

motors_stop = true;

}
//========================================LOOP========================================
void loop() 
{

rc_steering_inval = pulseIn(rc_steering_pin, HIGH); //constrained to 1020-1420 if <= 1420, 1520-1920 if >=1520, else 1470 mid-point (deadzone +- 50)
rc_throttle_inval = pulseIn(rc_throttle_pin, HIGH); //constrained to 1020-1420 if <= 1420, 1520-1920 if >=1520, else 1470 mid-point (deadzone +- 50)
rc_button_inval = pulseIn(rc_button_pin, HIGH);     //no need for constraint or deadzone, read below 1500 or above 1501

//==========CONSTRAIN INPUTS, SET DEADZONES, SET MOTOR DIRECTION FLAGS, REMAP VALUES FOR PWM OUTPUT==========
    if (rc_steering_inval <= 1420) 
    {
        rc_steering_inval = constrain(rc_steering_inval, 1020, 1420);
        rc_steering_mapval = map(rc_steering_inval, 1420, 1020, 89, 0);
    }
    else if (rc_steering_inval >= 1520) 
    {
        rc_steering_inval = constrain(rc_steering_inval, 1520, 1920);
        rc_steering_mapval = map(rc_steering_inval, 1520, 1920, 91, 180);
    }
    
    else 
    {
        rc_steering_inval = 1470;
        rc_steering_mapval = 90;
    }


    if (rc_throttle_inval <= 1420) 
    {
        rc_throttle_inval = constrain(rc_throttle_inval, 1020, 1420);
        rc_throttle_mapval = map(rc_throttle_inval, 1420, 1020, 0, 255);
        motors_forward = false;
        motors_reverse = true;
        motors_stop = false;
    }
    else if (rc_throttle_inval >= 1520) 
    {
        rc_throttle_inval = constrain(rc_throttle_inval, 1520, 1920);
        rc_throttle_mapval = map(rc_throttle_inval, 1520, 1920, 0, 255);
        motors_forward = true;
        motors_reverse = false;
        motors_stop = false;
    }
    else 
    {
        rc_throttle_inval = 1470;
        rc_throttle_mapval = 0;
        motors_forward = false;
        motors_reverse = false;
        motors_stop = true;
    }

//========================================TEST CODE START========================================
if (motors_forward == true)
    {
        digitalWrite(ina, HIGH);
        digitalWrite(inb, LOW);
        analogWrite(motor_speed_pin, rc_throttle_mapval);
    }
else if (motors_reverse == true)
    {
        digitalWrite(ina, LOW);
        digitalWrite(inb, HIGH);
        analogWrite(motor_speed_pin, rc_throttle_mapval);
    }
else
    {
        digitalWrite(ina, LOW);
        digitalWrite(inb, LOW);
    }
/*
So I'm adding these couple of lines
here to check the git commits and all that jazz
digitalWrite(ina, HIGH);
digitalWrite(inb, LOW);
rc_throttle_mapval = map(rc_throttle_inval, 1420, 1920, 0, 255);
rc_throttle_mapval = constrain(rc_throttle_mapval, 0, 255);
analogWrite(motor_speed_pin, rc_throttle_mapval);
*/
//========================================TEST CODE END========================================

//serial debugging
Serial.print("rc_steering_inval = ");
Serial.print(rc_steering_inval);
Serial.print(" rc_steering_mapval = ");
Serial.print(rc_steering_mapval);
Serial.print(" rc_throttle_inval = ");
Serial.print(rc_throttle_inval);
Serial.print(" rc_throttle_mapval = ");
Serial.print(rc_throttle_mapval);
Serial.print(" rc_button_inval = ");
Serial.print(rc_button_inval);
Serial.print(" motor fwd: ");
Serial.print(motors_forward);
Serial.print(" motor rev: ");
Serial.print(motors_reverse);\
Serial.print(" motor stop: ");
Serial.println(motors_stop);

}
//========================================FUNCTION DEFINITIONS======================================== 