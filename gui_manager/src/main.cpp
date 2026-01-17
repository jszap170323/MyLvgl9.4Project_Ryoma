

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
#include <atomic>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>

#include "lvgl/demos/lv_demos.h"
#include "lvgl/lvgl.h"

#include "gui_datamng.h"
#include "gui_message_ctrl.h"
#include "ipc_shared_memory.h"
#include "liveview_controller.h"
#include "lvgl/src/drivers/evdev/lv_evdev.h"
#include "src/lib/driver_backends.h"
#include "src/lib/simulator_settings.h"
#include "src/lib/simulator_util.h"

/* Internal functions */
static void configure_simulator(int argc, char **argv);
static void print_lvgl_version(void);
static void print_usage(void);

/* contains the name of the selected backend if user
 * has specified one on the command line */
static char *selected_backend;

/* Global simulator settings, defined in lv_linux_backend.c */
extern simulator_settings_t settings;

// ============ 全局变量 ============
static sem_t frame_sem;                 // 帧同步信号量
static sem_t draw_sem;                  // 绘制同步信号量
static uint8_t *video_buf = nullptr;    // 视频缓冲区
static uint8_t *display_buf = nullptr;  // 显示缓冲区（双缓冲）
static lv_obj_t *canvas = nullptr;      // Canvas对象
static std::atomic<bool> running{true}; // 运行标志
static std::atomic<int> fps_counter{0}; // FPS计数器

int W = 1024;
int H = 600;

/**
 * @brief Print LVGL version
 */
static void print_lvgl_version(void) {
    fprintf(stdout, "%d.%d.%d-%s\n", LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR,
            LVGL_VERSION_PATCH, LVGL_VERSION_INFO);
}

/**
 * @brief Print usage information
 */
static void print_usage(void) {
    fprintf(stdout, "\nlvglsim [-V] [-B] [-b backend_name] [-W window_width] "
                    "[-H window_height]\n\n");
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
static void configure_simulator(int argc, char **argv) {
    int opt = 0;

    selected_backend = NULL;
    driver_backends_register();

    const char *env_w = getenv("LV_SIM_WINDOW_WIDTH");
    const char *env_h = getenv("LV_SIM_WINDOW_HEIGHT");
    /* Default values */
    settings.window_width = atoi(env_w ? env_w : "800");
    settings.window_height = atoi(env_h ? env_h : "480");

    /* Parse the command-line options. */
    while ((opt = getopt(argc, argv, "b:fmW:H:BVh")) != -1) {
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

// ============ 信号处理 ============
#include <signal.h>
static void signal_handler(int sig) {
    printf("Received signal %d, shutting down...\n", sig);
    exit(0);
}

/**
 * @brief entry point
 * @description start a demo
 * @param argc the count of arguments in argv
 * @param argv The arguments
 */
int main(int argc, char **argv) {
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    configure_simulator(argc, argv);

    /* Initialize LVGL. */
    lv_init();

    /* Initialize the configured backend */
    if (driver_backends_init_backend(selected_backend) == -1) {
        die("Failed to initialize display backend");
    }

/* Enable for EVDEV support */
#if LV_USE_EVDEV
    printf("Initializing EVDEV...\n");
    lv_display_t *disp = lv_display_get_default();
    LV_ASSERT(disp);

    lv_indev_t *indev =
        lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event1");

    lv_indev_set_display(indev, disp);
#endif

    GuiMessageCtrl msg_ctrl;
    msg_ctrl.start();
    GUI_DataMng::getInstance()->setMsgCtrl(&msg_ctrl);
    GUI_LiveviewController lv_controller;

    // ============ 进入主循环 ============
    printf("Entering main loop...\n");

    /* Enter the run loop of the selected backend */
    driver_backends_run_loop();

    return 0;
}
