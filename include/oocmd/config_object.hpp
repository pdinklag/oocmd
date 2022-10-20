#ifndef _OOCMD_CONFIG_OBJECT_HPP
#define _OOCMD_CONFIG_OBJECT_HPP

#include <algorithm>
#include <iostream>
#include <unordered_map>

#include <oocmd/concepts.hpp>
#include <oocmd/params/bytes_param.hpp>
#include <oocmd/params/double_param.hpp>
#include <oocmd/params/flag_param.hpp>
#include <oocmd/params/float_param.hpp>
#include <oocmd/params/int_param.hpp>
#include <oocmd/params/string_list_param.hpp>
#include <oocmd/params/string_param.hpp>
#include <oocmd/params/uint_param.hpp>

#include <nlohmann/json.hpp>

namespace oocmd {

/**
 * \brief Abstract base for config objects
 * 
 * Config objects can be configured from the command line by an \ref Application.
 * 
 * The \ref configure "configuration" of an object is done based on the \em parameters it declares.
 * Parameters are typically bound to instance members of the class and should declared during construction using the protected \ref param methods.
 * Object parameters allow the use of member objects in terms of a configuration.
 * A configuration is thus represented by a tree of parameter assignments, internally represented using JSON.
 * 
 * Consider the following example class implementing a config object:
 * \code{.cpp}
 * class Example : public ConfigObject {
 * private:
 *     int x_;
 * 
 * public:
 *     inline Example() : ConfigObject("Example", "An example config object") {
 *         param("x", x_, "An example integer parameter");
 *     }
 * };
 * \endcode
 * An \c Example object will accept a configuration parameter named "x", which is interpreted as an integer value to be written into member variable \c x_ .
 * 
 * Now, consider the following template example featuring member objects:
 * \code{.cpp}
 * template<DerivedFromConfigObject MemberObjectType>
 * class TemplateExample : public ConfigObject {
 * private:
 *     MemberObjectType member_;
 * 
 * public:
 *     inline TemplateExample() : ConfigObject("TemplateExample", "An example using a subobject") {
 *         param("member", member_, "A parameterized member object");
 *     }
 * }
 * \endcode
 * If a configuration contains a sub configuration named \c member , it will be used to recursively configure the member \c member_ .
 * The concept \ref oocmd::DerivedFromConfigObject "DerivedFromConfigObject" can be used to constrain template types to those deriving from ConfigObject.
 */
class ConfigObject {
private:
    std::string type_name_;
    std::string desc_;
    std::unordered_map<std::string, std::unique_ptr<ConfigParam>> params_;
    std::unordered_map<char, std::string> short_params_;

    template<typename T, typename V>
    void make_param(const char short_name, std::string&& name, V& ref, std::string&& desc) {
        auto name_copy = std::string(name);
        auto p = std::make_unique<T>(short_name, std::move(name), ref, std::move(desc));
        if(p->has_short_name()) short_params_.emplace(short_name, name_copy);
        params_.emplace(name_copy, std::move(p));
    }

public:
    class NestedParam : public ConfigParam {
    private:
        ConfigObject* object_;

    public:
        inline NestedParam() {
        }

        inline NestedParam(const char short_name, std::string&& name, ConfigObject& x, std::string&& desc)
            : ConfigParam(short_name, std::move(name), std::move(desc)), object_(&x) {
        }

        NestedParam(NestedParam const&) = delete;
        NestedParam& operator=(NestedParam const&) = delete;
        NestedParam(NestedParam&&) = default;
        NestedParam& operator=(NestedParam&&) = default;

        /**
         * \brief Provides access to the targeted object
         * 
         * \return a reference to the targeted object
         */
        inline ConfigObject const& object() const { return *object_; }

        inline bool is_flag() const override { return false; }
        inline bool is_list() const override { return false; }

        inline bool configure(nlohmann::json const& json) const override {
            if(json.contains(name_)) {
                auto const& v = json[name_];
                if(v.is_object() && !v.empty()) {
                    // don't try to configure using anything but a non-empty JSON object
                    // if it's not an object, that's fine, it may have been just a type name indicating what object to use
                    object_->configure(v);
                }
                return true;
            } else {
                return false;
            }
        }

        inline void read_config(nlohmann::json& dst) const override {
            auto sub = object_->config();
            if(!sub.is_null()) {
                dst[name_] = sub;
            }
        }

        inline std::string value_type_str() const override { return "object"; }
        inline std::string default_value_str() const override { return std::string(object_->type_name()); }
    };

protected:
    /**
     * \brief Constructs an object with given type name and description
     * 
     * \param type_name the type display name used for error reporting and help output
     * \param desc a descriptive help text for users
     */
    inline ConfigObject(std::string&& type_name, std::string&& desc) : type_name_(std::move(type_name)), desc_(std::move(desc)) {
    }

