

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

// ============ Canvas 初始化 ============
void video_canvas_init(void) {
    size_t buf_size = LV_CANVAS_BUF_SIZE(W, H, 16, LV_DRAW_BUF_STRIDE_ALIGN);
    printf("Allocating canvas buffer: %zu bytes (%.2f MB)\n", buf_size,
           buf_size / (1024.0 * 1024.0));

    // 分配视频缓冲区
    video_buf = new uint8_t[buf_size];
    memset(video_buf, 0, buf_size);

    // 分配显示缓冲区（双缓冲）
    display_buf = new uint8_t[buf_size];
    memset(display_buf, 0, buf_size);

    // 创建 Canvas
    canvas = lv_canvas_create(lv_screen_active());

    // 使用显示缓冲区
    lv_canvas_set_buffer(canvas, display_buf, W, H, LV_COLOR_FORMAT_RGB565);

    lv_obj_set_size(canvas, W, H);
    lv_obj_center(canvas);

    // 设置 Canvas 背景透明
    lv_obj_set_style_bg_opa(canvas, LV_OPA_TRANSP, 0);

    printf("Canvas initialized successfully\n");
}

// ============ 视频线程函数 ============
void video_thread_func() {
    printf("Video thread started\n");

    IpcSharedMemory shm("/VideoFrameMemory", W * H * 2, false);

    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);

    while (running) {
        // 从共享内存读取视频帧
        shm.read(video_buf, W * H * 2);

        // 通知主线程有新帧可用
        sem_post(&frame_sem);

        // 等待主线程完成绘制
        sem_wait(&draw_sem);

        // 控制帧率（~60 FPS）
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        long elapsed_ns =
            (current_time.tv_sec - last_time.tv_sec) * 1000000000L +
            (current_time.tv_nsec - last_time.tv_nsec);

        long target_ns = 16666666L; // 60 FPS
        if (elapsed_ns < target_ns) {
            long sleep_ns = target_ns - elapsed_ns;
            struct timespec sleep_time = {.tv_sec = sleep_ns / 1000000000L,
                                          .tv_nsec = sleep_ns % 1000000000L};
            nanosleep(&sleep_time, NULL);
        }

        clock_gettime(CLOCK_MONOTONIC, &last_time);
    }

    printf("Video thread finished\n");
}

// ============ 帧复制函数（使用双缓冲避免撕裂） ============
void copy_video_frame() {
    static int frame_count = 0;
    frame_count++;

    // 每100帧打印一次FPS
    if (frame_count % 100 == 0) {
        static struct timespec last_fps_time;
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        if (frame_count == 100) {
            last_fps_time = now;
        } else {
            double elapsed = (now.tv_sec - last_fps_time.tv_sec) +
                             (now.tv_nsec - last_fps_time.tv_nsec) / 1e9;
            double fps = 100.0 / elapsed;
            printf("Video FPS: %.2f\n", fps);
            last_fps_time = now;
        }
    }

    // 复制视频帧到显示缓冲区
    memcpy(display_buf, video_buf, W * H * 2);
}

// ============ LVGL 定时器回调（用于刷新 Canvas） ============
static void refresh_timer_cb(lv_timer_t *timer) {
    static int sem_value;
    static int timeout_count = 0;

    // 尝试等待新帧（非阻塞）
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_nsec += 10000000; // 10ms 超时

    if (timeout.tv_nsec >= 1000000000L) {
        timeout.tv_sec += 1;
        timeout.tv_nsec -= 1000000000L;
    }

    int ret = sem_timedwait(&frame_sem, &timeout);

    if (ret == 0) {
        // 有新帧，复制到显示缓冲区
        copy_video_frame();

        // 标记 Canvas 需要刷新
        lv_obj_invalidate(canvas);

        // 通知视频线程可以继续
        sem_post(&draw_sem);

        timeout_count = 0;

        // 更新 FPS 计数器
        fps_counter++;
    } else if (errno == ETIMEDOUT) {
        // 超时，没有新帧
        timeout_count++;
        if (timeout_count > 100) {
            // 长时间没有新帧，强制刷新一次（防止卡死）
            lv_obj_invalidate(canvas);
            timeout_count = 0;
        }
    }

    // 每秒打印一次 UI FPS
    static uint32_t last_fps_time = 0;
    uint32_t current_time = lv_tick_get();
    if (current_time - last_fps_time > 1000) {
        // printf("UI FPS: %d\n", fps_counter.load());
        fps_counter = 0;
        last_fps_time = current_time;
    }
}

