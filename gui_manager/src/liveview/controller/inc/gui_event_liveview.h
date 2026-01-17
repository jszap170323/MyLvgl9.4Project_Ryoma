#ifndef _GUI_EVENT_LIVEVIEW_H
#define _GUI_EVENT_LIVEVIEW_H

enum class GUI_LiveviewEventType {
    ScreentClick,
    PhotoClick,
    StartPreview,
    StopPreview,
    StartRecord,
    StopRecord,
};

typedef struct {
    GUI_LiveviewEventType type;
    int value; // 可选
} GUI_LiveviewEvent;

#endif