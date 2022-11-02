#ifndef _OOCMD_STRING_LIST_PARAM_HPP
#define _OOCMD_STRING_LIST_PARAM_HPP

#include <string>
#include <vector>

#include <oocmd/params/value_param.hpp>

namespace oocmd {

class StringListParam : public ValueParam<std::vector<std::string>> {
public:
    using ValueParam::ValueParam;

    inline bool configure(nlohmann::json const& json) const override {
        if(json.contains(name_)) {
            std::vector<std::string> list;

            auto const& a = json[name_];
            if(a.is_array()) {
                list.reserve(a.size());
                for(auto const& e : a.items()) {                    
                    auto const& v = e.value();
                    if(v.is_string()) {
                        list.push_back(v.get<std::string>());
                    } else {
                        // not a string, abort
                        return false;
                    }
                }

                *ref_ = std::move(list);
                return true;
            } else if(a.is_string()) {
                // a single string is interpreted as a list of size 1
                list.reserve(1);
                list.push_back(a.get<std::string>());
                *ref_ = std::move(list);
                return true;
            }
        }
        return false;
    }

    inline bool is_list() const override { return true; }
    inline void read_config(nlohmann::json& dst) const override { dst[name_] = *ref_; }
    inline std::string value_type_str() const override { return "array of strings"; }
    inline std::string default_value_str() const override { return default_value_.empty() ? "none" : ("[" + std::to_string(default_value_.size()) + "]"); }
};

}

#endif
