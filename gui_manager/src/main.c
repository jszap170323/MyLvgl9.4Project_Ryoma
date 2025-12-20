/*******************************************************************
 *
 * main.c - LVGL simulator for GNU/Linux
 *
 * Based on the original file from the repository
 *
 * @note eventually this file won't contain a main function and will
 * become a library supporting all major operating systems
 *
 * To see how each driver is initialized check the
 * 'src/lib/display_backends' directory
 *
 * - Clean up
 * - Support for multiple backends at once
 *   2025 EDGEMTech Ltd.
 *
 * Author: EDGEMTech Ltd, Erik Tagirov (erik.tagirov@edgemtech.ch)
 *
 ******************************************************************/
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"

#include "src/lib/driver_backends.h"
#include "src/lib/simulator_util.h"
#include "src/lib/simulator_settings.h"
#include "lvgl/src/drivers/evdev/lv_evdev.h"


/* Internal functions */
static void configure_simulator(int argc, char **argv);
static void print_lvgl_version(void);
static void print_usage(void);

/* contains the name of the selected backend if user
 * has specified one on the command line */
static char *selected_backend;

/* Global simulator settings, defined in lv_linux_backend.c */
extern simulator_settings_t settings;


/**
 * @brief Print LVGL version
 */
static void print_lvgl_version(void)
{
    fprintf(stdout, "%d.%d.%d-%s\n",
            LVGL_VERSION_MAJOR,
            LVGL_VERSION_MINOR,
            LVGL_VERSION_PATCH,
            LVGL_VERSION_INFO);
}

/**
 * @brief Print usage information
 */
static void print_usage(void)
{
    fprintf(stdout, "\nlvglsim [-V] [-B] [-b backend_name] [-W window_width] [-H window_height]\n\n");
    fprintf(stdout, "-V print LVGL version\n");
    fprintf(stdout, "-B list supported backends\n");
}

/**
 * @brief Configure simulator
 * @description process arguments recieved by the program to select
 * appropriate options
 * @param argc the count of arguments in argv
 * @param argv The arguments
 */
static void configure_simulator(int argc, char **argv)
{
    int opt = 0;

    selected_backend = NULL;
    driver_backends_register();

    const char *env_w = getenv("LV_SIM_WINDOW_WIDTH");
    const char *env_h = getenv("LV_SIM_WINDOW_HEIGHT");
    /* Default values */
    settings.window_width = atoi(env_w ? env_w : "800");
    settings.window_height = atoi(env_h ? env_h : "480");

    /* Parse the command-line options. */
    while ((opt = getopt (argc, argv, "b:fmW:H:BVh")) != -1) {
        switch (opt) {
        case 'h':
            print_usage();
            exit(EXIT_SUCCESS);
            break;
        case 'V':
            print_lvgl_version();
            exit(EXIT_SUCCESS);
            break;
        case 'B':
            driver_backends_print_supported();
            exit(EXIT_SUCCESS);
            break;
        case 'b':
            if (driver_backends_is_supported(optarg) == 0) {
                die("error no such backend: %s\n", optarg);
            }
            selected_backend = strdup(optarg);
            break;
        case 'W':
            settings.window_width = atoi(optarg);
            break;
        case 'H':
            settings.window_height = atoi(optarg);
            break;
        case ':':
            print_usage();
            die("Option -%c requires an argument.\n", optopt);
            break;
        case '?':
            print_usage();
            die("Unknown option -%c.\n", optopt);
        }
    }
}

static uint8_t *video_buf;
static lv_obj_t *canvas;
int W = 1024;
int H = 600;
void video_canvas_init(void)
{
    video_buf = malloc(
        LV_CANVAS_BUF_SIZE(W, H, 16, LV_DRAW_BUF_STRIDE_ALIGN));

    canvas = lv_canvas_create(lv_screen_active());
    lv_canvas_set_buffer(canvas,
                         video_buf,
                         W,
                         H,
                         LV_COLOR_FORMAT_RGB565);

    lv_obj_set_size(canvas, W, H);
    lv_obj_center(canvas);
}


/**
 * @brief entry point
 * @description start a demo
 * @param argc the count of arguments in argv
 * @param argv The arguments
 */
int main(int argc, char **argv)
{

    configure_simulator(argc, argv);

    /* Initialize LVGL. */
    lv_init();

    /* Initialize the configured backend */
    if (driver_backends_init_backend(selected_backend) == -1) {
        die("Failed to initialize display backend");
    }

    /* Enable for EVDEV support */
	fprintf(stdout, "======================evdev\n");
/*lv_indev_t * indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event1");*/
/*#if LV_USE_EVDEV
	fprintf(stdout, "======================evdev\n");
    if (driver_backends_init_backend("EVDEV") == -1) {
        die("Failed to initialize evdev");
	fprintf(stdout, "=========failer=============evdev\n");
    }
#endif*/

/* 手动 evdev（唯一正确方式） */
#if LV_USE_EVDEV
lv_display_t * disp = lv_display_get_default();
LV_ASSERT(disp);

lv_indev_t * indev =
    lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event1");

lv_indev_set_display(indev, disp);
#endif



    /*Create a Demo*/
//    lv_demo_widgets();
//lv_obj_t * root = lv_obj_create(lv_screen_active());
// lv_obj_remove_style_all(root);
//    lv_obj_set_size(root, 1024, 600);
//    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
//lv_obj_set_style_bg_opa(root, LV_OPA_50,0);

video_canvas_init();
memset(video_buf, 0, 1024*600*2);
lv_obj_invalidate(canvas);
//lv_obj_t * l1 = lv_obj_create(root);
//lv_obj_center(l1);
//lv_obj_set_size(l1,80,80);

 // lv_obj_set_style_bg_color(l1, lv_color_hex(0x00ff00), 0);

//lv_obj_t* image = lv_image_create(root);
//lv_obj_center(image);

//lv_obj_set_size(image, 80,80);
//lv_image_set_src(image, "/data/test.svg");
//lv_image_set_inner_align(image,LV_IMAGE_ALIGN_CONTAIN);

//lv_style_set_border_width(image, 4);
//    lv_style_set_border_color(image, lv_color_black());
//lv_obj_t * btn = lv_obj_create(root);
//lv_obj_set_size(btn, 80,80);
//  lv_obj_set_style_bg_color(root, lv_color_white(), 0);

//lv_disp_load_scr(root);
  
//    lv_demo_widgets_start_slideshow();

    /* Enter the run loop of the selected backend */
    driver_backends_run_loop();

    return 0;
}
