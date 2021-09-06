#include <stdlib.h>
#include "dorobo32.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "stm32f0xx_hal.h"
#include "motor.h"

static void hulk(void *pvParameters);

int motorSpeedPos = 45;
int motorSpeedNeg = -45;
int motorSpeedPosSlow = 20;
int motorSpeedNegSlow = -20;
int motorSpeedZero = 0;
int motorSpeedPosFast = 48;
int motorSpeedNegFast = -48;

int main()
{
  dorobo_init(); //Call dorobo_init() function to initialize HAL, Clocks, Timers etc.

  xTaskCreate(hulk, "BLINKYTASK", 512, NULL, 2, NULL);  //create blinky task

  vTaskStartScheduler();  //start the freertos scheduler

  return 0;       //should not be reached!
}

/**
 * @brief pauses the drive
 */

void pause_drive()
{
  tracef("driving pause");
  motor_set(DM_MOTOR0, motorSpeedZero);
  motor_set(DM_MOTOR1, motorSpeedZero);
  motor_set(DM_MOTOR2, motorSpeedZero);

}

/**
 * @brief makes a 360 degree turn
 */
void turn_360()
{
  motor_set(DM_MOTOR0, motorSpeedPos);
  motor_set(DM_MOTOR1, motorSpeedPos);
  motor_set(DM_MOTOR2, motorSpeedPos);
  vTaskDelay(3420);
  pause_drive();
}
/**
 * @brief makes a 180 degree turn
 */

void turn_180()
{
  motor_set(DM_MOTOR0, motorSpeedPos);
  motor_set(DM_MOTOR1, motorSpeedPos);
  motor_set(DM_MOTOR2, motorSpeedPos);
  vTaskDelay(3420 / 2);
  pause_drive();
}

/**
 * @brief makes a 135 degree turn
 */
void turn_135()
{
  motor_set(DM_MOTOR0, motorSpeedPos);
  motor_set(DM_MOTOR1, motorSpeedPos);
  motor_set(DM_MOTOR2, motorSpeedPos);
  vTaskDelay(1000);
  pause_drive();
}

/**
 * @brief makes a left turn
 */
void turn_left()
{
  motor_set(DM_MOTOR0, motorSpeedPos);
  motor_set(DM_MOTOR1, motorSpeedPos);
  motor_set(DM_MOTOR2, motorSpeedPos);
  vTaskDelay(180);
}

/**
 * @brief makes a right turn
 */
void turn_right()
{
  motor_set(DM_MOTOR0, motorSpeedNeg);
  motor_set(DM_MOTOR1, motorSpeedNeg);
  motor_set(DM_MOTOR2, motorSpeedNeg);
  vTaskDelay(180);
}

/**
 * @brief drives in the forward direction
 */
void drive_straight()
{
  tracef("driving straight");
  motor_set(DM_MOTOR0, motorSpeedNegFast);
  motor_set(DM_MOTOR1, motorSpeedPosFast);
  motor_set(DM_MOTOR2, motorSpeedZero);
  vTaskDelay(180);
}

/**
 * @brief drives towards the transmitter if the target is locked
 */
void drive_straight_to_tx()
{
  while(1)
  {
    tracef("driving straight");
    drive_straight();
    pause_drive();
    int right_hit = 0; //stores the switch reading if the right switch is triggered
    int left_hit = 0;  //stores the switch reading if the left switch is triggered
    digital_configure_pin(DD_PIN_PB3, DD_CFG_INPUT_PULLUP);
    digital_configure_pin(DD_PIN_PB11, DD_CFG_INPUT_PULLUP);
    for (int j = 0; j < 1000; j++)
    {
      // read switch ip
      //working pins - PB3(right) and PB11(left)
      enum DD_PINLEVEL_E switch_right = digital_get_pin(DD_PIN_PB3);
      enum DD_PINLEVEL_E switch_left = digital_get_pin(DD_PIN_PB11);
      //checks if the left or the right switch is triggered
      if (switch_right == 0)
        right_hit++;

      if (switch_left == 0)
        left_hit++;
    }

    //the robot hits the transmitter
    if (right_hit > 0 || left_hit > 0)
    {
      pause_drive();
      break;
    }
  }
  pause_drive();
}
/**
 * @brief drives in the reverse direction
 */
