/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

/*
    See README_LOGGING.md for API usage doc.
    See common/emu_log_flags.h for compile-time settings.
    See void api_compile_test_() in this file for some more examples.
*/

#ifndef GFX_EMU_DEBUG_H
#define GFX_EMU_DEBUG_H

#define GFX_EMU_ALWAYS_DEBUG_ 0
//#define GFX_EMU_FAIL_ON_NOT_IMPLEMENTED_

#include <string>
#include <cstring>
#include <cstdio>
#include <stack>
#include <type_traits>
#include <cstdlib>

#include "emu_api_export.h"
#include "emu_utils.h"

#define GFX_EMU_WITH_FLAGS_ using namespace GfxEmu::Log::Flags

#if (GFX_EMU_ALWAYS_DEBUG_ || defined(_DEBUG) || !defined(NDEBUG))

#define GFX_EMU_DEBUG_ENABLED

#endif

#define CONCAT__(X,Y) X##Y
#define CONCAT(X,Y) CONCAT__(X,Y)

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

#define GFX_EMU_MESSAGE_SCOPE_PREFIX(v) \
    GfxEmu::Log::MessagePrefixGuard CONCAT(msgGuard,__LINE__) {v}

#ifdef GFX_EMU_DEBUG_ENABLED

// NB: C++20 supports __VA_OPT__(,), but for earlier versions have to do a trick.
#define ARG1_(F,...) F
#define ARG2_(F,S,...) S

// Minumum 2 arguments: cond, msg (, ...)
#define GFX_EMU_ASSERT_MESSAGE(...) \
    {static const char* m = "Assertion failed. " \
        TOSTRING(ARG2_(__VA_ARGS__)) ", condition: " TOSTRING(ARG1_(__VA_ARGS__))" (at " AT ")\n" ;\
     GfxEmu::AssertMsgOverride_(m , __VA_ARGS__);}

#define GFX_EMU_ASSERT(c) GFX_EMU_ASSERT_MESSAGE(c,#c)
#define GFX_EMU_DEBUG_MESSAGE(...) { GFX_EMU_WITH_FLAGS_; GfxEmu::DebugMessage(__VA_ARGS__); }
#define GFX_EMU_DEBUG_MESSAGE_AT(...) {GFX_EMU_WITH_FLAGS_; static const char* m = AT; GfxEmu::DebugMessage<GfxEmu::Log::Flags::fUnset,&m>(__VA_ARGS__);}

#else

#define GFX_EMU_ASSERT(cond)
#define GFX_EMU_ASSERT_MESSAGE(...)
#define GFX_EMU_DEBUG_MESSAGE(...)
#define GFX_EMU_DEBUG_MESSAGE_AT(...)

#endif

#define GFX_EMU_WARNING_MESSAGE(...) {GFX_EMU_WITH_FLAGS_; GfxEmu::WarningMessage(__VA_ARGS__);}
#define GFX_EMU_WARNING_MESSAGE_AT(...) {GFX_EMU_WITH_FLAGS_; static const char* m = "(at " AT ") "; GfxEmu::WarningMessage<GfxEmu::Log::Flags::fUnset,&m>(__VA_ARGS__);}

#define GFX_EMU_ERROR_MESSAGE(...) {GFX_EMU_WITH_FLAGS_; GfxEmu::ErrorMessage<GfxEmu::Log::Flags::fUnset,nullptr>(__VA_ARGS__);}
#define GFX_EMU_ERROR_MESSAGE_AT(...) {GFX_EMU_WITH_FLAGS_; static const char* m = "(at " AT ") "; GfxEmu::ErrorMessage<GfxEmu::Log::Flags::fUnset,&m>(__VA_ARGS__);}
#define GFX_EMU_FAIL_WITH_MESSAGE(...) {GFX_EMU_WITH_FLAGS_; static const char* m = "(at " AT ") "; GfxEmu::FailWithMessage<&m>(__VA_ARGS__);}

#define GFX_EMU_MESSAGE(...) { GFX_EMU_WITH_FLAGS_; GfxEmu::PrintMessage (__VA_ARGS__); }
#define GFX_EMU_MESSAGE_AT(...) { GFX_EMU_WITH_FLAGS_; static const char* m = AT; GfxEmu::PrintMessage<GfxEmu::Log::LogFile, &m> (__VA_ARGS__);}

