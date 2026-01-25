#include <gst/gst.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "mmed_controller.h"
#include "mmed_message_ctrl.h"
#include "mmed_setting_manager.h"
#include "mmed_statematchine.h"

int main() {

    const gchar *nano_str;
    guint major, minor, micro, nano;

    gst_init(NULL, NULL);

    gst_version(&major, &minor, &micro, &nano);

    if (nano == 1)
        nano_str = "(CVS)";
    else if (nano == 2)
        nano_str = "(Prerelease)";
    else
        nano_str = "";

    printf("This program is linked against GStreamer %d.%d.%d %s\n", major,
           minor, micro, nano_str);

    // MmedSettingManager setting;
    // MmedPipelineManager pipeline;
    // MmedCtroller mmed_controller(&pipeline);
    // pipeline.start();
    // mmed_controller.start();
    // MmedStateMatchine state_matchine(pipeline, setting);
    // MmedMessageCtrl msg_ctrl(&mmed_controller);
    // msg_ctrl.start();

    MmedPipelineManager pipeline;
    MmedSettingManager setting;
    MmedStateMatchine fsm(pipeline, setting);
    MmedMessageCtrl msg_ctrl(fsm);

    fsm.initlize();   // FSM ready
    pipeline.start(); // 开始干活
    msg_ctrl.start(); // 开始收 UI

    while (1) {
        usleep(1000);
    }
    return 0;
}