    /**
     * \brief Declares a boolean config parameter, also known as a flag
     * 
     * Flags that default to \c true can be disabled by assigning any value different from \c true or \c 1 in the configuration.
     * In the command line, in order to explicitly assign values to flags, the <tt>=</tt> operator must be used, e.g., \c --flag=0 .
     * 
     * \param short_name the short (single-character) name of the parameter
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(const char short_name, std::string&& name, bool& ref, std::string&& desc = "") { make_param<FlagParam>(short_name, std::move(name), ref, std::move(desc)); }

    /**
     * \brief Declares a boolean config parameter, also known as a flag
     * 
     * Flags that default to \c true can be disabled by assigning any value different from \c true or \c 1 in the configuration.
     * In the command line, in order to explicitly assign values to flags, the <tt>=</tt> operator must be used, e.g., \c --flag=0 .
     * 
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(std::string&& name, bool& ref, std::string&& desc = "") { param(0, std::move(name), ref, std::move(desc)); }

    /**
     * \brief Declares a (signed) integer config parameter
     * 
     * In the command line, in order to assign a negative value, the <tt>=</tt> operator must be used, e.g., \c --param=-1 .
     * 
     * \param short_name the short (single-character) name of the parameter
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(const char short_name, std::string&& name, int& ref, std::string&& desc = "") { make_param<IntParam>(short_name, std::move(name), ref, std::move(desc)); }
   
    /**
     * \brief Declares a (signed) integer config parameter
     * 
     * In the command line, in order to assign a negative value, the <tt>=</tt> operator must be used, e.g., \c --param=-1 .
     * 
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(std::string&& name, int& ref, std::string&& desc = "") { param(0, std::move(name), ref, std::move(desc)); }

    /**
     * \brief Declares an unsigned integer config parameter
     * 
     * \param short_name the short (single-character) name of the parameter
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(const char short_name, std::string&& name, unsigned int& ref, std::string&& desc = "") { make_param<UIntParam>(short_name, std::move(name), ref, std::move(desc)); }

    /**
     * \brief Declares an unsigned integer config parameter
     * 
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(std::string&& name, unsigned int& ref, std::string&& desc = "") { param(0, std::move(name), ref, std::move(desc)); }

    /**
     * \brief Declares an 64-bit unsigned integer config parameter
     * 
     * Values can be given as SI or IEC formatted strings and will be parsed into the represented number of bytes.
     * For example, the string "10K" is parsed to the number 10,000, and "10Ki" is parsed to the number 10,240.
     * The supported orders range from kilo (10^3) / kibi (2^10) to peta (10^15) / pebi (2^50).
     * 
     * \param short_name the short (single-character) name of the parameter
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(const char short_name, std::string&& name, uint64_t& ref, std::string&& desc = "") { make_param<BytesParam>(short_name, std::move(name), ref, std::move(desc)); }
   
    /**
     * \brief Declares an 64-bit unsigned integer config parameter
     * 
     * Values can be given as SI or IEC formatted strings and will be parsed into the represented number of bytes.
     * For example, the string "10K" is parsed to the number 10,000, and "10Ki" is parsed to the number 10,240.
     * The supported orders range from kilo (10^3) / kibi (2^10) to peta (10^15) / pebi (2^50).
     * 
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(std::string&& name, uint64_t& ref, std::string&& desc = "") { param(0, std::move(name), ref, std::move(desc)); }

    /**
     * \brief Declares a single-precision floating point config parameter
     * 
     * In the command line, in order to assign a negative value, the <tt>=</tt> operator must be used, e.g., \c --param=-0.1 .
     * 
     * \param short_name the short (single-character) name of the parameter
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(const char short_name, std::string&& name, float& ref, std::string&& desc = "") { make_param<FloatParam>(short_name, std::move(name), ref, std::move(desc)); }

    /**
     * \brief Declares a single-precision floating point config parameter
     * 
     * In the command line, in order to assign a negative value, the <tt>=</tt> operator must be used, e.g., \c --param=-0.1 .
     * 
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(std::string&& name, float& ref, std::string&& desc = "") { param(0, std::move(name), ref, std::move(desc)); }

    /**
     * \brief Declares a double-precision floating point config parameter
     * 
     * In the command line, in order to assign a negative value, the <tt>=</tt> operator must be used, e.g., \c --param=-0.1 .
     * 
     * \param short_name the short (single-character) name of the parameter
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(const char short_name, std::string&& name, double& ref, std::string&& desc = "") { make_param<DoubleParam>(short_name, std::move(name), ref, std::move(desc)); }

    /**
     * \brief Declares a double-precision floating point config parameter
     * 
     * In the command line, in order to assign a negative value, the <tt>=</tt> operator must be used, e.g., \c --param=-0.1 .
     * 
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(std::string&& name, double& ref, std::string&& desc = "") { param(0, std::move(name), ref, std::move(desc)); }

    /**
     * \brief Declares a string config parameter
     * 
     * \param short_name the short (single-character) name of the parameter
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(const char short_name, std::string&& name, std::string& ref, std::string&& desc = "") { make_param<StringParam>(short_name, std::move(name), ref, std::move(desc)); }

    /**
     * \brief Declares a string config parameter
     * 
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(std::string&& name, std::string& ref, std::string&& desc = "") { param(0, std::move(name), ref, std::move(desc)); }

    /**
     * \brief Declares a string list config parameter
     * 
     * In the command line, lists are created by passing the same argument multiple times, e.g., the arguments <tt>--param=first --param=second</tt> would create a list consisting of \c first and \c second.
     * 
     * \param short_name the short (single-character) name of the parameter
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(const char short_name, std::string&& name, std::vector<std::string>& ref, std::string&& desc = "") { make_param<StringListParam>(short_name, std::move(name), ref, std::move(desc)); }

    /**
     * \brief Declares a string list config parameter
     * 
     * In the command line, lists are created by passing the same argument multiple times, e.g., the arguments <tt>--param=first --param=second</tt> would create a list consisting of \c first and \c second.
     * 
     * \param name the name of the parameter
     * \param ref  a reference to the variable bound to the parameter
     * \param desc an optional descriptive help text for users
     */
    inline void param(std::string&& name, std::vector<std::string>& ref, std::string&& desc = "") { param(0, std::move(name), ref, std::move(desc)); }

