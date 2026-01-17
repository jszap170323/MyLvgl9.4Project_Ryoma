#ifndef _GUI_NAVIBAR_PANEL_H
#define _GUI_NAVIBAR_PANEL_H
#include "gui_base_view.h"
#include "gui_event_navibar.h"

#include "lvgl.h"
class GUI_NaviBarView : public GUI_BaseView<GUI_NavibarEvent> {
  public:
    GUI_NaviBarView(lv_obj_t *parent, int init_width = 100);
    ~GUI_NaviBarView() = default;

    // 获取当前宽度
    int getWidth() const { return panel_width_; }
    bool isNavibarfolded();
    void foldNavibar();

  private:
    lv_obj_t *parent_;
    lv_obj_t *panel_;
    lv_obj_t *drag_bar_;

    int panel_width_ = 0;

    int press_x_ = 0;
    int start_width_ = 0;

    static constexpr int kMaxWidth = 150;
    static constexpr int kMinWidth = 0;
    static constexpr int kDragBarWidth = 20;

    static void drag_event_cb(lv_event_t *e);
    void onDrag(lv_event_t *e);
    void onRelease();
    void applyLayout();
    void animateTo(int target_width);
};

#endif
