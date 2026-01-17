#ifndef _GUI_EVENT_NAVIBAR_H
#define _GUI_EVENT_NAVIBAR_H

enum class GUI_NavibarEventType {
    PixesMove,
};

typedef struct {
    GUI_NavibarEventType type;
    int value; // 可选
} GUI_NavibarEvent;

#endif