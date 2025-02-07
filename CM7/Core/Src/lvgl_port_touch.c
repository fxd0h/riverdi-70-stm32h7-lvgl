/*********************
 *      INCLUDES
 *********************/

#include "lvgl_port_touch.h"
#include "main.h"
#include "i2c.h"

/**********************
 *  STATIC VARIABLES
 **********************/

static volatile uint8_t touch_irq = 0;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void
lvgl_touchscreen_read (lv_indev_drv_t *indev, lv_indev_data_t *data);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void
lvgl_touchscreen_init (void)
{
  static lv_indev_drv_t indev_drv;

  /* 'i2c1' bus and touchscreen reset pin are already configure by CubeMX,
   *  here we just need to reset touchscreen controller */
  HAL_GPIO_WritePin(CTP_RST_GPIO_Port, CTP_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(CTP_RST_GPIO_Port, CTP_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(CTP_RST_GPIO_Port, CTP_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(10);

  /* basic LVGL driver initialization */
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = lvgl_touchscreen_read;

  /* register the driver in LVGL */
  lv_indev_drv_register(&indev_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void
lvgl_touchscreen_read (lv_indev_drv_t  *indev,
                       lv_indev_data_t *data)
{
  static lv_coord_t last_x = 0;
  static lv_coord_t last_y = 0;

  /* save the pressed coordinates and the state*/
  if(touch_irq)
    {
      HAL_StatusTypeDef status;
      uint8_t rx_buf[16] = {0};

      /* reset interrupt flag */
      touch_irq = 0;

      /* read x/y coordinates */
      status = HAL_I2C_Mem_Read(&hi2c1, (0x41 << 1), 0x10, 1, rx_buf, sizeof(rx_buf), 100);
      if (status == HAL_OK)
        {
          last_x = (rx_buf[3] & 0x0F) << 8 | rx_buf[2];
          last_y = (rx_buf[5] & 0x0F) << 8 | rx_buf[4];
          data->state = LV_INDEV_STATE_PRESSED;
        }
	  else
        {
          data->state = LV_INDEV_STATE_RELEASED;
        }
    }
  else
    {
      data->state = LV_INDEV_STATE_RELEASED;
    }

  /* set the last pressed coordinates */
  data->point.x = last_x;
  data->point.y = last_y;
}

void
HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == CTP_INT_Pin)
    touch_irq = 1;

}
