#ifndef _OOCMD_FLOAT_PARAM_HPP
#define _OOCMD_FLOAT_PARAM_HPP

#include <oocmd/params/value_param.hpp>
#include <sstream>

namespace oocmd {

class FloatParam : public ValueParam<float> {
public:
    using ValueParam::ValueParam;

    inline bool configure(nlohmann::json const& json) const override { return ValueParam::configure_number(json, name_, ref_, [](std::string const& s){ return std::stof(s); }); }
    inline void read_config(nlohmann::json& dst) const override { dst[name_] = *ref_; }
    inline std::string value_type_str() const override { return "single"; }

    inline std::string default_value_str() const override {
        std::ostringstream s;
        s << std::fixed << std::setprecision(2) << default_value_;
        return s.str();
    }
};

}

#endif