void drive_reverse()
{
  motor_set(DM_MOTOR0, motorSpeedPos);
  motor_set(DM_MOTOR1, motorSpeedNeg);
  motor_set(DM_MOTOR2, motorSpeedZero);
  vTaskDelay(200);
}
/**
 * @brief drives in the reverse and left  direction when a corner is encountered
 */
void drive_reverse_corner_left()
{
  motor_set(DM_MOTOR0, motorSpeedPos - 10);
  motor_set(DM_MOTOR1, motorSpeedNeg + 10);
  motor_set(DM_MOTOR2, motorSpeedPos - 10);
  vTaskDelay(200);
}
/**
 * @brief /drives in the reverse and right  direction when a corner is encountered
 */
void drive_reverse_corner_right()
{
  motor_set(DM_MOTOR0, motorSpeedPos + 10);
  motor_set(DM_MOTOR1, motorSpeedNeg - 10);
  motor_set(DM_MOTOR2, motorSpeedNeg - 10);
  vTaskDelay(200);
}
/**
 * @brief initializes the system
 */
void system_init()
{
  dorobo_init();
  trace_init();
  motor_init();
  digital_init();
  adc_init();
}
/**
 * @brief method to test the analog readings detected by the IR sensor
 */
void test_IR()
{
  while (1)
  {
    int right_detected = 0;    //stores the value of thr right IR sensor
    int left_detected = 0;     //stores the value of the left IR sensor
    for (int j = 0; j < 100; j++)
    { // read IR again
      uint32_t IR_right = adc_get_value(7);   //set the right IR pin to analog 7
      uint32_t IR_left = adc_get_value(9);     //set the left IR pin to analog 9
      if (IR_right == 0)
        right_detected++;
      if (IR_left == 0)
        left_detected++;
      tracef("IR : right - %d....left - %d\n", IR_right, IR_left);
    }
    tracef("total : right - %d....left - %d\n\n\n", right_detected,
        left_detected);
    vTaskDelay(1000);

  }
}
/**
 * @brief method to read if the transmitter has been locked
 * @return
 * 0 - if transmitter has not been locked
 * 1 - if transmitter is detected
 */
int IR_read()
{
  tracef("reading IR");
  int right_detected = 0;
  int left_detected = 0;
  digital_configure_pin(DD_PIN_PD14, DD_CFG_INPUT_PULLDOWN);
  digital_configure_pin(DD_PIN_PC9, DD_CFG_INPUT_PULLDOWN);
  for (int j = 0; j < 100; j++)
  {
    // read IR again
    enum DD_PINLEVEL_E IR_right = digital_get_pin(DD_PIN_PD14);
    enum DD_PINLEVEL_E IR_left = digital_get_pin(DD_PIN_PC9);
    //checks if the right IR sensor has read the transmitter
    if (IR_right == 0)
      right_detected++;
    //checks if the LEFT IR sensor has read the transmitter
    if (IR_left == 0)
      left_detected++;
  }
  //checks if the transmitter has been locked
  if ((left_detected >= 45 || right_detected >= 45))
  {
    //returns 1 if the transmitter has been locked
    return 1; //transmitter detected, breaking out of while
    tracef("YAAAAAYYYY.. found IR...");
  }
  else
  {
    //returns 0 if the transmitter has not been locked
    return 0;
  }
}

/**
 * @brief method to test the readings detected by the distance sensor
 */
void test_dist_sensors()
{
  while (1)
  {
    int right_detected = 0;
    int left_detected = 0;
    // read dist sensor ip
    uint32_t dist_right = adc_get_value(0);
    uint32_t dist_left = adc_get_value(1);
    tracef("dist : right - %d.... left - %d\n", dist_right, dist_left);
    vTaskDelay(100);
  }
}
/**
 * @brief method to test the digital readings detected by the switch
 */

