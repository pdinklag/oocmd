#ifndef _OOCMD_BOOL_STRING_HPP
#define _OOCMD_BOOL_STRING_HPP

#include <cctype>
#include <string>

namespace oocmd {

inline bool iequals(char const* s1, char const* s2) {
    while(*s1 != 0 && *s2 != 0) {
        if(std::tolower((unsigned char)*s1) != std::tolower((unsigned char)*s2)) {
            return false;
        } else {
            ++s1;
            ++s2;
        }
    }
    return (*s1 == 0 && *s2 == 0);
}

inline bool string_contains_true(char const* s) { return iequals(s, "1") || iequals(s, "on") || iequals(s, "true"); };
inline bool string_contains_true(std::string const& s) { return string_contains_true(s.c_str()); }

inline bool string_contains_false(char const* s) { return iequals(s, "0") || iequals(s, "off") || iequals(s, "false"); };
inline bool string_contains_false(std::string const& s) { return string_contains_false(s.c_str()); }

}

#endif
