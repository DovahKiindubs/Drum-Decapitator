#pragma once

#if defined(JUCE_MODULE_AVAILABLE_juce_core) || defined(JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED)
#include <JuceHeader.h>
#include <cassert>
#define massert(cond) jassert(cond)  // 使用 JUCE 的断言系统
#else

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#include <cmath>
#include <cassert>

#define finline inline

#define MINIWL_NONCOPYABLE_AND_LEAK_DETECTOR( classname ) \
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( classname )

#define massert(cond) assert(cond)

namespace miniwl {
    template< typename T >
    using DecaySIMD = T;

    template< typename T >
    constexpr static T Pi = T(M_PI);

} // end of namespace miniwl

#endif