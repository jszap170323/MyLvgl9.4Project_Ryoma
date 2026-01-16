#ifndef _GUI_BASE_CONTROLLER_H
#define _GUI_BASE_CONTROLLER_H
#include "gui_message_ctrl.h"
#include <memory>

class GUI_BaseController {

  public:
    GUI_BaseController(GuiMessageCtrl *msg_ctrl) : m_msg_ctrl(msg_ctrl) {}

  protected:
    GuiMessageCtrl *m_msg_ctrl;
};

#endif
