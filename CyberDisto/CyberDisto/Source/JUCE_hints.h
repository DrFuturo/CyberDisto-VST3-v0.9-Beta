#pragma once

#ifndef JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR
#define JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(className) \
    className (const className&) = delete; \
    className& operator= (const className&) = delete; \
    JUCE_LEAK_DETECTOR(className)
#endif

#ifndef JUCE_LEAK_DETECTOR
#define JUCE_LEAK_DETECTOR(className) \
    private: \
        static void* operator new (size_t); \
        static void* operator new (size_t, void*) noexcept; \
    public: \
        static void operator delete (void*); \
        static void operator delete (void*, void*) noexcept; \
    private: \
        const char* const leakID;
#endif

#ifndef JUCE_DECLARE_NON_COPYABLE
#define JUCE_DECLARE_NON_COPYABLE(className) \
    className (const className&) = delete; \
    className& operator= (const className&) = delete;
#endif

#ifndef JUCE_DECLARE_NON_MOVEABLE
#define JUCE_DECLARE_NON_MOVEABLE(className) \
    className (className&&) = delete; \
    className& operator= (className&&) = delete;
#endif
