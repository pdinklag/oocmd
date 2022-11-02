#ifndef _OOCMD_BYTES_PARAM_HPP
#define _OOCMD_BYTES_PARAM_HPP

#include <cstdint>

#include <oocmd/params/value_param.hpp>
#include <oocmd/util/si_iec_string.hpp>

namespace oocmd {

class BytesParam : public ValueParam<uintmax_t> {
public:
    using ValueParam::ValueParam;

    inline bool configure(nlohmann::json const& json) const override {
        if(json.contains(name_)) {
            auto const& v = json[name_];
            if(v.is_number()) {
                *ref_ = v.get<uint64_t>();
                return true;
            } else if(v.is_string()) {
                uint64_t parse_result;
                if(parse_si_iec_string(v.get<std::string>(), parse_result)) {
                    *ref_ = parse_result;
                    return true;
                }
            }
        }
        return false;
    }

    inline  void read_config(nlohmann::json& dst) const override {
        dst[name_] = *ref_; // TODO: format using SI IEC
    }

    inline std::string value_type_str() const override { return "non-negative SI/IEC integer"; }
    inline std::string default_value_str() const override { return make_si_iec_string(default_value_); }
};

}

#endif
