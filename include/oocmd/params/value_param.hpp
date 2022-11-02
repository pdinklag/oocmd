#ifndef _OOCMD_VALUE_PARAM_HPP
#define _OOCMD_VALUE_PARAM_HPP

#include <concepts>

#include <oocmd/config_param.hpp>

namespace oocmd {

template<std::semiregular T>
class ValueParam : public ConfigParam {
protected:
    template<typename Parser>
    static bool configure_number(nlohmann::json const& json, std::string const& name, T* ref, Parser parse) {
        if(json.contains(name)) {
            auto const& v = json[name];
            if(v.is_string()) {
                try {
                    *ref = parse(v.get<std::string>());
                    return true;
                } catch(std::invalid_argument const&) {
                } catch(std::out_of_range const&) {
                }
            } else if(v.is_number()) {
                *ref = v.get<T>();
                return true;
            }
        }
        return false;
    }

    T* ref_;
    T  default_value_;

public:
    inline ValueParam() : ref_(nullptr) {
    }

    inline ValueParam(const char short_name, std::string&& name, T& ref, std::string&& desc) : ConfigParam(short_name, std::move(name), std::move(desc)), ref_(&ref) {
        default_value_ = ref;
    }

    ValueParam(ValueParam const&) = delete;
    ValueParam& operator=(ValueParam const&) = delete;
    ValueParam(ValueParam&&) = default;
    ValueParam& operator=(ValueParam&&) = default;

    inline virtual bool is_flag() const override { return false; }
    inline virtual bool is_list() const override { return false; }
};

}

#endif
