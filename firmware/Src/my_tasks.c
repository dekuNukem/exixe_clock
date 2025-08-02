#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shared.h"
#include "my_tasks.h"
#include "main.h"
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
#include "fatfs.h"

// comment this out to use IN-12 tubes
// #define USE_EXIXE_14

#ifdef USE_EXIXE_14
#define TUBE1 0
#define TUBE2 1
#define TUBE3 2
#define TUBE4 3
#define TUBE5 4
#define TUBE6 5
#else
#define TUBE1 5
#define TUBE2 4
#define TUBE3 3
#define TUBE4 2
#define TUBE5 1
#define TUBE6 0
#endif

uint8_t spi_buf[SPI_CMD_SIZE];
uint8_t gps_byte_buf[1];
uint32_t frame_counter;
int16_t raw_temp, temp_c_int;
int32_t current_time;
uint8_t year, month, day, hour, minute, second;
uint8_t display_mode, is_in_setup_mode, use_24hour;
int8_t utc_offset;
digit_animation tube_animation[TUBE_COUNT];
led_animation rgb_animation[TUBE_COUNT];
linear_buf gps_lb;
my_button up_button, down_button;
uint8_t rgb_orange[LED_CHANNEL_SIZE] = {255, 55, 0};
uint8_t rgb_purple[LED_CHANNEL_SIZE] = {255, 0, 255};
struct minmea_sentence_rmc gps_rmc;
struct minmea_sentence_gga gps_gga;
struct minmea_sentence_gsa gps_gsa;
struct minmea_sentence_gll gps_gll;
struct minmea_sentence_gst gps_gst;
struct minmea_sentence_gsv gps_gsv;

FRESULT sd_fresult;
FATFS sd_fs;
FIL sd_file;
DIR dir;
FILINFO fno;
UINT bytes_written, bytes_read;

char last_rmc[GPS_BUF_SIZE];

#define SD_WRITE_BUF_SIZE 256
char sd_write_buf[SD_WRITE_BUF_SIZE];

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
  linear_buf_init(&gps_lb, GPS_BUF_SIZE);
  gps_init();
  
  for (int i = 0; i < TUBE_COUNT; ++i)
    animation_init(&(tube_animation[i]));
  for (int i = 0; i < TUBE_COUNT; ++i)
    led_animation_init(&(rgb_animation[i]));
  for (int i = 0; i < TUBE_COUNT; ++i)
    led_start_animation(&(rgb_animation[i]), rgb_orange, ANIMATION_CROSS_FADE);

  button_init(&up_button, HAL_GPIO_ReadPin(UP_BUTTON_GPIO_Port, UP_BUTTON_Pin));
  button_init(&down_button, HAL_GPIO_ReadPin(DOWN_BUTTON_GPIO_Port, DOWN_BUTTON_Pin));
  HAL_UART_Receive_IT(gps_uart_ptr, gps_byte_buf, 1);
  display_mode = get_display_mode();
  utc_offset = get_utc_offset();
  use_24hour = get_use_24hour();
  if(HAL_GPIO_ReadPin(DOWN_BUTTON_GPIO_Port, DOWN_BUTTON_Pin) == GPIO_PIN_RESET)
    is_in_setup_mode = SETUP_UTC_OFFSET;
  else if(HAL_GPIO_ReadPin(UP_BUTTON_GPIO_Port, UP_BUTTON_Pin) == GPIO_PIN_RESET)
    is_in_setup_mode = SETUP_12H24H;
  else
    printf("launching scheduler...\n");

  sd_fresult = f_mount(&sd_fs, "", 1);
  printf("mount sd: %d\n", sd_fresult);
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
  for(;;)
  {
    if(is_in_setup_mode == SETUP_UTC_OFFSET)
    {
      if(button_update(&up_button, HAL_GPIO_ReadPin(UP_BUTTON_GPIO_Port, UP_BUTTON_Pin)) == BUTTON_SHORT_PRESS)
        utc_offset++;
      else if(button_update(&down_button, HAL_GPIO_ReadPin(DOWN_BUTTON_GPIO_Port, DOWN_BUTTON_Pin)) == BUTTON_SHORT_PRESS)
        utc_offset--;
      utc_offset = utc_offset_trim(utc_offset);
      tube_print2(utc_offset, &(tube_animation[3]), &(tube_animation[2]), ANIMATION_BREATHING);
      eeprom_write(EEPROM_ADDR_UTC_OFFSET, utc_offset);
    }
    else if(is_in_setup_mode == SETUP_12H24H)
    {
      if(button_update(&up_button, HAL_GPIO_ReadPin(UP_BUTTON_GPIO_Port, UP_BUTTON_Pin)) == BUTTON_SHORT_PRESS || \
        button_update(&down_button, HAL_GPIO_ReadPin(DOWN_BUTTON_GPIO_Port, DOWN_BUTTON_Pin)) == BUTTON_SHORT_PRESS)
        use_24hour = (use_24hour + 1) % 2;
      eeprom_write(EEPROM_ADDR_USE_24HR, use_24hour);
      if(use_24hour)
        tube_print2(24, &(tube_animation[3]), &(tube_animation[2]), ANIMATION_BREATHING);
      else
        tube_print2(12, &(tube_animation[3]), &(tube_animation[2]), ANIMATION_BREATHING);
    }
    else if(is_in_setup_mode == SETUP_NO_SETUP)
    {
      if(button_update(&up_button, HAL_GPIO_ReadPin(UP_BUTTON_GPIO_Port, UP_BUTTON_Pin)) == BUTTON_SHORT_PRESS || \
        button_update(&down_button, HAL_GPIO_ReadPin(DOWN_BUTTON_GPIO_Port, DOWN_BUTTON_Pin)) == BUTTON_SHORT_PRESS)
      {
        display_mode = (display_mode + 1) % DISPLAY_MODE_SIZE;
        eeprom_write(EEPROM_ADDR_DISPLAY_MODE, display_mode);
        printf("display_mode: %d\n", display_mode);
      }
    }
    osDelay(50);
  }
}

