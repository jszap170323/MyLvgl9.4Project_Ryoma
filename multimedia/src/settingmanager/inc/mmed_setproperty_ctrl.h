#ifndef _MMED_SETPROPERTY_CTRL_H
#define _MMED_SETPROPERTY_CTRL_H

#include <functional>
#include <map>
#include <string>
#include <vector>

class MmedPropertySettingCtrl {
  public:
    // using PropertyValue = std::variant<int, float, bool, std::string>;
    // using PropertyArgs = std::vector<PropertyValue>;
    // using PropertyHandler = std::function<void(const PropertyArgs &)>;

    MmedPropertySettingCtrl();
    ~MmedPropertySettingCtrl();

    // ⭐ 唯一对外接口
    // void setProperty(const std::string &cmd, const PropertyArgs &args = {});

  private:
};

#endif
