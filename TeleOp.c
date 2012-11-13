#pragma config(Hubs,  S1, HTMotor,  HTMotor,  HTServo,  HTMotor)
#pragma config(Sensor, S1,     ,               sensorI2CMuxController)
#pragma config(Motor,  motorB,           ,             tmotorNXT, openLoop)
#pragma config(Motor,  motorC,           ,             tmotorNXT, openLoop)
#pragma config(Motor,  mtr_S1_C1_1,     backRightMotor,        tmotorTetrix, openLoop)
#pragma config(Motor,  mtr_S1_C1_2,     frontLeftMotor,        tmotorTetrix, openLoop, reversed)
#pragma config(Motor,  mtr_S1_C2_2,     backLeftMotor,        tmotorTetrix, openLoop)
#pragma config(Motor,  mtr_S1_C4_1,     motorH,        tmotorTetrix, openLoop, reversed, encoder)
#pragma config(Motor,  mtr_S1_C4_2,     frontRightMotor,        tmotorTetrix, openLoop, reversed)
#pragma config(Servo,  srvo_S1_C3_1,    clawServo,               tServoStandard)
#pragma config(Servo,  srvo_S1_C3_2,    servo2,               tServoNone)
#pragma config(Servo,  srvo_S1_C3_3,    servo3,               tServoNone)
#pragma config(Servo,  srvo_S1_C3_4,    servo4,               tServoNone)
#pragma config(Servo,  srvo_S1_C3_5,    servo5,               tServoNone)
#pragma config(Servo,  srvo_S1_C3_6,    servo6,               tServoNone)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#include "JoystickDriver.c"

#pragma platform(Tetrix)

/* Motor Roles:
 * - MotorA           moves ramp
 * - frontLeftMotor   moves robot, front left wheel
 * - backRightMotor   moves robot, back right wheel
 * - frontRightMotor  moves robot, front right wheel
 * - backLeftMotor    moves robot, back left wheel
 * - MotorH           moves arm up and down
 * - clawServo        moves hand
 */

//#define ENABLE_LATCH
#define ENABLE_ARM
//#define ENABLE_CLAW
//#define ENABLE_MACRO_BUTTONS

void omniDrive(int joyx, int joyy, float scale, int joyspin);

#ifdef ENABLE_LATCH
void latchrelease();
#endif

#ifdef ENABLE_ARM
void armMove(int moveUp);
#endif

#ifdef ENABLE_MACRO_BUTTONS
//Refine presets
void armMacro(int set, int sensor);
const int presets[4] = {0, 360,  720, 1080};
#endif

#define NORMAL_SCALE 1.0
#define SLOW_SCALE 0.5
float scale = NORMAL_SCALE;

task main() {
    int encoder = 0;
    while(true) {
        getJoystickSettings(joystick);

        //see if btn 8 is depressed.
        //if so set a scale factor for all movement calculations
        //in omnidrive function
        if(joy1Btn(8)) {
            scale = SLOW_SCALE;
        } else {
            scale = NORMAL_SCALE;
        }
        omniDrive(joystick.joy1_x1, joystick.joy1_y1, scale, joystick.joy1_x2);

#ifdef ENABLE_CLAW

        switch(joystick.joy1_TopHat) {
        case 0:
        case 1:
        case 7:
            servo[clawServo] = 40;
            break;

        case 5:
        case 4:
        case 3:
            servo[clawServo] = -40;
            break;

        default:
            servo[clawServo] = 0;
        }

#endif

        int preset = 0;

#ifdef ENABLE_MACRO_BUTTONS
        if(joy1Btn(1)) {
            preset = 1;
        }
        if(joy1Btn(2)) {
            preset = 2;
        }
        if(joy1Btn(3)) {
            preset = 3;
        }
        if(joy1Btn(4)) {
            preset = 4;
        }
#endif

#ifdef ENABLE_ARM

        if(joy1Btn(7) && joy1Btn(5)) {
            //if both 7 and 5 are depressed DO NOTHING!
            preset = -1;
            armMove(0);
            writeDebugStreamLine("Both #5 and #7 depressed");
        } else if(joy1Btn(7)) {
		//Call are movement function; move down
            armMove(-1);
            preset = -1;
            writeDebugStreamLine("joy1_btn 7 depressed; moving arm down");
        } else if(joy1Btn(5)) {
            //call arm movement function; Move up
            armMove(1);
            preset = -1;
            writeDebugStreamLine("joy1_btn 5 depressed; moving arm up");
	} else if(preset < 0) {
	    armMove(0);
	}

#endif

#ifdef ENABLE_MACRO_BUTTONS
        // NOTE: If the arm motor can turn more than 32,767 degrees in
        //       either direction, the encoder value cannot be relied upon
        //       alone, and a separate variable (preferably a long) should
        //       be used to accumulate encoder ticks.
        if(preset >= 0) {
            armMacro(presets[preset], nMotorEncoder[motorH]);
        } else {

        }

#endif

#ifdef ENABLE_LATCH

        if(joy1Btn(10) && joy1Btn(9)) {
            //Release Latch
            latchrelease(encoder);
            break;
        }

#endif

    }
    writeDebugStreamLine("Main execution aborted");
}

