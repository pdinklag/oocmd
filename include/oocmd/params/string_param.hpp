#ifndef _OOCMD_STRING_PARAM_HPP
#define _OOCMD_STRING_PARAM_HPP

#include <string>

#include <oocmd/params/value_param.hpp>

namespace oocmd {

class StringParam : public ValueParam<std::string> {
public:
    using ValueParam::ValueParam;
    
    inline bool configure(nlohmann::json const& json) const override {
        if(json.contains(name_)) {
            auto const& v = json[name_];
            if(v.is_string()) {
                *ref_ = v.get<std::string>();
                return true;
            }
        }
        return false;
    }

    inline void read_config(nlohmann::json& dst) const override { dst[name_] = *ref_; }
    inline std::string value_type_str() const override { return "string"; }
    inline std::string default_value_str() const override { return std::string(*ref_); }
};

}

#endif
