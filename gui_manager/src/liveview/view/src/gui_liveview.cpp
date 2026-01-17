#include "gui_liveview.h"

static bool startrec = true;

GUI_Liveview::GUI_Liveview() {
    m_root = lv_obj_create(NULL);

    static lv_style_t background_style;
    lv_style_set_bg_color(&background_style, lv_color_black());
    lv_style_set_bg_opa(&background_style, LV_OPA_TRANSP);
    lv_style_set_size(&background_style, 1024, 600);

    lv_obj_add_style(m_root, &background_style, 0);
    // lv_obj_remove_style_all(m_root);
    // lv_obj_remove_flag(m_root, LV_OBJ_FLAG_SCROLLABLE);

    m_video_frame_canvas = std::make_unique<GUI_VideoFrameCanvas>(m_root);

    m_photo = lv_obj_create(m_root);
    lv_obj_center(m_photo);
    lv_obj_set_size(m_photo, 50, 50);

    lv_obj_add_event_cb(m_photo,
                        [](lv_event_t *e) {
                            GUI_Liveview *lv = static_cast<GUI_Liveview *>(
                                lv_event_get_user_data(e));
                            GUI_LiveviewEvent event;
                            event.type = GUI_LiveviewEventType::PhotoClick;
                            lv->emit(event);
                        },
                        LV_EVENT_CLICKED, this);

    lv_obj_t *m_rec_stop = lv_obj_create(m_root);
    lv_obj_align(m_rec_stop, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_size(m_rec_stop, 50, 50);

    lv_obj_add_event_cb(m_rec_stop,
                        [](lv_event_t *e) {
                            GUI_Liveview *lv = static_cast<GUI_Liveview *>(
                                lv_event_get_user_data(e));

                            GUI_LiveviewEvent event;
                            if (startrec == false) {
                                event.type = GUI_LiveviewEventType::StopRecord;
                            } else {
                                event.type = GUI_LiveviewEventType::StartRecord;
                            }
                            startrec = !startrec;

                            lv->emit(event);
                        },
                        LV_EVENT_CLICKED, this);

    lv_obj_add_event_cb(m_root,
                        [](lv_event_t *e) {
                            GUI_Liveview *lv = static_cast<GUI_Liveview *>(
                                lv_event_get_user_data(e));
                            lv->handleUIEvent(e);

                        },
                        LV_EVENT_ALL, this);

    lv_screen_load(m_root);
}
GUI_Liveview::~GUI_Liveview() {}

// void GUI_Liveview::setEventCallback(EventCallback cb) { m_ctrl_callback = cb;
// }

void GUI_Liveview::handleUIEvent(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    // printf("=================\n");
    switch (code) {
    case LV_EVENT_GESTURE:
        printf("=================\n");
        break;

    case LV_EVENT_CLICKED: {
        GUI_LiveviewEvent event;
        event.type = GUI_LiveviewEventType::ScreentClick;
        emit(event);
        break;
    }

    default:
        break;
    }
}
