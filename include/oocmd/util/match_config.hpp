#ifndef _OOCMD_MATCH_CONFIG_HPP
#define _OOCMD_MATCH_CONFIG_HPP

#include <sstream>
#include <oocmd/config_object.hpp>

namespace oocmd {

// walk the JSON and attempt to find matching config parameters in the object
// in the process, build a new config JSON consisting only of matched parameters
// arguments that are identified as parameter values will be discarded from args, so only free arguments remain
// in the end, matched keys will be removed from the input config
inline nlohmann::json match_config(ConfigObject const& cfgobj, nlohmann::json& config, std::vector<char const*>& args, bool ignore_unknown_params, std::string const& context, std::vector<std::string>& errors) {
    static constexpr int NO_VALUE = -1;
    static auto print_error_context = [](std::ostringstream& err, ConfigObject const& x, std::string const& context) {
        if(!context.empty()) {
            err << "object " << context << " (of type " << x.type_name() << ")";
        } else {
            err << "root object (of type " << x.type_name() << ")";
        }
    };

    nlohmann::json matched;
    std::vector<std::string> matched_keys;

    for(auto& p : config.items()) {
        auto const& key = p.key();
        auto& v = p.value();

        // try to get parameter in object
        ConfigParam const* param = cfgobj.get_param(key);
        if(!param && key.length() == 1) {
            // try interpreting it as a short param name instead
            param = cfgobj.get_param(key[0]);
        }

        if(param) {
            // before anything, do a few sanity checks
            if(v.is_array() && !param->is_list()) {
                // TODO: use std::format once GCC supports it...
                std::ostringstream err;
                err << "configuration parameter \"" << key << "\" for ";
                print_error_context(err, cfgobj, context);
                err << " expects a single value, but a list was given";
                errors.emplace_back(err.str());
                continue;
            }

            // check what type of parameter we are dealing with
            if(ObjectParam const* eparam = dynamic_cast<ObjectParam const*>(param)) {
                // this is an object parameter
                // as we are configuring a concrete ConfigObject, we know that it must have been successfully parsed at an earlier point
                if(v.is_object()) {
                    // the value is an object, recurse
                    std::string sub_context;
                    if(context.empty()) {
                        sub_context = key;
                    } else {
                        // TODO: use std::format once GCC supports it...
                        std::ostringstream sub_context_builder;
                        sub_context_builder << context << "." << key;
                        sub_context = sub_context_builder.str();
                    }

                    matched[param->name()] = match_config(eparam->object(), v, args, ignore_unknown_params, sub_context, errors);
                    matched_keys.push_back(key);
                } else {
                    // TODO: use std::format once GCC supports it...
                    std::ostringstream err;
                    err << "cannot assign a value to object parameter \"" << key << "\" of ";
                    print_error_context(err, cfgobj, context);
                    errors.emplace_back(err.str());
                }
            } else {
                // this is any other type of parameter
                if(param->is_flag()) {
                    // this parameter can only be true or false
                    if(v.is_string()) {
                        // *if* it has a value, it should have been assigned via the '=' operator
                        matched[param->name()] = v;
                        matched_keys.push_back(key);
                    } else {
                        // otherwise we assume it to be a free argument and ignore it here; this needs to be documented
                        matched[param->name()] = true;
                        matched_keys.push_back(key);
                    }
                } else {
                    // this parameter does expect a value, so make sure there is one
                    int i;
                    if(v.is_string()) {
                        // it's a string, assign directly
                        matched[param->name()] = v;
                        matched_keys.push_back(key);
                    } else if(v.is_number() && (i = v.get<int>()) != NO_VALUE) {
                        // it's an index into args, assign and discard from args
                        matched[param->name()] = std::string(args[i]);
                        matched_keys.push_back(key);
                        args[i] = nullptr;
                    } else if(v.is_array()) {
                        // walk over array and discard items from args as needed
                        nlohmann::json list;
                        for(auto& x : v.items()) {
                            auto& item = x.value();
                            if(item.is_string()) {
                                // string, push directly
                                list.push_back(item);
                            } else if(item.is_number()) {
                                // index into args, push and remove from args
                                i = item.get<int>();
                                assert(i != NO_VALUE); // shouldn't be possible, because we don't create a list for nothing
                                list.push_back(std::string(args[i]));
                                args[i] = nullptr;
                            } else {
                                // TODO: use std::format once GCC supports it...
                                std::ostringstream err;
                                err << "array item " << x.key() << " of configuration parameter \"" << key << "\" for ";
                                print_error_context(err, cfgobj, context);
                                err << " is of unsupported type " << item.type_name();
                                errors.emplace_back(err.str());
                            }
                        }
                        matched[param->name()] = list;
                        matched_keys.push_back(key);
                    } else {
                        // TODO: use std::format once GCC supports it...
                        std::ostringstream err;
                        err << "configuration parameter \"" << key << "\" for ";
                        print_error_context(err, cfgobj, context);
                        err << " expects a value, but none was given";
                        errors.emplace_back(err.str());
                    }
                }
            }
        } else if(!ignore_unknown_params) {
            // TODO: use std::format once GCC supports it...
            std::ostringstream err;
            err << "unknown configuration parameter \"" << key << "\" for ";
            print_error_context(err, cfgobj, context);
            errors.emplace_back(err.str());
        }
    }

    // erase matched parameters from input config
    if(config.is_object()) {
        for(auto const& key : matched_keys) {
            config.erase(key);
        }
    }

    // return matched parameters that can be used to configure the object
    return matched;
}

}

#endif