//--------------------------------
#define GFX_EMU_STATIC_WARNING__(cond, msg) { \
struct WarnMe{ \
    [[deprecated( msg )]] void DoWarn(std::false_type) { \
        static int once = 0; if(!once) { \
            GFX_EMU_ERROR_MESSAGE(msg); GFX_EMU_MESSAGE(msg); once++; }; \
    } \
    void DoWarn(std::true_type) {}; \
    WarnMe() {DoWarn(typename std::conditional<(cond),std::true_type,std::false_type>::type {});} \
} warn_me; };

#define GFX_EMU_STATIC_WARNING(cond, msg) GFX_EMU_STATIC_WARNING__(cond, "WARNING: " msg " at " AT);
#define GFX_EMU_STATIC_ERROR(C,M) static_assert((C), " *** Error: " M)
//--------------------------------

namespace GfxEmu {

namespace Log {
    constexpr bool kIsDebugBuild =
#ifdef GFX_EMU_DEBUG_ENABLED
        true
#else
        false
#endif
    ;

    inline std::FILE
        *LogFile_ = stdout,
        *ErrFile_ = stderr
    ;

    constexpr auto ErrFile = &ErrFile_;
    constexpr auto LogFile = &LogFile_;

    void setLogFile(const std::string& file);
    GFX_EMU_API inline void flush() {
        std::fflush(*ErrFile);
        std::fflush(*LogFile);
    }

    namespace Flags {
        using Type = uint64_t;
        constexpr size_t kLevelFlagsShiftOffset = 40;

        GFX_EMU_API_IMPORT const char* toStr(const Log::Flags::Type flags);

#define GFX_EMU_DEBUG_MINIMAL_LEVEL(...)
#define GFX_EMU_DEBUG_LEVEL(msg,constant,num) constant = static_cast<Type>(1) << (kLevelFlagsShiftOffset + num),
#define GFX_EMU_DEBUG_FLAG(msg,constant,shift,enabled) constant = static_cast<Type>(1) << shift,
        constexpr auto
#include "emu_log_flags.h"
            fAll          = ~static_cast<Type>(0),
            fUnset        =  static_cast<Type>(0)
        ;
#undef GFX_EMU_DEBUG_MINIMAL_LEVEL
#undef GFX_EMU_DEBUG_LEVEL
#undef GFX_EMU_DEBUG_FLAG

#define GFX_EMU_DEBUG_MINIMAL_LEVEL(...)
#define GFX_EMU_DEBUG_LEVEL(...) + 1
#define GFX_EMU_DEBUG_FLAG(...)
        constexpr auto kImportanceLevels = 0
#include "emu_log_flags.h"
        ;
#undef GFX_EMU_DEBUG_MINIMAL_LEVEL
#undef GFX_EMU_DEBUG_LEVEL
#undef GFX_EMU_DEBUG_FLAG

        static_assert(kLevelFlagsShiftOffset < sizeof(Type)*8);
        static_assert(sizeof(Type)*8 - kLevelFlagsShiftOffset >= kImportanceLevels);

        constexpr auto fLevelsMask = ~Type(0) << kLevelFlagsShiftOffset;

#define GFX_EMU_DEBUG_FLAG(...)
#define GFX_EMU_DEBUG_LEVEL(...)
#define GFX_EMU_DEBUG_MINIMAL_LEVEL(min_level) min_level
        constexpr Type kDefaultLogLevel =
#include "emu_log_flags.h"
                & fLevelsMask ?
#include "emu_log_flags.h"
                :
                static_cast<Type>(1) << (kLevelFlagsShiftOffset + (
#include "emu_log_flags.h"
                ))
        ;
#undef GFX_EMU_DEBUG_MINIMAL_LEVEL
#undef GFX_EMU_DEBUG_LEVEL
#undef GFX_EMU_DEBUG_FLAG

#define GFX_EMU_DEBUG_MINIMAL_LEVEL(...)
#define GFX_EMU_DEBUG_LEVEL(...)
#define GFX_EMU_DEBUG_FLAG(msg,constant,shift,enabled) | (enabled ? constant : Flags::fUnset)
        constexpr auto kDefaultLogFlagsMask {
            Flags::fUnset
#include "emu_log_flags.h"
        };
#undef GFX_EMU_DEBUG_MINIMAL_LEVEL
#undef GFX_EMU_DEBUG_LEVEL
#undef GFX_EMU_DEBUG_FLAG

        GFX_EMU_API_IMPORT bool isEnabled (Type f);
    } // namespace Flags

    void adviceToEnable(Flags::Type, const std::string&);

    struct MessagePrefixGuard {
    private:
        inline static thread_local std::stack<std::string> prevPrefix;
        inline static thread_local std::string curPrefix;
    public:
        GFX_EMU_API static const std::string& getPrefix () { return curPrefix; }
        GFX_EMU_API MessagePrefixGuard(const std::string& p);
        GFX_EMU_API ~MessagePrefixGuard();
    };

