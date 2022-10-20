#ifndef _OOCMD_UINT_PARAM_HPP
#define _OOCMD_UINT_PARAM_HPP

#include <oocmd/params/value_param.hpp>

namespace oocmd {

class UIntParam : public ValueParam<unsigned int> {
public:
    using ValueParam::ValueParam;

    inline bool configure(nlohmann::json const& json) const override { return ValueParam::configure_number(json, name_, ref_, [](std::string const& s){ return std::stoul(s); }); }
    inline void read_config(nlohmann::json& dst) const override { dst[name_] = *ref_; }
    inline std::string value_type_str() const override { return "non-negative integer"; }
    inline std::string default_value_str() const override { return std::to_string(*ref_); }
};

}

#endif
