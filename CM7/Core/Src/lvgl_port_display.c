/*********************
 *      INCLUDES
 *********************/

#include "lvgl_port_display.h"
#include "main.h"
#include "ltdc.h"
#include "dma2d.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void disp_flush (lv_disp_drv_t*, const lv_area_t*, lv_color_t*);
static void disp_flush_complete (DMA2D_HandleTypeDef*);

/**********************
 *  STATIC VARIABLES
 **********************/

static lv_disp_drv_t disp_drv;
static lv_disp_draw_buf_t disp_buf;
//static __attribute__((aligned(32))) lv_color_t buf_1[MY_DISP_HOR_RES * 64];
//static __attribute__((aligned(32)),(section ("TouchGFX_Framebuffer"))) lv_color_t buf_1[MY_DISP_HOR_RES * 64];
static  __attribute__((aligned(32))) /*_attribute__( (section ("LVGLBufferSection")) )*/ lv_color_t buf_1[MY_DISP_HOR_RES * 64];//__attribute__( (section ("BufferSection")) )//__attribute__((aligned(32)))

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void
lvgl_display_init (void)
{
  /* display initialization */
  ; /* display is already initialized by cubemx-generated code */

  /* display buffer initialization */
  lv_disp_draw_buf_init (&disp_buf,
                         (void*) buf_1,
                         NULL,
                         MY_DISP_HOR_RES * 64);

  /* register the display in LVGL */
  lv_disp_drv_init(&disp_drv);

  /* set the resolution of the display */
  disp_drv.hor_res = MY_DISP_HOR_RES;
  disp_drv.ver_res = MY_DISP_VER_RES;

  /* set callback for display driver */
  disp_drv.flush_cb = disp_flush;
  disp_drv.full_refresh = 0;

  /* interrupt callback for DMA2D transfer */
  hdma2d.XferCpltCallback = disp_flush_complete;

  /* set a display buffer */
  disp_drv.draw_buf = &disp_buf;

  /* finally register the driver */
  lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void
disp_flush (lv_disp_drv_t   *drv,
            const lv_area_t *area,
            lv_color_t      *color_p)
{
  lv_coord_t width = lv_area_get_width(area);
  lv_coord_t height = lv_area_get_height(area);

  DMA2D->CR = 0x0U << DMA2D_CR_MODE_Pos;
  DMA2D->FGPFCCR = DMA2D_INPUT_ARGB8888;
  DMA2D->FGMAR = (uint32_t)color_p;
  DMA2D->FGOR = 0;
  DMA2D->OPFCCR = DMA2D_OUTPUT_ARGB8888;
  DMA2D->OMAR = hltdc.LayerCfg[0].FBStartAdress + 4 * \
                (area->y1 * MY_DISP_HOR_RES + area->x1);
  DMA2D->OOR = MY_DISP_HOR_RES - width;
  DMA2D->NLR = (width << DMA2D_NLR_PL_Pos) | (height << DMA2D_NLR_NL_Pos);
  DMA2D->IFCR = 0x3FU;
  DMA2D->CR |= DMA2D_CR_TCIE;
  DMA2D->CR |= DMA2D_CR_START;
/*
  volatile int x1, x2, y1, y2;

     x1 = area->x1;
     x2 = area->x2;
     y1 = area->y1;
     y2 = area->y2;



     volatile int32_t act_x1 = x1 < 0 ? 0 : x1;
     volatile int32_t act_y1 = y1 < 0 ? 0 : y1;
     volatile int32_t act_x2 = x2-1;// > width - 1 ? width - 1 : x2;
     volatile int32_t act_y2 = y2 -1;//> height - 1 ? height - 1 : y2;

     uint32_t x;
     uint32_t y;
     long int location = 0;


     lv_color32_t *fbp32 = (lv_color32_t *)hltdc.LayerCfg[0].FBStartAdress;

     for (y = act_y1; y <= act_y1+act_y2; y++)
     {
         for (x = act_x1; x <= act_x2; x++)
         {
             location = (x) + (y) * 1024;
             fbp32[location]= *color_p;
             color_p++;
         }

         color_p += x2 - act_x2;
     }
     lv_disp_flush_ready(&disp_drv);
     */
}

static void
disp_flush_complete (DMA2D_HandleTypeDef *hdma2d)
{
  lv_disp_flush_ready(&disp_drv);
}
