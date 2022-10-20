#ifndef _OOCMD_SI_IEC_STRING_HPP
#define _OOCMD_SI_IEC_STRING_HPP

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>

namespace oocmd {

inline bool parse_si_iec_string(const char* str, uint64_t& out_v) {
    static constexpr uint64_t SI_BASE = 1000;
    static constexpr uint64_t IEC_BASE = 1024;

    char* p;
    out_v = std::strtoull(str, &p, 10);
    if(p == nullptr) {
        return false; // could not parse any number
    }

    while(*p == ' ') ++p; // skip whitespace

    // find power as indicated by first letter
    uintmax_t power = 0;
    switch(std::toupper((unsigned char)*p)) {
        case 'K': power = 1; break;
        case 'M': power = 2; break;
        case 'G': power = 3; break;
        case 'T': power = 4; break;
        case 'P': power = 5; break;
    }
    
    // if power was given, decide between SI and IEC units
    uintmax_t base = SI_BASE; // default to SI
    if(power != 0) {
        ++p;
        if(std::toupper((unsigned char)*p) == 'I') {
            base = IEC_BASE; // switch to IEC
            ++p;
        }
    }

    // adjust output value
    for (uintmax_t i = 0; i < power; i++) {
        out_v *= base;
    }

    // skip possible byte indicator in case no power was given
    if(power == 0 && std::toupper((unsigned char)*p) == 'B') {
        ++p;
    }
 
    // skip over any remaining spaces
    while(*p == ' ') ++p;

    // report success if end of string was reached
    return (*p == 0);
}

inline bool parse_si_iec_string(std::string const& s, uint64_t& out_v) {
    return parse_si_iec_string(s.data(), out_v);
}

inline std::string make_si_iec_string(uint64_t v) {
    static constexpr uint64_t SI_BASE = 1000;
    static constexpr uint64_t IEC_BASE = 1024;

    static const std::string SI_UNITS[] = { "K", "M", "G", "T", "P", "E" };
    static const std::string IEC_UNITS[] = { "Ki", "Mi", "Gi", "Ti", "Pi", "Ei" };

    size_t order = 0;

    if(v > 0 && v % SI_BASE == 0) {
        // format as SI
        do {
            ++order;
            v /= SI_BASE;
        } while(v > 0 && v % SI_BASE == 0);

        return std::to_string(v) + SI_UNITS[order - 1];
    } else if(v > 0 && v % IEC_BASE == 0) {
        // format as IEC
        do {
            ++order;
            v /= IEC_BASE;
        } while(v > 0 && v % IEC_BASE == 0);

        return std::to_string(v) + IEC_UNITS[order - 1];
    } else {
        // no format
        return std::to_string(v);
    }
}

}

#endif
