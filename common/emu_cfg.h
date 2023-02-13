/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include <limits>
#include <string>
#include <sstream>
#include <type_traits>
#include <functional>
#include <vector>

#include "emu_log.h"
#include "emu_utils.h"
#include "emu_api_export.h"
#include "emu_cfg_platform.h"

namespace GfxEmu {

namespace CfgCache {
    inline std::atomic<int64_t> LogChannels   {GfxEmu::Log::Flags::kDefaultLogFlagsMask};
    inline std::atomic<int64_t> MinimalLevel  {GfxEmu::Log::Flags::kDefaultLogLevel};
    inline std::string LogFileMode = "w+";
};

namespace Cfg {

GFX_EMU_API_IMPORT void* init ();
GFX_EMU_API_IMPORT void printSummary ();

inline void* init__ = init();

class Param {
public:
    template<bool getPrevV = false>
    std::string getDbgStr() const {
        const auto& source = getPrevV ? prevV : actualV;
        std::stringstream ss;
        ss << "[";
        if(isInt () || isString ()) {
            ss << "str: " << source.vStr;
            ss << " int: " << std::dec << source.vInt;
            ss << "(0x" << std::hex << source.vInt << ")";
        }
        else if(isFp ()) ss << " fp: " << source.vFp;
        else if(isBool ()) ss << " bool: " << source.vBool;
        ss << "]";
        return ss.str();
    }

private:

    struct SourceSpec {
        std::string env;
        std::string cli;
    };

    SourceSpec srcSpec;

    std::string name, desc;

    struct Value {
        std::string vStr;
        int64_t     vInt;
        double      vFp;
        bool        vBool;
        bool operator ==(const Value& o) {
            return
                vStr == o.vStr &&
                vInt == o.vInt &&
                vFp == o.vFp &&
                vBool == o.vBool
            ;
        }

        bool operator !=(const Value& o) { return !(*this == o);}
    };

    Value actualV, defaultV, prevV;

    bool isUserDefined_ = false,
         isSettingDefaults_ = false;

    enum class Type {
        Bool, Int, Fp, String
    };

    using ValueCallback = std::function<bool(Param&)>;
    Type type;
    ValueCallback valueCallback;
    std::string valueCallbackErrStr;

    static const int64_t kIntValueInvalid;
    static const double kFpValueInvalid;

    enum SetParams {
        kNoParams = 0,
        kSetDefaults = 1,
        kSetSubvalue = 1 << 1,
        kFromCallback = 1 << 2
    };

