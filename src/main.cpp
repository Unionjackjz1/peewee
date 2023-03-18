/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "main.h"

void at_roller() {
  relative_ptp(2, 20);
  set_intake(50);
  pros::delay(500);
  set_intake(0);
}

// Display constructor
ez::GUI display(
    {
        {l1, "left 1"},
        {r1, "right 1"},
        {intake, "roller"},
        {lift_motor, "lift"}
    },

    {{"Drive 36in - Turn three times - come back", auton1},
     {"at roller", at_roller}});

// This occurs as soon as the program starts.
void initialize() {
  pros::delay(300);

  display.enable();

  set_pid_defaults();
  reset_trackers();

  set_curve_default(0.0, 0.0);
  set_left_curve_buttons(pros::E_CONTROLLER_DIGITAL_LEFT, pros::E_CONTROLLER_DIGITAL_RIGHT);
  set_right_curve_buttons(pros::E_CONTROLLER_DIGITAL_Y, pros::E_CONTROLLER_DIGITAL_A);

  imu.reset(true);
  imu.set_data_rate(5);
  master.rumble(".");
}

// Runs while the robot is in disabled at on a competition.
void disabled() { trackingTask.suspend(); }

// Runs after initialize(), and before autonomous when connected to a competition.
void competition_initialize() { trackingTask.suspend(); }

// Runs the user autonomous code.
void autonomous() {
  set_pid_defaults();
  reset_odom();
  reset_pid_targets();
  drive_brake(MOTOR_BRAKE_HOLD);
  set_angle(0);
  trackingTask.resume();
  auton_start_time = pros::millis();

  display.auton_call();
}

// Runs the operator control code.
void opcontrol() {
  // Drive brake, this is preference
  drive_brake(MOTOR_BRAKE_BRAKE);

  // pose starting = {4.8, 4.3, 180};
  // set_pose(starting);

  while (true) {
    // printf("(%f, %f, %f)\n", current.x, current.y, current.theta);
    // printf("rawlr(%ld, %ld)   lr(%f, %f)   (%f, %f, %f)\n", get_raw_left(), get_raw_right(), get_left(), get_right(), current.x, current.y, current.theta);
    intake_opcontrol();
    lift_control();

    if (master.get_digital_new_press(DIGITAL_X))
      is_tank = !is_tank;

    if (is_tank)
      tank_control();
    else
      arcade_control();

    modify_curve_with_controller();

    pros::delay(DELAY_TIME);
  }
}