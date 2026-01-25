#ifndef _MMED_SYSTEM_SETTING_CTRL_H
#define _MMED_SYSTEM_SETTING_CTRL_H

#include <cstdint>
#include <fcntl.h>
#include <functional>
#include <map>
#include <string>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "ipc_ui_multimedia_command.h"

class MmedSystemSettingCtrl {
  public:
    using Handler = std::function<bool(const void *)>;
    MmedSystemSettingCtrl();
    ~MmedSystemSettingCtrl();

    void addHandler(uint32_t event, Handler h) {
        m_handlers[event] = std::move(h);
    }

    void init();
    bool writeDataToFile(const std::string &path, int32_t value);

    bool setBacklightLevel(int32_t level);

    bool handle(uint32_t cmd, const void *data);

    bool canHandle(uint32_t cmd) const;

  private:
    int32_t m_backlight_level;
    std::unordered_map<uint32_t, Handler> m_handlers;
};

#endif