// ============ 添加 UI 控件 ============
void create_ui_overlay() {
    // 创建半透明背景层
    lv_obj_t *overlay = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(overlay);
    lv_obj_set_size(overlay, W, H);
    lv_obj_set_style_bg_color(overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(overlay, LV_OPA_TRANSP, 0);
    lv_obj_set_style_outline_opa(overlay, LV_OPA_TRANSP, 0);

    // 创建控制面板
    lv_obj_t *panel = lv_obj_create(overlay);
    lv_obj_set_size(panel, 300, 200);
    lv_obj_align(panel, LV_ALIGN_TOP_RIGHT, -20, 20);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_80, 0);
    lv_obj_set_style_radius(panel, 10, 0);

    // 添加标题
    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title, "Video Overlay Control");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);

    // 添加 FPS 显示
    static lv_obj_t *fps_label;
    fps_label = lv_label_create(panel);
    lv_label_set_text(fps_label, "FPS: --");
    lv_obj_align(fps_label, LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_set_style_text_color(fps_label, lv_color_hex(0x00FF00), 0);

    // 添加控制按钮
    lv_obj_t *btn = lv_btn_create(panel);
    lv_obj_set_size(btn, 100, 40);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x007ACC), 0);

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Control");
    lv_obj_center(btn_label);

    // 创建更新 FPS 的定时器
    lv_timer_t *fps_timer = lv_timer_create(
        [](lv_timer_t *timer) {
            static int frame_count = 0;
            static uint32_t last_time = 0;

            frame_count++;
            uint32_t current_time = lv_tick_get();

            if (current_time - last_time >= 1000) {
                char buf[32];
                snprintf(buf, sizeof(buf), "FPS: %d", frame_count);
                lv_label_set_text(fps_label, buf);
                frame_count = 0;
                last_time = current_time;
            }
        },
        1000, NULL);
}

// ============ 清理函数 ============
void cleanup() {
    printf("Cleaning up...\n");

    running = false;

    // 销毁信号量
    sem_destroy(&frame_sem);
    sem_destroy(&draw_sem);

    // 释放内存
    if (video_buf) {
        delete[] video_buf;
        video_buf = nullptr;
    }

    if (display_buf) {
        delete[] display_buf;
        display_buf = nullptr;
    }

    printf("Cleanup completed\n");
}

// ============ 信号处理 ============
#include <signal.h>
static void signal_handler(int sig) {
    printf("Received signal %d, shutting down...\n", sig);
    cleanup();
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
    fprintf(stdout, "======================evdev\n");

#if LV_USE_EVDEV
    printf("Initializing EVDEV...\n");
    lv_display_t *disp = lv_display_get_default();
    LV_ASSERT(disp);

    lv_indev_t *indev =
        lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event1");

    lv_indev_set_display(indev, disp);
    printf("EVDEV initialized\n");
#endif

    printf("444444444444444444444444444444444\n");

    // ============ 初始化信号量 ============
    sem_init(&frame_sem, 0, 0); // 初始值为0，等待视频帧
    sem_init(&draw_sem, 0, 0);  // 初始值为0，等待绘制完成

    printf("aaaaaaaaaaaaaaaaaaaaaaaaa\n");

    // ============ 初始化 Canvas ============
    video_canvas_init();

    printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

    // ============ 创建 UI 叠加层 ============
    // create_ui_overlay();

    // ============ 创建刷新定时器 ============
    lv_timer_t *refresh_timer =
        lv_timer_create(refresh_timer_cb, 16, NULL); // ~60Hz

    // ============ 启动视频线程 ============
    std::thread video_thread(video_thread_func);

    // 设置线程名字（便于调试）
    pthread_setname_np(video_thread.native_handle(), "video_thread");

    printf("Video thread started\n");

    GuiMessageCtrl msg_ctrl;
    msg_ctrl.start();
    GUI_LiveviewController lv_controller(&msg_ctrl);

    // ============ 进入主循环 ============
    printf("Entering main loop...\n");

    /* Enter the run loop of the selected backend */
    driver_backends_run_loop();

    // ============ 清理 ============
    cleanup();

    // 等待视频线程结束
    if (video_thread.joinable()) {
        video_thread.join();
    }

    return 0;
}
