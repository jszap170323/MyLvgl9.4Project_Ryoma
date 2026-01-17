#include "gui_navibar_view.h"

GUI_NaviBarView::GUI_NaviBarView(lv_obj_t *parent, int init_width)
    : parent_(parent) {

    lv_coord_t h = lv_obj_get_height(parent_);

    panel_ = lv_obj_create(parent_);
    lv_obj_set_size(panel_, panel_width_, h);
    lv_obj_align(panel_, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(panel_, lv_color_hex(0x000000), 0);

    drag_bar_ = lv_obj_create(parent_);
    lv_obj_set_size(drag_bar_, kDragBarWidth, h);
    lv_obj_align(drag_bar_, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(drag_bar_, lv_color_hex(0x888888), 0);

    lv_obj_add_event_cb(drag_bar_, drag_event_cb, LV_EVENT_ALL, this);
}

// 拖动事件回调
void GUI_NaviBarView::drag_event_cb(lv_event_t *e) {
    auto *self = static_cast<GUI_NaviBarView *>(lv_event_get_user_data(e));

    self->onDrag(e);
}

void GUI_NaviBarView::onDrag(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t *indev = lv_indev_get_act();

    lv_point_t p;
    lv_indev_get_point(indev, &p);

    if (code == LV_EVENT_PRESSED) {
        press_x_ = p.x;
        start_width_ = panel_width_;
    } else if (code == LV_EVENT_PRESSING) {
        int delta = press_x_ - p.x;
        int new_width = start_width_ + delta;

        if (new_width < kMinWidth)
            new_width = kMinWidth;
        if (new_width > kMaxWidth)
            new_width = kMaxWidth;

        if (new_width != panel_width_) {
            panel_width_ = new_width;
            applyLayout();
        }
    } else if (code == LV_EVENT_RELEASED) {
        onRelease();
    }
}

void GUI_NaviBarView::onRelease() {
    const int threshold = kMaxWidth / 3;

    if (panel_width_ >= threshold) {
        // 吸附展开
        animateTo(kMaxWidth);
    } else {
        // 吸附收紧
        animateTo(kMinWidth);
    }
}

void GUI_NaviBarView::applyLayout() {
    lv_coord_t h = lv_obj_get_height(parent_);
    printf("panel_width_ = %d\n", panel_width_);

    lv_obj_set_size(panel_, panel_width_, h);
    lv_obj_align(panel_, LV_ALIGN_RIGHT_MID, 0, 0);

    // 拖动条始终贴在 panel 左边
    lv_obj_set_x(drag_bar_, -panel_width_);
    GUI_NavibarEvent event;
    event.type = GUI_NavibarEventType::PixesMove;
    event.value = panel_width_;
    emit(event);
}

void GUI_NaviBarView::animateTo(int target_width) {
    lv_anim_t a;
    lv_anim_init(&a);

    lv_anim_set_var(&a, this);
    lv_anim_set_exec_cb(&a, [](void *var, int32_t v) {
        auto *self = static_cast<GUI_NaviBarView *>(var);
        self->panel_width_ = v;
        self->applyLayout();
    });

    lv_anim_set_values(&a, panel_width_, target_width);
    lv_anim_set_time(&a, 150); // 150ms 手感很好
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);

    lv_anim_start(&a);
}

bool GUI_NaviBarView::isNavibarfolded() { return panel_width_ == kMinWidth; }

void GUI_NaviBarView::foldNavibar() { animateTo(kMinWidth); }