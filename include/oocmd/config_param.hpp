#ifndef _OOCMD_CONFIG_PARAM_HPP
#define _OOCMD_CONFIG_PARAM_HPP

#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

namespace oocmd {

// abstract base for configuration parameters
class ConfigParam {
protected:
    char        short_name_;
    std::string name_;
    std::string desc_;

public:
    inline ConfigParam() {
    }

    inline ConfigParam(const char short_name, std::string&& name, std::string&& desc) : short_name_(short_name), name_(std::move(name)), desc_(std::move(desc)) {
    }

    ConfigParam(ConfigParam const&) = delete;
    ConfigParam& operator=(ConfigParam const&) = delete;
    ConfigParam(ConfigParam&&) = default;
    ConfigParam& operator=(ConfigParam&&) = default;

    inline bool has_short_name() const { return short_name_ != 0; }
    inline char short_name() const { return short_name_; }
    inline std::string const& name() const { return name_; }
    inline std::string const& description() const { return desc_; }

    virtual bool is_flag() const = 0;
    virtual bool is_list() const = 0;

    virtual bool configure(nlohmann::json const& json) const = 0;
    virtual void read_config(nlohmann::json& dst) const = 0;

    virtual std::string value_type_str() const = 0;
    virtual std::string default_value_str() const = 0;
};

}

#endif
