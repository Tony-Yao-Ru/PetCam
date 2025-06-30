#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

#define CHIPNAME "gpiochip0"

// FRONT Motor Module (Steering - Front Wheels)
#define FRONT_IN1 17  // Motor A
#define FRONT_IN2 22
#define FRONT_IN3 23  // Motor B
#define FRONT_IN4 24

// BACK Motor Module (Drive - Back Wheels)
#define BACK_IN1 5    // Motor C
#define BACK_IN2 6
#define BACK_IN3 13   // Motor D
#define BACK_IN4 19

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

// --- Main Program ---
int main() {
    struct gpiod_chip *chip = gpiod_chip_open_by_name(CHIPNAME);
    if (!chip) {
        perror("Failed to open GPIO chip");
        return 1;
    }
    
    /*
    printf("Action: Move Forward\n");
    front_forward(chip);   // wheels straight
    back_forward(chip);    // drive forward
    sleep(2);
   

    printf("Action: Move Backward\n");
    front_backward(chip);   // wheels straight
    back_backward(chip);   // drive backward
    sleep(2);
    */

    /*
    printf("Action: Turn Left\n");
    front_left_turn(chip); // wheels turn left
    back_forward(chip);    // optional forward movement
    sleep(2);
    */

    
    printf("Action: Turn Right\n");
    front_right_turn(chip); // wheels turn right
    back_forward(chip);     // optional forward movement
    sleep(2);
    

    printf("Action: Stop\n");
    all_stop(chip);
   

    gpiod_chip_close(chip);
    return 0;
}