    /**
      * \brief Declares an object config parameter
      * 
      * The object will be configured recursively; see \ref Entity "class description" for details.
      * Note that object parameters cannot have a short name.
      * 
      * \tparam E the object type
      * \param name the name of the parameter
      * \param ref  a reference to the variable bound to the parameter
      * \param desc an optional descriptive help text for users
      */
    template<DerivedFromConfigObject T>
    void param(std::string&& name, T& ref, std::string&& desc = "") { make_param<NestedParam>(0, std::move(name), dynamic_cast<ConfigObject&>(ref), std::move(desc)); }

public:
    /**
     * \brief Constructs an empty object
     * 
     * Note that an object constructed in this way cannot be properly registered by an \ref Application as it is missing a type name.
     * Consider using the \ref ConfigObject(std::string&&,std::string&&) constructor instead.
     */
    inline ConfigObject() {
    }

    ConfigObject(ConfigObject const&) = delete;
    ConfigObject& operator=(ConfigObject const&) = delete;

    ConfigObject(ConfigObject&&) = default;
    ConfigObject& operator=(ConfigObject&&) = default;

    /**
     * \brief Attempts to retrieve a config parameter by name
     *
     * \param name the name of the parameter
     * \return a const pointer to the parameter with the given name, or \c nullptr if no such parameter exists
     */
    inline ConfigParam const* get_param(std::string const& name) const {
        auto it = params_.find(name);
        if(it != params_.end()) {
            return it->second.get();
        } else {
            return nullptr;
        }
    }

    /**
     * \brief Attempts to retrieve a config parameter by its short name
     *
     * \param short_name the short name of the parameter
     * \return a const pointer to the parameter with the given short name, or \c nullptr if no such parameter exists
     */
    inline ConfigParam const* get_param(char const short_name) const {
        auto it = short_params_.find(short_name);
        if(it != short_params_.end()) {
            return params_.find(it->second)->second.get(); // no need for an additional check; long name must exist
        } else {
            return nullptr;
        }
    }

    /**
     * \brief Configures the object using the given configuration
     * 
     * This is done automatically by an \ref Application after reading the command line, but it can also be used to configure an object manually using an external JSON configuration.
     * 
     * \param json the configuration as JSON
     */
    inline void configure(nlohmann::json const& json) {
        for(auto const& it : params_) {
            it.second->configure(json);
        }
    }

    /**
     * \brief Reports the object's current configuration as JSON
     * 
     * \return the object's current configuration as JSON
     */
    inline nlohmann::json config() const {
        nlohmann::json cfg;
        for(auto const& it : params_) {
            it.second->read_config(cfg);
        }
        return cfg;
    }

    /**
     * \brief Gets the object's type name for registration purposes
     * 
     * \return the object's type name for registration purposes
     */
    inline std::string const& type_name() const {
        return type_name_;
    }

    /**
     * \brief Gets the object's descriptive text for display in a help screen
     * 
     * \return the object's descriptive text for display in a help screen
     */
    inline std::string const& description() const {
        return desc_;
    }

    /**
     * \brief Provides access to the parameters declared by the object
     * 
     * \return the mapping of parameter names to parameters
     */
    inline auto const& params() const {
        return params_;
    }
};

using ObjectParam = ConfigObject::NestedParam;

}

#endif
