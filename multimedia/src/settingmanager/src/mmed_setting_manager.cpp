#include "mmed_setting_manager.h"

MmedSettingManager::MmedSettingManager() {}
MmedSettingManager::~MmedSettingManager() {}

bool MmedSettingManager::canHandle(uint32_t cmd) const {
    // // system setting: 0x3000 - 0x3FFF（你自己定）
    // if (cmd >= 0x3000 && cmd < 0x4000)
    //     return true;

    // // property setting
    // if (cmd >= 0x1000 && cmd < 0x3000)
    //     return true;

    if (m_system_setting_ctrl.canHandle(cmd)) {
        return true;
    }

    return false;
}

bool MmedSettingManager::handle(uint32_t cmd, const void *data) {
    if (m_system_setting_ctrl.canHandle(cmd)) {
        return m_system_setting_ctrl.handle(cmd, data);
    }

    // if (cmd >= 0x1000 && cmd < 0x3000) {
    //     return m_property_setting_ctrl.handle(cmd, data);
    // }

    return false;
}
