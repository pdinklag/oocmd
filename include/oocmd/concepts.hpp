#ifndef _OOCMD_CONCEPTS_HPP
#define _OOCMD_CONCEPTS_HPP

#include <concepts>

namespace oocmd {

class ConfigObject;

/**
 * \brief Requires a type to derive from \ref oocmd::ConfigObject "ConfigObject".
 * 
 * \tparam T the type
 */
template<typename T>
concept DerivedFromConfigObject = std::derived_from<T, ConfigObject>;

}

#endif
