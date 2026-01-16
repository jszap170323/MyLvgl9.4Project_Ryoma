#include "gui_liveview.h"

static bool startrec = true;

GUI_Liveview::GUI_Liveview() {
    m_root = lv_obj_create(lv_screen_active());

    static lv_style_t background_style;
    lv_style_set_bg_color(&background_style, lv_color_black());
    lv_style_set_bg_opa(&background_style, LV_OPA_TRANSP);
    lv_style_set_size(&background_style, 1024, 600);

    lv_obj_add_style(m_root, &background_style, 0);
    // lv_obj_remove_style_all(m_root);
    lv_obj_remove_flag(m_root, LV_OBJ_FLAG_SCROLLABLE);

    m_photo = lv_obj_create(m_root);
    lv_obj_center(m_photo);
    lv_obj_set_size(m_photo, 50, 50);

    lv_obj_add_event_cb(m_photo,
                        [](lv_event_t *e) {
                            GUI_Liveview *lv = static_cast<GUI_Liveview *>(
                                lv_event_get_user_data(e));
                            GUI_Event event;
                            event.type = GUI_EventType::PHOTO_CLICK;
                            lv->m_ctrl_callback(event);
                        },
                        LV_EVENT_CLICKED, this);

    lv_obj_t *m_rec_stop = lv_obj_create(m_root);
    lv_obj_align(m_rec_stop, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_size(m_rec_stop, 50, 50);

    lv_obj_add_event_cb(m_rec_stop,
                        [](lv_event_t *e) {
                            GUI_Liveview *lv = static_cast<GUI_Liveview *>(
                                lv_event_get_user_data(e));

                            GUI_Event event;
                            if (startrec == false) {
                                event.type = GUI_EventType::StopRecord;
                            } else {
                                event.type = GUI_EventType::StartRecord;
                            }
                            startrec = !startrec;
                            lv->m_ctrl_callback(event);
                        },
                        LV_EVENT_CLICKED, this);
}
GUI_Liveview::~GUI_Liveview() {}

void GUI_Liveview::setEventCallback(EventCallback cb) { m_ctrl_callback = cb; }
