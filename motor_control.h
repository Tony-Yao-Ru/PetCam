// motor_control.h

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <gpiod.h>

#define CHIPNAME "gpiochip0"

// FRONT Motor Module (Steering - Front Wheels)
#define FRONT_IN1 17
#define FRONT_IN2 22
#define FRONT_IN3 23
#define FRONT_IN4 24

// BACK Motor Module (Drive - Back Wheels)
#define BACK_IN1 5
#define BACK_IN2 6
#define BACK_IN3 13
#define BACK_IN4 19

// Motor Moving period
#define MOVINGPEIOD 400000 //500000 micoseconds

void set_gpio(struct gpiod_chip *chip, int gpio, int value);

// Front Wheels (Steering)
void front_forward(struct gpiod_chip *chip);
void front_backward(struct gpiod_chip *chip);
void front_left_turn(struct gpiod_chip *chip);
void front_right_turn(struct gpiod_chip *chip);
void front_stop(struct gpiod_chip *chip);

// Back Wheels (Drive)
void back_forward(struct gpiod_chip *chip);
void back_backward(struct gpiod_chip *chip);
void back_stop(struct gpiod_chip *chip);

// All Stop
void all_stop(struct gpiod_chip *chip);

// Buttons
void onClickedLeft(struct gpiod_chip *chip);
void onClickedForward(struct gpiod_chip *chip);
void onClickedBackward(struct gpiod_chip *chip);
void onClickedRight(struct gpiod_chip *chip);

#endif

