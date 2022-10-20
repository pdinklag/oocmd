#ifndef _OOCMD_APPLICATION_HPP
#define _OOCMD_APPLICATION_HPP

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include <oocmd/config_object.hpp>
#include <oocmd/util/match_config.hpp>
#include <oocmd/util/parse_cmdline.hpp>
#include <oocmd/util/usage.hpp>

namespace oocmd {

/**
 * \brief Parses the command line and configures a \ref ConfigObject
 * 
 * Applications are designed as the entry point of a program that parses command-line arguments and configures an \ref ConfigObject .
 * 
 * The command line is first parsed into a JSON dataset as follows:
 * each argument starting with <tt>-</tt> or <tt>--</tt> is considered a parameter.
 * If a dot ( <tt>.</tt> ) is encountered within a parameter name, this results in the creation of a sub object into which the parameter is stored.
 * If an object parameter is assigned a value and it also has sub parameters, said value will be considered the object's \em name, which is later matched against the \ref ConfigObject::type_name "type name" of a registered config object.
 * Values are assigned to parameters by stating them as the subsequent argument, or by using the equals ( <tt>=</tt> ) symbol.
 * Note that values are not interpreted before an config object is selected and configured.
 * If the same parameter is assigned a value multiple times, it will result in a list containing all values in their order of occurrence.
 * 
 * As an example, the command line <tt>-x --obj.a 100 --obj.b str --obj.flag in1 in2</tt> may be parsed to the following JSON object:
 * \code {.json}
 * {
 *     "obj": {
 *         "a": "100",
 *         "b": "str",
 *         "flag": true
 *     },
 *     "x": true
 * }
 * \endcode
 * The argument \c in2 is considered a \em free argument because it is not bound to any parameter.
 * This will later be considered an input file argument.
 * The case of \c in1 is ambiguous at first: whether it is the value of \c --obj.flag depends on whether the object named \c obj declares a flag parameter named \c flag .
 * Note that the ambiguities can be avoided by using the equals ( <tt>=</tt> ) symbol for assignments, e.g., in order to set \c obj.flag to \c false explicitly, the argument \c obj.flag=false should be passed.
 */
class Application : public ConfigObject {
private:
    inline static bool report_errors(std::vector<std::string> const& errors) {
        if(!errors.empty()) {
            for(auto& e : errors) {
                std::cerr << e << std::endl;
            }
            return true;
        } else {
            return false;
        }
    }

    // command line parameters
    bool good_;

    std::filesystem::path binary_;
    std::vector<std::string> args_;

    bool help_ = false;

public:
    /**
     * \brief Attempts to parse the given command line and configure the given object
     * 
     * This method is typically called by a \c main method.
     * Refer to the detailed description of \ref Application for details on how the command line is parsed and interpreted.
     * 
     * \param x the object to configure
     * \param argc the number of provided command line arguments
     * \param argv the command line arguments
     */
    inline Application(ConfigObject& x, int argc, char** argv) : ConfigObject("Application", "Command line parser of oocmd"), good_(false) {
        // declare params
        {
            param('h', "help", help_, "Shows this help.");
        }

        // parse
        {
            if(argc > 0) {
                binary_ = argv[0];
            }

            std::vector<std::string> errors;

            // parse command line into json
            auto cmdline = parse_cmdline(argc, argv, errors);
            if(report_errors(errors)) return;
            // if constexpr(DEBUG) std::cout << "parsed config: " << cmdline.json << std::endl;

            // configure the application itself
            {
                auto matched = match_config(*this, cmdline.json, cmdline.args, true, "", errors);
                if(report_errors(errors)) return;
                configure(matched);
            }
            
            // attempt to match the parsed configuration to the given object
            auto matched = match_config(x, cmdline.json, cmdline.args, false, "", errors);

            if(report_errors(errors)) return;
            assert(cmdline.json.empty()); // everything should have been matched

            // configure the executable
            x.configure(matched);

            // gather the remaining free arguments
            for(auto const& arg : cmdline.args) {
                if(arg) args_.emplace_back(arg);
            }
        }

        if(help_) {
            // print help
            print_usage(x);
        } else {
            good_ = true;
        }
    }

    /**
     * \brief Equivalent to \ref good
     */
    explicit inline operator bool() const { return good(); }

    /**
     * \brief Tests whether the application is good to go, i.e., the command line has been successfully parsed
     * \return true in case the command line has successfully been parsed and the application is ready to proceed
     * \return false in case any errors occured during the parsing, or in case the \c --help argument was read
     */
    inline bool good() const { return good_; }

    /**
     * \brief Prints a usage text to the standard output for the given object
     * 
     * \param x a config object
     */
    void print_usage(ConfigObject const& x) const {
        std::cout << "Usage: " << binary_.filename().string() << " [PARAM=VALUE]... [FILE]..." << std::endl;
        std::cout << std::endl;
        
        oocmd::print_usage(std::cout, x);
        oocmd::print_usage(std::cout, *this);
    }

    /**
     * \brief Reports the path to the binary parsed from the command line
     * 
     * This is retrieved from the first argument passed via the command line
     * 
     * \return the path to the current binary
     */
    inline const std::filesystem::path binary() const { return binary_; }

    /**
     * \brief Reports the free arguments gathered from the command line
     * 
     * Free arguments are those that did not represent values of any object parameters.
     * Typically, these are considered paths to input files.
     * 
     * \return the free arguments gathered from the command line
     */
    inline std::vector<std::string> const& args() const { return args_; }
};

}

#endif
