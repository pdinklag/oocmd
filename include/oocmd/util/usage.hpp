#ifndef _OOCMD_USAGE_HPP
#define _OOCMD_USAGE_HPP

#include <iostream>

#include <oocmd/config_object.hpp>

namespace oocmd {

inline void print_usage(std::ostream& out, ConfigObject const& e, std::string const& prefix = "") {
    static auto compare_by_name = [](ConfigParam const* a, ConfigParam const* b) {
        if(a->has_short_name() && b->has_short_name()) {
            return a->short_name() < b->short_name();
        } else if(a->has_short_name()) {
            return a->short_name() < b->name()[0];
        } else if(b->has_short_name()) {
            return a->name()[0] < b->short_name();
        } else {
            return a->name() < b->name(); 
        }
    };

    if(prefix.empty()) {
        out << "Options for " << e.type_name() << " -- " << e.description() << ":" << std::endl;
    }

    std::vector<ConfigParam const*> group;
    std::vector<ObjectParam const*> nested;

    // gather immediate (non-object) params into a local group
    for(auto const& it : e.params()) {
        auto const& p = *it.second;
        auto const* eparam = dynamic_cast<ObjectParam const*>(&p);
        if(eparam) {
            nested.push_back(eparam);
        } else {
            group.push_back(&p);
        }
    }

    // sort params by name
    std::sort(group.begin(), group.end(), compare_by_name);
    std::sort(nested.begin(), nested.end(), compare_by_name);

    // determine indentation of right column
    size_t rindent = 0;
    for(auto p : group) {
        size_t i = 2; // "  "

        if(prefix.empty() && p->has_short_name()) {
            // only show short names on root level
            i += 4; // "-X, "
        }

        i += prefix.length() + p->name().length() + 4; // "--<prefix><name>  "

        rindent = std::max(i, rindent);
    }

    // print
    for(auto p : group) {
        out << "  ";
        size_t i = 2;
        if(prefix.empty() && p->has_short_name()) {
            out << "-" << p->short_name() << ", ";
            i += 4;
        }

        out << "--" << prefix << p->name();
        i += prefix.length() + p->name().length() + 2;

        while(i++ < rindent) out << " ";
        out << p->description();
        out << " (" << p->value_type_str() << ", default: " << p->default_value_str()  << ")";
        out << std::endl;
    }
    out << std::endl;

    // handle nested objects
    for(auto eparam : nested) {
        auto const& e = eparam->object();
        out << "Options for " << eparam->name() << " -- " << eparam->description() << " (" << e.type_name() << " -- " << e.description() << ")" << std::endl;
        print_usage(out, e, prefix + eparam->name() + ".");
    }
}

}

#endif
