#ifndef _GUI_BASE_VIEW_H
#define _GUI_BASE_VIEW_H

// #include <any>
#include <functional>

enum class GUI_EventType {
    PHOTO_CLICK,
    START_PREVIEW,
    STOP_PREVIEW,
    StartRecord,
    StopRecord,
    GET_ISO,
    SET_ISO,
    // ... 可以无限扩展
};

struct GUI_Event {
    GUI_EventType type;
    // std::any data; // 附加参数，可选
};

using EventCallback = std::function<void(const GUI_Event &)>;

class GUI_BaseView {

  public:
    virtual void setEventCallback(EventCallback cb){};
};

#endif
