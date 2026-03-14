#ifndef AR_ASSERT_H
#define AR_ASSERT_H
#pragma once

#include <cstdio>
#include <cstdlib>

#if defined(_MSC_VER)
#define AR_DEBUG_BREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
#define AR_DEBUG_BREAK() __builtin_trap()
#else
#include <csignal>
#define AR_DEBUG_BREAK() std::raise(SIGTRAP)
#endif

namespace ar
{
/**
 * @brief Handles a failed AR_ASSERT().
 */
inline void assert_fail(
    const char* expression_text,
    const char* file_name,
    int line_number,
    const char* function_name) noexcept
{
    std::fprintf(
        stderr,
        "AR_ASSERT failed\n"
        "  expression: %s\n"
        "  file:       %s\n"
        "  line:       %d\n"
        "  function:   %s\n",
        expression_text,
        file_name,
        line_number,
        function_name);

    std::fflush(stderr);

    AR_DEBUG_BREAK();
    std::abort();
}
} // namespace ar

#if defined(NDEBUG)

#define AR_ASSERT(EXP) \
do { \
        (void)sizeof(EXP); \
} while (0)

#else

#define AR_ASSERT(EXP) \
do { \
        if (!(EXP)) { \
            ::ar::assert_fail(#EXP, __FILE__, __LINE__, __func__); \
    } \
} while (0)

#endif
#endif // AR_ASSERT_H
