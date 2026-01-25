#include "mmed_system_setting_ctrl.h"

MmedSystemSettingCtrl::MmedSystemSettingCtrl() { init(); }
MmedSystemSettingCtrl::~MmedSystemSettingCtrl() {}

void MmedSystemSettingCtrl::init() {
    addHandler(static_cast<uint32_t>(SetParamCmd::SetBackLightLevel),
               [this](const void *args) {
                   int32_t level = 7;
                   memcpy(&level, args, sizeof(int32_t));
                   return setBacklightLevel(level);
               });
}
bool MmedSystemSettingCtrl::writeDataToFile(const std::string &path,
                                            int32_t value) {
    printf("==============writeDataToFile value = %d\n", value);
    int32_t fd = open(path.c_str(), O_WRONLY);
    if (fd < 0)
        return false;

    char buf[16];
    int32_t len = snprintf(buf, sizeof(buf), "%d\n", value);

    ssize_t ret = write(fd, buf, len);
    close(fd);

    return ret == len;
}

bool MmedSystemSettingCtrl::setBacklightLevel(int32_t level) {

    bool result =
        writeDataToFile("/sys/class/backlight/backlight/brightness", level);
    return result;
}

bool MmedSystemSettingCtrl::handle(uint32_t cmd, const void *data) {

    auto it = m_handlers.find(cmd);
    if (it == m_handlers.end()) {
        return false;
    }
    return it->second(data);
}

bool MmedSystemSettingCtrl::canHandle(uint32_t cmd) const {

    auto it = m_handlers.find(cmd);
    if (it == m_handlers.end()) {
        return false;
    }

    return true;
}