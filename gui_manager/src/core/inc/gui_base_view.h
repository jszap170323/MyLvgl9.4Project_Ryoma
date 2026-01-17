#ifndef _GUI_BASE_VIEW_H
#define _GUI_BASE_VIEW_H

// // #include <any>
// #include <functional>
// #include <lvgl.h>

// #include "gui_core_def.h"

// enum class GUI_EventType {
//     PhotoClick,
//     StartPreview,
//     StopPreview,
//     StartRecord,
//     StopRecord,
//     GetISO,
//     SetISO,
// };

// struct GUI_Event {
//     GUI_EventType type;
//     // std::any data; // 附加参数，可选
// };

// using EventCallback = std::function<void(const GUI_Event &)>;

// class GUI_BaseView {

//   public:
//     virtual void setEventCallback(EventCallback cb){};
//     virtual lv_obj_t *getRoot() { return m_root; }

//   protected:
//     lv_obj_t *m_root;
// };

// #endif

#include "gui_core_def.h"
#include <functional>
#include <lvgl.h>
#include <thread>
template <typename EventT>

class GUI_BaseView {
  public:
    using Callback = std::function<void(const EventT &)>;

    void setEventCallback(Callback cb) { callback_ = cb; }

    virtual lv_obj_t *getRoot() { return m_root; }

  protected:
    void emit(const EventT &evt) {
        if (callback_)
            callback_(evt);
    }

    lv_obj_t *m_root = nullptr;

  private:
    Callback callback_;
};
#endif