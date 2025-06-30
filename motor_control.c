// motor_control.c

#include "motor_control.h"
#include <stdio.h>
#include <unistd.h>

void set_gpio(struct gpiod_chip *chip, int gpio, int value) {
    struct gpiod_line *line = gpiod_chip_get_line(chip, gpio);
    if (!line) {
        fprintf(stderr, "Failed to get GPIO line %d\n", gpio);
        return;
    }
    if (gpiod_line_request_output(line, "motor", 0) < 0) {
        fprintf(stderr, "Failed to request output for GPIO %d\n", gpio);
        gpiod_line_release(line);
        return;
    }
    gpiod_line_set_value(line, value);
    gpiod_line_release(line);
}

// --- Front Wheels (Steering) ---
void front_forward(struct gpiod_chip *chip) {
    set_gpio(chip, FRONT_IN1, 1); set_gpio(chip, FRONT_IN2, 0);
    set_gpio(chip, FRONT_IN3, 1); set_gpio(chip, FRONT_IN4, 0);
}

void front_backward(struct gpiod_chip *chip) {
    set_gpio(chip, FRONT_IN1, 0); set_gpio(chip, FRONT_IN2, 1);
    set_gpio(chip, FRONT_IN3, 0); set_gpio(chip, FRONT_IN4, 1);
}

void front_left_turn(struct gpiod_chip *chip) {
    set_gpio(chip, FRONT_IN1, 0); set_gpio(chip, FRONT_IN2, 1);
    set_gpio(chip, FRONT_IN3, 1); set_gpio(chip, FRONT_IN4, 0);
}

void front_right_turn(struct gpiod_chip *chip) {
    set_gpio(chip, FRONT_IN1, 1); set_gpio(chip, FRONT_IN2, 0);
    set_gpio(chip, FRONT_IN3, 0); set_gpio(chip, FRONT_IN4, 1);
}

void front_stop(struct gpiod_chip *chip) {
    set_gpio(chip, FRONT_IN1, 0); set_gpio(chip, FRONT_IN2, 0);
    set_gpio(chip, FRONT_IN3, 0); set_gpio(chip, FRONT_IN4, 0);
}

// --- Back Wheels (Drive) ---
void back_forward(struct gpiod_chip *chip) {
    set_gpio(chip, BACK_IN1, 0); set_gpio(chip, BACK_IN2, 1);
    set_gpio(chip, BACK_IN3, 0); set_gpio(chip, BACK_IN4, 1);
}

void back_backward(struct gpiod_chip *chip) {
    set_gpio(chip, BACK_IN1, 1); set_gpio(chip, BACK_IN2, 0);
    set_gpio(chip, BACK_IN3, 1); set_gpio(chip, BACK_IN4, 0);
}

void back_stop(struct gpiod_chip *chip) {
    set_gpio(chip, BACK_IN1, 0); set_gpio(chip, BACK_IN2, 0);
    set_gpio(chip, BACK_IN3, 0); set_gpio(chip, BACK_IN4, 0);
}

// --- Full Stop ---
void all_stop(struct gpiod_chip *chip) {
    front_stop(chip);
    back_stop(chip);
}

void onClickedLeft(struct gpiod_chip *chip) {
	front_left_turn(chip);   // turn wheels left
        back_forward(chip);      // drive forward
        usleep(MOVINGPEIOD);
        all_stop(chip);
}

void onClickedForward(struct gpiod_chip *chip) {
	front_forward(chip);     // align front wheels
        back_forward(chip);      // drive forward
        usleep(MOVINGPEIOD);
        all_stop(chip);
}

void onClickedBackward(struct gpiod_chip *chip) {
	front_backward(chip);     // keep wheels straight
        back_backward(chip);     // reverse
        usleep(MOVINGPEIOD);
	all_stop(chip);
}

void onClickedRight(struct gpiod_chip *chip) {
	front_right_turn(chip);  // turn wheels right
	back_forward(chip);      // drive forward
	usleep(MOVINGPEIOD);
	all_stop(chip);
}
