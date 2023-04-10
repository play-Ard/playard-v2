#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "lvgl.h"
#include "main.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
//
//#include "images/menu.c"

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<T_DISPLAY_S3_PIN_NUM_BK_LIGHT) | (1ULL<<T_DISPLAY_S3_PIN_RD) | (1ULL<<T_DISPLAY_S3_PIN_PWR))

// LVGL image declare
LV_IMG_DECLARE(menu)
LV_IMG_DECLARE(playard_logo)

static lv_obj_t *img_menu;
static lv_obj_t *img_logo;


static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
static lv_disp_drv_t disp_drv;      // contains callback functions
esp_lcd_panel_dev_config_t panel_config;
esp_lcd_panel_handle_t panel_handle = NULL;

static bool T_DISPLAY_S3_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void T_DISPLAY_S3_example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{

    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);

}

static void T_DISPLAY_S3_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(T_DISPLAY_S3_LVGL_TICK_PERIOD_MS);
}

void gpio_init(){
	// BACKLIGHT GPIO CONFIG
	gpio_config_t bk_gpio_config = {
		 .mode = GPIO_MODE_OUTPUT,
		 .pin_bit_mask = GPIO_OUTPUT_PIN_SEL
		};

		ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

	    gpio_set_level(T_DISPLAY_S3_PIN_RD, true);
	    gpio_set_level(T_DISPLAY_S3_PIN_NUM_BK_LIGHT, T_DISPLAY_S3_LCD_BK_LIGHT_ON_LEVEL);

	    ESP_LOGI(TAG, "Initialize Intel 8080 bus");
	    esp_lcd_i80_bus_handle_t i80_bus = NULL;
	    esp_lcd_i80_bus_config_t bus_config = {
	      .dc_gpio_num = T_DISPLAY_S3_PIN_NUM_DC,
	      .wr_gpio_num = T_DISPLAY_S3_PIN_NUM_PCLK,
	      .clk_src	= LCD_CLK_SRC_PLL160M,
	      .data_gpio_nums = {
	                    T_DISPLAY_S3_PIN_NUM_DATA0,
	                    T_DISPLAY_S3_PIN_NUM_DATA1,
	                    T_DISPLAY_S3_PIN_NUM_DATA2,
	                    T_DISPLAY_S3_PIN_NUM_DATA3,
	                    T_DISPLAY_S3_PIN_NUM_DATA4,
	                    T_DISPLAY_S3_PIN_NUM_DATA5,
	                    T_DISPLAY_S3_PIN_NUM_DATA6,
	                    T_DISPLAY_S3_PIN_NUM_DATA7,
	                },
	                .bus_width = 8,
	                .max_transfer_bytes = (T_DISPLAY_S3_LCD_H_RES * T_DISPLAY_S3_LCD_V_RES) * 40 * sizeof(uint16_t)
	            };

	        ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

	        esp_lcd_panel_io_handle_t io_handle = NULL;

	        esp_lcd_panel_io_i80_config_t io_config = {
	             .cs_gpio_num = T_DISPLAY_S3_PIN_NUM_CS,
	             .pclk_hz = T_DISPLAY_S3_LCD_PIXEL_CLOCK_HZ,
	             .trans_queue_depth = 20,
	             .dc_levels = {
							  .dc_idle_level = 0,
							  .dc_cmd_level = 0,
							  .dc_dummy_level = 0,
							  .dc_data_level = 1,
	                    },
	              .on_color_trans_done = T_DISPLAY_S3_notify_lvgl_flush_ready,
	              .user_ctx = &disp_drv,
	              .lcd_cmd_bits = T_DISPLAY_S3_LCD_CMD_BITS,
	             .lcd_param_bits = T_DISPLAY_S3_LCD_PARAM_BITS,
	     };
	       ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));
	       ESP_LOGI(TAG, "Install LCD driver of st7789");

	       panel_config.reset_gpio_num = T_DISPLAY_S3_PIN_NUM_RST;
	       panel_config.color_space = ESP_LCD_COLOR_SPACE_RGB;
	       panel_config.bits_per_pixel = 16;

	       ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

	       esp_lcd_panel_reset(panel_handle);
	       esp_lcd_panel_init(panel_handle);
	       esp_lcd_panel_invert_color(panel_handle, true);

	       esp_lcd_panel_swap_xy(panel_handle, true);
	       esp_lcd_panel_mirror(panel_handle, false, true);
	                  // the gap is LCD panel specific, even panels with the same driver IC, can have different gap value
	       esp_lcd_panel_set_gap(panel_handle, 0, 35);

	       ESP_LOGI(TAG, "Turn on LCD backlight");
	       gpio_set_level(T_DISPLAY_S3_PIN_PWR, true);
	       gpio_set_level(T_DISPLAY_S3_PIN_NUM_BK_LIGHT, T_DISPLAY_S3_LCD_BK_LIGHT_ON_LEVEL);

	       ESP_LOGI(TAG, "Initialize LVGL library");
}

void app_main(void)
{
	gpio_init();

    lv_init();

    lv_color_t *buf1 = heap_caps_malloc(T_DISPLAY_S3_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    lv_color_t *buf2 = heap_caps_malloc(T_DISPLAY_S3_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);
     // initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, T_DISPLAY_S3_LCD_H_RES * 20);

    ESP_LOGI(TAG, "Register display driver to LVGL");


    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = T_DISPLAY_S3_LCD_H_RES;
    disp_drv.ver_res = T_DISPLAY_S3_LCD_V_RES;
    disp_drv.flush_cb = T_DISPLAY_S3_example_lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;

    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Install LVGL tick timer");
         // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
     const esp_timer_create_args_t lvgl_tick_timer_args = {
         .callback = &T_DISPLAY_S3_increase_lvgl_tick,
         .name = "lvgl_tick"
     };
     esp_timer_handle_t lvgl_tick_timer = NULL;
     ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
     ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

     ESP_LOGI(TAG, "Display LVGL animation");

     // LVGL UI //

     lv_obj_t *scr = lv_disp_get_scr_act(disp);


     img_menu = lv_img_create(scr);
     lv_img_set_src(img_menu, &menu);
     lv_obj_center(img_menu);

     lv_obj_t * obj1;
     obj1 = lv_obj_create(lv_scr_act());
     lv_obj_set_size(obj1, 150, 40);

     lv_obj_t * label1 = lv_label_create(lv_scr_act());
     lv_label_set_text(label1, "#0000ff Re-color# #ff00ff words# #ff0000 of a# label "
                                   "and  wrap long text automatically.");
     lv_obj_set_width(label1, 150);


     // LVGL UI END //


//lv_obj_set_style(obj1, &lv_style_plain_color);
// lv_obj_align(obj1, NULL, LV_ALIGN_IN_TOP_MID, 0, 40);
	while (true) {
		vTaskDelay(pdMS_TO_TICKS(10));
		lv_timer_handler();
    }
}