void test_switch()
{
  while (1)
  {
    int right_detected = 0;
    int left_detected = 0;
    digital_configure_pin(DD_PIN_PD14, DD_CFG_INPUT_PULLDOWN);
    digital_configure_pin(DD_PIN_PC9, DD_CFG_INPUT_PULLDOWN);
    for (int j = 0; j < 1500; j++)
    { // read switch ip
      //working pins - PD14(right) and PC9(left)
      enum DD_PINLEVEL_E switch_right = digital_get_pin(DD_PIN_PD14);
      enum DD_PINLEVEL_E switch_left = digital_get_pin(DD_PIN_PC9);
      if (switch_right == 0)
        right_detected++;
      if (switch_left == 0)
        left_detected++;
      tracef("switch : right - %d.... left - %d", switch_right, switch_left);
    }
    tracef("total : right - %d....left - %d\n\n\n", right_detected,
        left_detected);
    vTaskDelay(1000);
  }
}

/**
 * @brief implements the logic of the robot turning its path depending on if any of the switches are triggered
 */
void switch_logic()
{
  int right_hit = 0; //stores the switch reading if the right switch is triggered
  int left_hit = 0;  //stores the switch reading if the left switch is triggered
  digital_configure_pin(DD_PIN_PB3, DD_CFG_INPUT_PULLUP);
  digital_configure_pin(DD_PIN_PB11, DD_CFG_INPUT_PULLUP);
  for (int j = 0; j < 1000; j++)
  {
    // read switch ip
    //working pins - PB3(right) and PB11(left)
    enum DD_PINLEVEL_E switch_right = digital_get_pin(DD_PIN_PB3);
    enum DD_PINLEVEL_E switch_left = digital_get_pin(DD_PIN_PB11);
    //checks if the left or the right switch is triggered
    if (switch_right == 0)
      right_hit++;

    if (switch_left == 0)
      left_hit++;
  }

  //the robot hits the center and both the switches are triggered at once
  if (right_hit > 0 && left_hit > 0)
  {
    pause_drive();
    vTaskDelay(300);
    drive_reverse();
    turn_180(); //turns its path and goes in the opposite direction as there is an obstacle head-on
    pause_drive();
  }
  //the right switch is triggered and the robot turns left
  else if (right_hit > 0)
  {
    pause_drive();
    vTaskDelay(300);
    drive_reverse();
    turn_left();
    vTaskDelay(200);
    pause_drive();
  }
  //the left switch is triggered and the robot turns right
  else if (left_hit > 0)
  {
    pause_drive();
    vTaskDelay(300);
    drive_reverse();
    turn_right();
    vTaskDelay(200);
    pause_drive();
  }

}
/**
 * @brief implements the logic of the robot deflecting obstacles and finding the target
 */
void working_code(int i)
{
  while (1)
  {
    int left_obst = 0;
    int right_obst = 0;
    //calls the switch logic method
    switch_logic();
    // checks if any obstacle is ahead using the distance sensor
    uint32_t adc_right = adc_get_value(0);
    uint32_t adc_left = adc_get_value(1);
    int adc_distance = 1400;
    //a corner situation has been detected
    if ((adc_right > adc_distance && adc_left > adc_distance))
    { // corner!!
      pause_drive();
      vTaskDelay(100);

      //the robot is in the right corner and has to turn left
      if (adc_right > adc_left + 70)
      {
        drive_reverse_corner_left();
        pause_drive();
        vTaskDelay(100);
        turn_left();
        vTaskDelay(240);
      }
      //the robot is in the left corner and has to turn right
      else
      {
        drive_reverse_corner_right();
        pause_drive();
        vTaskDelay(100);
        turn_right();
        vTaskDelay(240);
      }

      vTaskDelay(300);
    }

    else if (adc_right > adc_distance || adc_left > adc_distance)
    {
      //the right sensor senses an obstacle and the robot turns left
      if (adc_right > adc_left + 70)
      {
        //turn left
        pause_drive();
        turn_left();

      }
      //the left sensor senses an obstacle and the robot turns right
      else
      {
        // turn right
        pause_drive();
        turn_right();
      }
    }
    // no obstacle detected, can traverse in the forward direction
    else
    {
      // go straight
      drive_straight();
      if (IR_read() == 1)    //checks if the transmitter has been locked
      {
        tracef("found IR.., leaving while");   //transmitter has been locked
        break; //transmitter detected, breaking out of while
      }

    }
    i++;

  }
  // found transmitter - going towards transmitter

  drive_straight_to_tx();

}

static void hulk(void *pvParameters)
{
  int row = 10;
  int arr[2][row];
  int i = 0;
  system_init();
  vTaskDelay(500);
  working_code(i);
}
