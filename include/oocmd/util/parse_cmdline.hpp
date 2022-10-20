#ifndef _OOCMD_PARSE_CMDLINE_HPP
#define _OOCMD_PARSE_CMDLINE_HPP

#include <nlohmann/json.hpp>
#include <oocmd/util/bool_string.hpp>

namespace oocmd {
    
// result of the context-free command line parsing (first, context-oblivious pass)
struct CmdlineConfig {
    // contains the command line parameters parsed into a hierarchy
    // all fields have an integer value that point into the list of potential free arguments (args), -1 indicating none
    nlohmann::json json;
    
    // the list of arguments that may either represent parameter values or free arguments
    // this can only be figured out in a context aware pass
    std::vector<char const*> args;
};

inline static CmdlineConfig parse_cmdline(int argc, char** argv, std::vector<std::string>& errors) {
    static constexpr int NO_VALUE = -1;
    static auto set_or_make_list = [](nlohmann::json* obj, auto const& value) {
        if(obj->is_null() || (obj->is_number() && obj->get<int>() == NO_VALUE)) {
            // no value, simply set
            *obj = value;
        } else {
            // has a value, make it a list
            nlohmann::json list;
            list.push_back(*obj);
            list.push_back(value);
            *obj = list;
        }
    };

    nlohmann::json obj;

    std::vector<char const*> args;
    nlohmann::json* current_param = nullptr;
    bool parsed_assignment = false;

    // walk arguments, skip first
    char* arg;
    for(int i = 1; i < argc; i++) {
        bool assign_value;
        if(parsed_assignment) {
            assign_value = true; // no matter what comes now, it must be a value, not a parameter name
            *(arg-1) = '='; // restore original command line
            parsed_assignment = false; // reset
        } else {
            arg = argv[i];
            assign_value = false;
        }

        // parse arg
        if(!assign_value && *arg == '-') {
            // this argument is a parameter name
            ++arg;
            if(*arg == '-') {
                // long param - parse and create object path
                ++arg;
                
                char* current_name = arg;
                nlohmann::json* current_obj = &obj;
                while(*arg) {
                    if(*arg == '=') {
                        // we found an assignment operator, treat it like the end of the parameter name
                        *arg = 0; // the name should only be parsed up to here, we restore it in the next step
                        ++arg; // advance one position
                        --i;   // make sure we stay in the current command-line argument
                        parsed_assignment = true; // also make sure we don't go back to the beginning of the argument
                        break;
                    } else if(*arg == '.') {
                        // we found a dereferencing operator - navigate into sub object
                        *arg = 0;
                        if(current_obj->contains(current_name)) {
                            // the key already exists
                            auto& v = (*current_obj)[current_name];
                            if(v.is_object()) {
                                // the value is already an object, simply navigate to it
                                current_obj = &v;
                            } else {
                                // the value is something other than an object - this is not legal
                                // TODO: use std::format once GCC supports it...
                                *arg = '.';
                                std::ostringstream err;
                                err << "error parsing argument \"" << argv[i] << "\": already assigned a value to alleged parent ";
                                *arg = 0;
                                err << "\"" << argv[i] << "\"";
                                errors.emplace_back(err.str());
                            }
                        } else {
                            // the key does not exist yet, create an empty sub object and navigate to it
                            (*current_obj)[current_name] = nlohmann::json::object();
                            current_obj = &(*current_obj)[current_name];
                        }
                        
                        current_name = arg + 1;
                        *arg = '.'; // restore original command line
                    }
                    ++arg;
                }
                
                // assign -1 (no value yet) to current name in current object, and make it the "current" parameter
                // TODO: assert that current_name is non-empty
                if(current_obj->contains(current_name)) {
                    current_param = &(*current_obj)[current_name];
                } else {
                    (*current_obj)[current_name] = NO_VALUE;
                    current_param = &(*current_obj)[current_name];
                }
            } else {
                // short param - interpret every character in the argument as a short parameter name
                std::string name;
                name.resize(1);
                while(*arg) {
                    name[0] = *arg;
                    obj[name] = NO_VALUE;
                    current_param = &obj[name];
                    ++arg;
                }
            }
        } else {
            if(current_param) {
                // this could either be a free argument or the value of current_param
                if(current_param->is_object()) {
                    // the current parameter is an object, which means that a previous command line argument stated a sub parameter
                    // the only way this can legally be assigned a value is its type name
                    std::ostringstream err;
                    err << "error parsing argument \"" << argv[i] << "\": cannot assign value because a sub parameter has already been defined";
                    errors.emplace_back(err.str());
                }

                if(assign_value) {
                    set_or_make_list(current_param, arg);
                } else {
                    // we keep potential free arguments in a separate list, so assign the index here
                    const auto arg_index = (int)args.size();
                    set_or_make_list(current_param, arg_index);
                }

                // reset current parameter after assignment
                current_param = nullptr;
            }

            if(!assign_value) {
                // unless we are dealing with an assignment ('=' operator), consider the argument a potential free argument

                // do a few sanity checks to aid the user
                if(string_contains_true(arg) || string_contains_false(arg)) {
                    // TODO: use std::format once GCC supports it...
                    std::ostringstream err;
                    err << "error parsing argument \"" << arg << "\": in case you are trying to explicitly set a value, use the '=' operator instead, e.g., \"--x=" << arg << "\" instead of \"--x " << arg << "\"";
                    err << " (if you actually have an input file named \"" << arg << "\", please consider using a different file name ...)";
                    errors.emplace_back(err.str());
                }

                // store it
                args.emplace_back(arg);
            }
        }
    }

    // done
    return { obj, args };
}

}

#endif