    GFX_EMU_API_IMPORT void printBacktrace();
    GFX_EMU_API_IMPORT bool warningsEnabled__ ();
}; // namespace Log

#define GFX_EMU_LOG_PREFIX "EMU: "

template <std::FILE** out = Log::LogFile,
          const char** at = nullptr,
          bool noFlagsChk = false,
          class FlagsT,
          std::enable_if_t<
            std::is_same_v<typename std::decay<FlagsT>::type, Log::Flags::Type>,bool> = true,
          class ... ArgsT>
inline void PrintMessage(const FlagsT flags, const std::string m, ArgsT&& ... args) {
    if (!noFlagsChk && flags && !Log::Flags::isEnabled (flags))
        return;

    std::fputs(GFX_EMU_LOG_PREFIX, *out);

    if(flags)
        std::fprintf(*out, "[%s] ", Log::Flags::toStr(flags));

    if (!Log::MessagePrefixGuard::getPrefix ().empty())
        std::fprintf(*out, "%s", Log::MessagePrefixGuard::getPrefix ().c_str());
    if

#if defined(_WIN32) || __clang__ || __GNUC__ > 7
        constexpr
#endif
    (static_cast<bool> (at)) std::fprintf(*out, "(at %s) ", *at);
    if constexpr (static_cast<bool> (sizeof...(args)))
        std::fprintf(*out, m.c_str (), std::forward<ArgsT>(args)...);
    else
        std::fputs(m.c_str (),*out);
    std::fflush(*out);
}

template <std::FILE** out = Log::LogFile,
          const char** at = nullptr,
          bool noFlagsChk = false,
          class MsgT,
          std::enable_if_t<
                std::is_same_v<typename std::decay<MsgT>::type, std::string> ||
                std::is_same_v<typename std::decay<MsgT>::type, const char*>
            ,bool> = true,
          class ... ArgsT>
inline void PrintMessage(const MsgT m, ArgsT&& ... args) {
    PrintMessage<out,at,noFlagsChk>(Log::Flags::fUnset, m, std::forward<ArgsT>(args)...);
}

template <Log::Flags::Type flags,
          std::FILE** out = Log::LogFile,
          class ... ArgsT>
inline void PrintMessage(const std::string m, ArgsT&& ... args) {
    PrintMessage<out>(flags, m, std::forward<ArgsT>(args)...);
}

template <Log::Flags::Type = Log::Flags::fUnset,
          const char** at = nullptr,
          class ... ArgsT>
inline void DebugMessage(const Log::Flags::Type flags, const char *msg, ArgsT&& ... args) {
    //auto prefix = std::string("*** Debug ");
    PrintMessage<Log::LogFile,at>(flags, /*prefix +*/ msg, std::forward<ArgsT>(args)...);
}

template <Log::Flags::Type flags = Log::Flags::fUnset,
          const char** at = nullptr,
          class ... ArgsT>
inline void DebugMessage(const char *msg, ArgsT&& ... args) {
    DebugMessage<Log::Flags::fUnset,at> (flags, msg, std::forward<ArgsT>(args)...);
}

template <Log::Flags::Type = Log::Flags::fUnset,
          const char** at = nullptr,
          class ... ArgsT>
void WarningMessage(const Log::Flags::Type flags, const char *msg, ArgsT&& ... args) {
    if(!GfxEmu::Log::warningsEnabled__()) return;
    auto prefix = std::string("*** Warning ");
    if
#if defined(_WIN32) || __clang__ || __GNUC__ > 7
        constexpr
#endif
    (static_cast<bool>(at)) prefix += *at;
    PrintMessage<Log::LogFile,nullptr,true>(flags, prefix + msg, std::forward<ArgsT>(args)...);
}

template <Log::Flags::Type flags = Log::Flags::fUnset,
          const char** at = nullptr,
          class ... ArgsT>
inline void WarningMessage(const char *msg, ArgsT&& ... args) {
    WarningMessage<Log::Flags::fUnset,at> (flags, msg, std::forward<ArgsT>(args)...);
}

template <Log::Flags::Type = Log::Flags::fUnset,
          const char** at = nullptr,
          class ... ArgsT>
inline void ErrorMessage(const Log::Flags::Type flags, const char *msg, ArgsT&& ... args) {
    auto prefix = std::string("*** Error ");
    if
#if defined(_WIN32) || __clang__ || __GNUC__ > 7
        constexpr
#endif
    (static_cast<bool>(at)) prefix += *at;
    PrintMessage<Log::ErrFile,nullptr,true>(flags, prefix + msg, std::forward<ArgsT>(args)...);
    PrintMessage<Log::LogFile,nullptr,true>(flags, prefix + msg, std::forward<ArgsT>(args)...);
}

template <Log::Flags::Type flags = Log::Flags::fUnset,
          const char** at = nullptr,
          class ... ArgsT>
inline void ErrorMessage(const char *msg, ArgsT&& ... args) {
    ErrorMessage<Log::Flags::fUnset,at> (flags, msg, std::forward<ArgsT>(args)...);
}

template<class ... ArgsT>
inline void Assert_(
    const bool cond,
    ArgsT&& ... args
    ) {
#ifdef GFX_EMU_DEBUG_ENABLED
    if (cond)
#endif
        return;

    if constexpr (static_cast<bool>(sizeof...(args))) {
        PrintMessage(std::forward<ArgsT>(args)...);
    }

    GfxEmu::Utils::debugBreak ();
}

template<class ... ArgsT>
inline void AssertMsgOverride_(
    const char* mOverride,
    const bool cond,
    const char* oldM,
    ArgsT&& ... args) { Assert_(cond, mOverride, std::forward<ArgsT>(args)...); }

template<
    const char **at = nullptr,
    class MsgOrFlagsT,
    class ... T>
[[ noreturn ]] inline void FailWithMessage(int code, MsgOrFlagsT arg, T ... args) {
    ErrorMessage<Log::Flags::fUnset
#ifndef __clang__
        , at
#endif
    >(std::forward<MsgOrFlagsT>(arg), std::forward<T>(args)...);

    if constexpr (std::is_same_v<MsgOrFlagsT, Log::Flags::Type>) {
        if(!Log::Flags::isEnabled (arg) || !Log::kIsDebugBuild)
            ErrorMessage<Log::Flags::fUnset
#ifndef __clang__
            , at
#endif
            >("For more info make sure to build Debug version with %s flags enabled in common/emu_log_flags.h\n",
                Log::Flags::toStr(arg));
    }

    GfxEmu::Utils::terminate(code);
}

template<const char **at = nullptr, class ... T>
[[ noreturn ]] inline void FailWithMessage(T ... args) {
    FailWithMessage<at>(EXIT_FAILURE,std::forward<T>(args)...);
}

inline void api_compile_test_ () {

    GFX_EMU_ASSERT(false);
    GFX_EMU_ASSERT_MESSAGE(false, "");
    GFX_EMU_ASSERT_MESSAGE(false, "%u", 42);

    using namespace Log::Flags;
    PrintMessage("");
    PrintMessage(fUnset,"");
    PrintMessage<fUnset>("");
    PrintMessage<Log::ErrFile>("");
    PrintMessage<Log::ErrFile>(fUnset,"");
    GFX_EMU_MESSAGE("");
    GFX_EMU_MESSAGE(fUnset,"");
    GFX_EMU_MESSAGE_AT("");
    GFX_EMU_MESSAGE_AT(fUnset,"");

    DebugMessage("");
    DebugMessage(fUnset,"");
    DebugMessage<fUnset>("");
    GFX_EMU_DEBUG_MESSAGE("");
    GFX_EMU_DEBUG_MESSAGE(fUnset,"");
    GFX_EMU_DEBUG_MESSAGE_AT("");
    GFX_EMU_DEBUG_MESSAGE_AT(fUnset,"");

    WarningMessage("");
    WarningMessage(fUnset,"");
    WarningMessage<fUnset>("");
    GFX_EMU_WARNING_MESSAGE("");
    GFX_EMU_WARNING_MESSAGE(fUnset, "");
    GFX_EMU_WARNING_MESSAGE_AT("");
    GFX_EMU_WARNING_MESSAGE_AT(fUnset, "");

    ErrorMessage("");
    ErrorMessage(fUnset,"");
    ErrorMessage<fUnset>("");
    GFX_EMU_ERROR_MESSAGE("");
    GFX_EMU_ERROR_MESSAGE(fUnset,"");
    GFX_EMU_FAIL_WITH_MESSAGE("");
    GFX_EMU_FAIL_WITH_MESSAGE("%u", 42);
    GFX_EMU_FAIL_WITH_MESSAGE(fUnset, "%u", 42);

    FailWithMessage("");
    FailWithMessage("%u",42);
}
}; // namespace GfxEmu

#endif  // #ifndef GFX_EMU_DEBUG_H
