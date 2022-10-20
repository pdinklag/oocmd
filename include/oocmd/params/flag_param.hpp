#ifndef _OOCMD_FLAG_PARAM_HPP
#define _OOCMD_FLAG_PARAM_HPP

#include <oocmd/params/value_param.hpp>
#include <oocmd/util/bool_string.hpp>

namespace oocmd {

class FlagParam : public ValueParam<bool> {
public:
    using ValueParam::ValueParam;

    inline bool configure(nlohmann::json const& json) const override {
        if(json.contains(name_)) {
            auto const& v = json[name_];
            if(v.is_boolean()) {
                *ref_ = v.get<bool>();
                return true;
            } else if(v.is_string()) {
                *ref_ = string_contains_true(v.get<std::string>());
                return true;
            }
        }
        return false;
    }

    inline void read_config(nlohmann::json& dst) const override { dst[name_] = *ref_; }
    inline bool is_flag() const override { return true; }
    inline std::string value_type_str() const override { return "flag"; }
    inline std::string default_value_str() const override { return *ref_ ? "on" : "off"; }
};

}

#endif
