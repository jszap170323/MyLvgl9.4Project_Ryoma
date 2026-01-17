#ifndef _GUI_DATAMNG_H
#define _GUI_DATAMNG_H
#include "gui_message_ctrl.h"

class GUI_DataMng {
  public:
    static GUI_DataMng *getInstance() {
        static GUI_DataMng mng;
        return &mng;
    }

    void setMsgCtrl(GuiMessageCtrl *ctrl) { m_msg_ctrl = ctrl; }

    bool sendMsgToMmed(uint32_t cmd_type, uint32_t cmd, const void *payload,
                       uint32_t payload_len);

    bool setFrameMovePixes(uint32_t offset_x);

  private:
    GUI_DataMng();
    ~GUI_DataMng();

    GUI_DataMng(const GUI_DataMng &) = delete;
    GUI_DataMng(const GUI_DataMng &&) = delete;

    GuiMessageCtrl *m_msg_ctrl;
};

#endif