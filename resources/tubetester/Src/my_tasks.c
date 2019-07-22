#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shared.h"
#include "my_tasks.h"
#include "main.h"
#include "my_usb.h"
#include "helpers.h"
#include "animations.h"
#include "delay_us.h"
#include "my_1wire.h"
#include "ds18b20.h"
#include "eeprom.h"
#include "gps_config.h"
#include "minmea.h"
#include "config.h"
#include "buttons.h"

#define TUBE1 5
#define TUBE2 4
#define TUBE3 3
#define TUBE4 2
#define TUBE5 1
#define TUBE6 0

uint8_t spi_buf[SPI_CMD_SIZE];
uint32_t frame_counter;
digit_animation tube_animation[TUBE_COUNT];
led_animation rgb_animation[TUBE_COUNT];
my_button up_button, down_button;
uint8_t rgb_red[LED_CHANNEL_SIZE] = {255, 0, 0};
uint8_t rgb_green[LED_CHANNEL_SIZE] = {0, 255, 0};
uint8_t rgb_blue[LED_CHANNEL_SIZE] = {0, 0, 255};

void spi_send(uint8_t* data, uint8_t size, uint8_t index)
{
  GPIO_TypeDef *target_port;
  uint16_t target_pin;

  switch(index)
  {
    case TUBE1:
    target_port = EXIXE1_CS_GPIO_Port;
    target_pin = EXIXE1_CS_Pin;
    break;

    case TUBE2:
    target_port = EXIXE2_CS_GPIO_Port;
    target_pin = EXIXE2_CS_Pin;
    break;

    case TUBE3:
    target_port = EXIXE3_CS_GPIO_Port;
    target_pin = EXIXE3_CS_Pin;
    break;

    case TUBE4:
    target_port = EXIXE4_CS_GPIO_Port;
    target_pin = EXIXE4_CS_Pin;
    break;

    case TUBE5:
    target_port = EXIXE5_CS_GPIO_Port;
    target_pin = EXIXE5_CS_Pin;
    break;

    case TUBE6:
    target_port = EXIXE6_CS_GPIO_Port;
    target_pin = EXIXE6_CS_Pin;
    break;

    default:
    return;
  }

  HAL_GPIO_WritePin(target_port, target_pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, data, size, 100);
  HAL_GPIO_WritePin(target_port, target_pin, GPIO_PIN_SET);
}

void setup_task(void)
{
  HAL_GPIO_WritePin(EXIXE_RESET_GPIO_Port, EXIXE_RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(EXIXE_RESET_GPIO_Port, EXIXE_RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(10);
   
  for (int i = 0; i < TUBE_COUNT; ++i)
    animation_init(&(tube_animation[i]));
  for (int i = 0; i < TUBE_COUNT; ++i)
    led_animation_init(&(rgb_animation[i]));
  for (int i = 0; i < TUBE_COUNT; ++i)
    led_start_animation(&(rgb_animation[i]), rgb_red, ANIMATION_CROSS_FADE);
  button_init(&up_button, HAL_GPIO_ReadPin(UP_BUTTON_GPIO_Port, UP_BUTTON_Pin));
  button_init(&down_button, HAL_GPIO_ReadPin(DOWN_BUTTON_GPIO_Port, DOWN_BUTTON_Pin));
}

void animation_task_start(void const * argument)
{
  for(;;)
  {
    frame_counter++;

    for (int i = 0; i < TUBE_COUNT; ++i)
    {
      animation_handler(&(tube_animation[i]));
      led_animation_handler(&(rgb_animation[i]));
    }

    for (int curr_tube = 0; curr_tube < TUBE_COUNT; ++curr_tube)
    {
      spi_buf[0] = SPI_CMD_UPDATE;
      if(tube_animation[curr_tube].end_digit == DIGIT_2 || tube_animation[curr_tube].end_digit == DIGIT_3)
        spi_buf[0] |= 0x1;

      // digits
      for (int j = 1; j < SPI_SMD_DIGIT_END; ++j)
        spi_buf[j] = ((uint8_t)(tube_animation[curr_tube].pwm_status[j]) >> 1) | 0x80;
      // dots
      for (int j = SPI_SMD_DIGIT_END; j < SPI_CMD_DOT_END; ++j)
        spi_buf[j] = (tube_animation[curr_tube].pwm_status[j] >> 1) | 0x80;
      // LEDs
      for (int j = SPI_CMD_DOT_END; j < SPI_CMD_SIZE; ++j)
        spi_buf[j] = (((uint8_t)(rgb_animation[curr_tube].pwm_status[j - SPI_CMD_DOT_END])) >> 1) | 0x80;
      spi_send(spi_buf, SPI_CMD_SIZE, curr_tube);
    }
    osDelay(17);
  }
}

void test_task_start(void const * argument)
{
  uint8_t count = 0;
  uint8_t *color_ptr;
  uint8_t digit;
  for(;;)
  {
    count++;
    if(count % 3 == 0)
      color_ptr = rgb_red;
    else if(count % 3 == 1)
      color_ptr = rgb_green;
    else
      color_ptr = rgb_blue;
    for (int i = 0; i < TUBE_COUNT; ++i)
      led_start_animation(&(rgb_animation[i]), color_ptr, ANIMATION_CROSS_FADE);

    digit = (count % 10) * 11;
    tube_print2(digit, &(tube_animation[5]), &(tube_animation[4]), ANIMATION_CROSS_FADE);
    tube_print2(digit, &(tube_animation[3]), &(tube_animation[2]), ANIMATION_CROSS_FADE);
    tube_print2(digit, &(tube_animation[1]), &(tube_animation[0]), ANIMATION_CROSS_FADE);
    osDelay(1000);
  }
}
