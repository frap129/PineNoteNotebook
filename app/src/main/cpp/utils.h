//
// Created by Joshua Mulliken on 6/2/22.
//

#ifndef PINENOTE_LIB_UTILS_H
#define PINENOTE_LIB_UTILS_H

#include <stdexcept>

using namespace std;

template<typename Target, typename Source>
Target narrow_cast(Source var) {
    auto r = static_cast<Target> (var);
    if (static_cast<Source> (r) != var)
        throw runtime_error("narrow_cast<>() failed");

    return r;
}

#if defined(__GNUC__) || defined(__clang__)
#define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED
#endif

#endif //PINENOTE_LIB_UTILS_H
