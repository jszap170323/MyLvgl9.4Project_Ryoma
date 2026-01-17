#ifndef _GUI_EVENT_FRAME_H
#define _GUI_EVENT_FRAME_H

enum class GUI_FrameEventType {
    ScreentClick,
    PhotoClick,
    StartPreview,
    StopPreview,
    StartRecord,
    StopRecord,
};

typedef struct {
    GUI_FrameEventType type;
    int value; // 可选
} GUI_FrameEvent;

#endif