    template<
        SetParams params  = kNoParams,
        bool setDefaults = (bool)(params & kSetDefaults),
        bool setAll = !(params & kSetSubvalue),
        bool fromCallback = (bool)(params & kFromCallback),
        class T>
    void set_(T v) {

        if (!isSettingDefaults_){
            if constexpr (setDefaults)
                isSettingDefaults_ = true;
            else
                isUserDefined_ = true;
        }

        Value& target = isSettingDefaults_ ? defaultV : actualV;
        if constexpr (std::is_same_v<bool,T>) {
            if constexpr (setDefaults) type = Type::Bool;
            if(setAll) {
                target.vInt = static_cast<int64_t>(v);
                target.vFp = static_cast<double>(v);
                target.vStr = std::to_string(v);
            }
            target.vBool = v;
        } else if constexpr (std::is_integral_v<T>) {
            if constexpr (setDefaults) type = Type::Int;
            target.vInt = static_cast<int64_t>(v);
            if(setAll) {
                target.vFp = static_cast<double>(v);
                target.vBool = static_cast<bool>(v);
                target.vStr = std::to_string(v);
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            if constexpr (setDefaults) type = Type::Fp;
            target.vFp = static_cast<double>(v);
            if(setAll) {
                target.vInt = static_cast<int64_t>(v);
                target.vBool = static_cast<bool>(v);
                target.vStr = std::to_string(v);
            }
        } else if constexpr (
            std::is_same_v<typename std::decay<T>::type, std::string> ||
            std::is_same_v<typename std::decay<T>::type, const char*>
        ) {
            if constexpr (setDefaults) type = Type::String;
            target.vStr = v;
            if(setAll) {
                if(isBool()) {
                    target.vBool = GfxEmu::Utils::stringToBool(GfxEmu::Utils::toLower(v));
                    target.vInt = target.vFp = target.vBool;
                } else {
                    try {
                        target.vInt = std::stoll(v);
                    } catch (...) {
                        const auto extractedInts =
                            GfxEmu::Utils::extractFromStr<decltype(target.vInt)>(v,std::regex("[+-]?[0-9]+"));
                        target.vInt = extractedInts.size () ? extractedInts[0] : kIntValueInvalid;
                    };

                    try {
                        target.vFp = std::stod(v);
                    } catch (...) {
                        const auto extractedFp =
                            GfxEmu::Utils::extractFromStr<decltype(target.vFp)>(v,std::regex("[+-]?[0-9]+([.,][0-9]+)?"));
                        target.vFp = extractedFp.size () ? extractedFp[0] : kIntValueInvalid;
                    };

                    target.vBool = target.vInt;
                }
            }
        }

        if (isSettingDefaults_) actualV = defaultV;

        if constexpr (!fromCallback) {
            if(!valueCallback(*this)) {
                GFX_EMU_FAIL_WITH_MESSAGE(fCfg,
                    "%s\n", valueCallbackErrStr.c_str ()
                );
            }

            if(!setDefaults) {
                if(actualV != prevV) {
                    GFX_EMU_MESSAGE(fCfg,
                        "%s <- %s (old: %s)\n",
                            name.c_str(),
                            getDbgStr ().c_str (),
                            getDbgStr<true> ().c_str ()
                        );
                }
            }

            prevV = actualV;
        }

        if constexpr (setDefaults) isSettingDefaults_ = false;
    }

    void setFromSources () {
        // Environment source.
        if(srcSpec.env != "") {
            const auto envValPtr = std::getenv(srcSpec.env.c_str ());
            if(envValPtr) {
                isUserDefined_ = true;
                const auto envVal = std::string {envValPtr};
                GFX_EMU_MESSAGE(fCfg,
                    "ENV: %s = %s\n",
                        srcSpec.env.c_str (), envVal.c_str ());
                if(envVal != "") {
                    set_(envVal);
                }
            }
        }
    }

    void addToRegistry ();

public:
    bool isSettingDefaults () const { return isSettingDefaults_; }

    template<class T>
    void set(T v) {
        set_<SetParams(kFromCallback)> (v);
    }

    template<class T>
    void setSubValue(T v) {
        set_<SetParams(kSetSubvalue | kFromCallback)> (v);
    }

    void setValidPredErrStr(std::string v) {valueCallbackErrStr = v;}

    const std::string& getStr () const { return actualV.vStr; }
    template<class TargetT = int64_t>
    TargetT getInt () const { return static_cast<TargetT>(actualV.vInt); }
    const int64_t& getIntRef () const { return actualV.vInt; }
    double getFp () const { return actualV.vFp; }
    bool getBool () const { return actualV.vBool; }

    std::string getDefaultStr () const { return defaultV.vStr; }
    int64_t getDefaultInt () const { return defaultV.vInt; }
    double getDefaultFp () const { return defaultV.vFp; }
    bool getDefaultBool () const { return defaultV.vBool; }

    const std::string& getName () const { return name; }
    const std::string& getDesc () const { return desc; }

    bool isNotDefault () const {
        return actualV.vStr != defaultV.vStr;
    }

    bool isUserDefined() const { return isUserDefined_; }

    template<class T, class dT = typename std::decay<T>::type,
        std::enable_if_t<
            (std::is_integral_v<dT> && !std::is_same_v<dT,bool>) ||
             std::is_enum_v<dT>
                ,int> = 0>
    explicit operator T () const { return getInt (); }

    explicit operator bool () const { return getBool(); }

    template<class T, std::enable_if_t<std::is_floating_point_v<typename std::decay<T>::type>,int> = 0>
    explicit operator T () const { return getFp (); }

    explicit operator std::string () const { return getStr (); }
    explicit operator const char* () const { return getStr ().c_str (); }

    bool isInt() const { return type == Type::Int; }
    bool isFp() const { return type == Type::Fp; }
    bool isBool() const { return type == Type::Bool; }
    bool isString() const { return type == Type::String; }

    template<class T>
    Param& operator=(const T& v) {
        set(v);
        return *this;
    }

    Param() = default;

    template<class T>
    Param(
        std::string name_,
        std::string desc_,
        SourceSpec srcSpec_,
        const T& defaultV_,
        ValueCallback valueCallback_ = [](Param&){return true;},
        std::string valueCallbackErrStr_ = ""
    ) :
        name(name_),
        desc(desc_),
        srcSpec(srcSpec_),
        valueCallback(valueCallback_),
        valueCallbackErrStr (valueCallbackErrStr_)
    {
        addToRegistry ();
        set_<kSetDefaults> (defaultV_);
        setFromSources();
    }

}; // class Param

#define CFG_PARAM(n,...) GFX_EMU_API GfxEmu::Cfg::Param& n();
#include "emu_cfg_params.h"
#undef CFG_PARAM

}; // namespace Cfg

}; // namespace GfxEmu
