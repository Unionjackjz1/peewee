#include "main.h"
#include "pros/adi.hpp"

using namespace ez;

const int SLOW_SPEED = 100;
const int FAST_SPEED = 127;

PID liftPID{4, 0, 0, 0, "Lift"};
int lift_max_speed = 127;
void set_lift_speed(int input) { lift_max_speed = abs(input); }
void set_lift(int input) { lift_motor = input; }
void reset_lift() { lift_motor.tare_position(); }

void set_lift_exit() { liftPID.set_exit_condition(80, 20, 300, 50, 500, 500); }

std::string lift_state_to_string(lift_state input) {
  switch (input) {
    case DOWN:
      return "Down";
      break;
    case MID:
      return "Mid";
      break;
    case UP:
      return "Up";
      break;
    default:
      return "Out of bounds lift state";
      break;
  }
}

lift_state current_lift_state;
void set_lift_state(lift_state input) {
  set_lift_speed(current_lift_state > input ? SLOW_SPEED : FAST_SPEED);
  current_lift_state = input;
  liftPID.set_target(input);
  std::cout << "\nNew Lift State: " << lift_state_to_string(input);
}

void liftTask() {
  double output = 0;
  long timer = 0;
  bool did_reset = false;
  while (true) {
    double current = lift_motor.get_position();
    double clipped_pid = clip_num(liftPID.compute(current), lift_max_speed, -lift_max_speed);

    if (current_lift_state == DOWN) {
      if (current >= 20)
        output = clipped_pid;
      else {
        bool check = (lift_motor.get_actual_velocity() == 0 && !pros::competition::is_disabled()) ? true : false;
        if (check) timer += DELAY_TIME;
        if (timer >= 250) {
          output = -3;
          if (!did_reset) reset_lift();
          did_reset = true;
          timer = 250;
        } else {
          output = -40;
        }
      }
    } else {
      timer = 0;
      did_reset = false;
      output = clipped_pid;
    }

    if (pros::competition::is_disabled()) timer = 0;

    set_lift(output);

    pros::delay(DELAY_TIME);
  }
}
pros::Task lift_task(liftTask);

void wait_lift() {
  while (liftPID.exit_condition(lift_motor, true) == RUNNING) {
    pros::delay(DELAY_TIME);
  }
}

bool last_l1 = 0;
// bool last_r2 = 0;
void lift_control() {
  if (master.get_digital(DIGITAL_L1) && last_l1 == 0) {
    if (current_lift_state == UP || current_lift_state == MID)
      set_lift_state(DOWN);
    else
      set_lift_state(UP);
  }
  last_l1 = master.get_digital(DIGITAL_L1);

  // if (master.get_digital(DIGITAL_R2) && last_r2 == 0)
  //   set_lift_state(MID);
  // last_r2 = master.get_digital(DIGITAL_R2);
}