void replace_nlcr_with_space(char *str, size_t size)
{
  for (size_t i = 0; i < size; ++i)
    if (str[i] == '\n' || str[i] == '\r')
      str[i] = ' ';
}

const char log_file_path[] = "log.txt";

uint8_t write_log(char* msg)
{
  uint32_t bytes_written = 0;
  uint32_t msg_size = strlen(msg);
  if(f_open(&sd_file, log_file_path, 	FA_OPEN_ALWAYS | FA_WRITE))
    return 2;
  if(f_lseek(&sd_file, f_size(&sd_file)))
    return 3;
  if(f_write(&sd_file, msg, msg_size, &bytes_written))
    return 4;
  f_close(&sd_file);
  return 0;
}

uint8_t sd_error_count;
uint8_t counter;
// 1PPS interrupt
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(is_in_setup_mode)
    return;
  if(GPIO_Pin != GPS_TP_Pin)
    return;

  if(gps_rmc.date.year >= 17)
    HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_RESET);
  else
    HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_SET);
  
  if(gps_rmc.date.year >= 17)
    current_time = get_time_rmc(&gps_rmc) + 1;
  else
    current_time++;

  unix_ts_2_datetime(current_time + 3600 * utc_offset, &year, &month, &day, &hour, &minute, &second);
  if(use_24hour == 0 && hour > 12)
    hour -= 12;

  // tube display
  tube_print2(hour, &(tube_animation[5]), &(tube_animation[4]), ANIMATION_CROSS_FADE);
  tube_print2(minute, &(tube_animation[3]), &(tube_animation[2]), ANIMATION_CROSS_FADE);
  if(display_mode == DISPLAY_MODE_TIME_ONLY)
    tube_print2(second, &(tube_animation[1]), &(tube_animation[0]), ANIMATION_CROSS_FADE);
  else
    tube_print2(temp_c_int, &(tube_animation[1]), &(tube_animation[0]), ANIMATION_CROSS_FADE);
  // printf("----\n");
  // led display
  if(display_mode == DISPLAY_MODE_TIME_TEMP)
  {
    for (int i = 0; i < 2; ++i)
      led_start_animation(&(rgb_animation[i]), rgb_purple, ANIMATION_CROSS_FADE);
    for (int i = 2; i < TUBE_COUNT; ++i)
      led_start_animation(&(rgb_animation[i]), rgb_orange, ANIMATION_BREATHING);
  }
  else
  {
    for (int i = 0; i < TUBE_COUNT; ++i)
      led_start_animation(&(rgb_animation[i]), rgb_orange, ANIMATION_CROSS_FADE);
  }

  // printf("%s", sd_write_buf);
  counter++;
  if(counter % 20)
    return;

  replace_nlcr_with_space(last_rmc, GPS_BUF_SIZE);
  double temp_c_float = (double)raw_temp / (double)16;
  memset(sd_write_buf, 0, SD_WRITE_BUF_SIZE);
  sprintf(sd_write_buf, "{\"temp\": %.2f, \"date\": \"20%02d-%02d-%02d\", \"time\": \"%02d:%02d:%02d\", \"utc_offset\": %d, \"last_rmc\": \"%s\"}\n",
       temp_c_float, year, month, day, hour, minute, second, utc_offset, last_rmc);

  sd_error_count += write_log(sd_write_buf);
  // printf("sd: %d\n", sd_error_count);
}

// void my_halt(void)
// {
//   HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
//   while(1)
//   {
//     HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
//     osDelay(100);
//     HAL_IWDG_Refresh(iwdg_ptr);
//   }
// }

void gps_temp_parse_task_start(void const * argument)
{
  uint8_t loop_count = 0;
  for(;;)
  {
    if(sd_error_count > 5)
      HAL_NVIC_SystemReset();

    HAL_IWDG_Refresh(iwdg_ptr);
    if(linear_buf_line_available(&gps_lb))
    {
      memset(last_rmc, 0, GPS_BUF_SIZE);
      strncpy(last_rmc, gps_lb.buf, GPS_BUF_SIZE);
      parse_gps((char*)gps_lb.buf, &gps_rmc, &gps_gga, &gps_gsa, &gps_gll, &gps_gst, &gps_gsv);
      linear_buf_reset(&gps_lb);
    }
    if(loop_count == 0)
      ds18b20_start_conversion();
    if(loop_count == 8)
    {
      raw_temp = ds18b20_get_temp();
      temp_c_int = raw_temp >> 4;
    }
    loop_count = (loop_count + 1) % 10;
    osDelay(100);
  }
}