int min(int a,int b) {
    return a<b ? a : b;
}

//float scale: multiplies by the scale factor to receive new speed
void omniDrive(int joyx, int joyy, float scale, int joyspin) {
    float x    = (100.0/128) * (float)joyx,
          y    = (100.0/128) * (float)joyy,
          spin = (50.0/128)  * (float)joyspin;

    int upRightSpeed = (x + y)  / sqrt(2);
    int upLeftSpeed  = (-x + y) / sqrt(2);

    int frontRight  = min(100,upLeftSpeed  - spin) * scale,
        frontLeft   = min(100,upRightSpeed + spin) * scale,
        bottomRight = min(100,upRightSpeed - spin) * scale,
        bottomLeft  = min(100,upLeftSpeed  + spin) * scale;

    motor[frontLeftMotor]  = frontLeft;
    motor[backRightMotor]  = bottomRight;
    motor[backLeftMotor]   = bottomLeft;
    motor[frontRightMotor] = frontRight;

    writeDebugStreamLine("frontRightMotor:%d,frontLeftMotor:%d,backRightMotor:%d,backLeftMotor:%d", frontRight,frontLeft,bottomRight,bottomLeft);
}

#ifdef ENABLE_LATCH

void latchrelease(int armPos) {
    writeDebugStreamLine("Latch Release initiated.");
    #define LATCH_TARGET 270
    #define ARM_TARGET
    int latchEncoder = 0, armEncoder = 0;
    //TODO: test latch release
    while(nMotorEncoder[motorA] < LATCH_TARGET) {
        motor[motorA] = 50;
    }
    motor[motorA] = 0;
    motor[motorH] = 0;
    while (armEncoder < ARM_TARGET - armPos) {
            armEncoder += nMotorEncoder[motorH];
            nMotorEncoder[motorH] = 0;
    }
    writeDebugStreamLine("Latch release completed.");
}

#endif

#ifdef ENABLE_ARM

void armMove(int moveUp) {
    //TODO: Tune movement speed
#define ARM_SPEED 40
    //TODO: Motor designation changed
    if(moveUp>0) {
        motor[motorH] = ARM_SPEED;
    } else if(moveUp<0) {
        motor[motorH] = - ARM_SPEED;
    } else {
        motor[motorH] = 0;
    }
}

#endif

#ifdef ENABLE_MACRO_BUTTONS

void armMacro(int set, int sensor) {
#define KP 1
#define MIN_ERR 15
#define ARM_SPEEDLIMIT 50
    int err = set - sensor;
    if(abs(err) <= MIN_ERR) {
        motor[motorH] = min(ARM_SPEEDLIMIT,KP * err);
    } else {
        motor[motorH] = 0;
    }
}

#endif
