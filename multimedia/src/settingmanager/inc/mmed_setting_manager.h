#ifndef _MMED_SETTING_MANAGER_H
#define _MMED_SETTING_MANAGER_H

#include "mmed_setproperty_ctrl.h"
#include "mmed_system_setting_ctrl.h"
class MmedSettingManager {
  public:
    // using PropertyValue = std::variant<int, float, bool, std::string>;
    // using PropertyArgs = std::vector<PropertyValue>;
    // using PropertyHandler = std::function<void(const PropertyArgs &)>;

    MmedSettingManager();
    ~MmedSettingManager();

    // ⭐ 唯一对外接口
    // void setProperty(const std::string &cmd, const PropertyArgs &args = {});

    bool canHandle(uint32_t event) const;
    bool handle(uint32_t cmd, const void *data);

  private:
    MmedSystemSettingCtrl m_system_setting_ctrl;
    MmedPropertySettingCtrl m_property_setting_ctrl;
};

#endif
