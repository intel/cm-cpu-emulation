CFG_PARAM(LogFile,
    "log file",
    "a file to where log messages shall be redirected",
    {"EMU_LOG_FILE", ""},
    "",
    [](auto& p){
        if(!p.isSettingDefaults ()) {
            if(std::regex_search(
                p.getStr (),
                std::regex{"\\w"}
            )) {
                GfxEmu::Log::setLogFile(p.getStr ());
            } else
                GFX_EMU_FAIL_WITH_MESSAGE(fCfg, "bad log file name %s containing no word charachters.\n",
                    p.getStr ().c_str());
        }
        return true;
    }
);

CFG_PARAM( LogChannels,
    "log channels",
    "enable/disable logging topic channels",
    {"EMU_LOG_CHANNELS",""},
    GfxEmu::CfgCache::LogChannels.load(),
    [](auto& p){
        GfxEmu::CfgCache::LogChannels.store(
            GfxEmu::Cfg::updateLogCfg(p, false)
        );
        return true;
    }
);

CFG_PARAM( LogLevel,
    "log level",
    "set minimum message importance level",
    {"EMU_LOG_LEVEL",""},
    GfxEmu::CfgCache::MinimalLevel.load(),
    [](auto& p){
        GfxEmu::CfgCache::MinimalLevel.store(
            GfxEmu::Cfg::updateLogCfg(p, true)
        );
        return true;
    }
);

CFG_PARAM( LogWarnings,
    "log warnings",
    "enable warnings in logging output.",
    {"EMU_LOG_WARNINGS",""},
    false
);

CFG_PARAM( Platform,
    "HW platform",
    "",
    {"CM_RT_PLATFORM",""},
    "SKL",
    [](auto& p) {
        p.set(GfxEmu::Utils::toLower(p.getStr ()));
        const auto it = GfxEmu::Platform::StaticData_nameToInt ().find(GfxEmu::Utils::toUpper(p.getStr ()));
        if(it == GfxEmu::Platform::StaticData_nameToInt ().end ()) {
            p.setSubValue(GfxEmu::Platform::UNDEFINED);
            return false;
        }
        p.setSubValue(it->second);
        return true;
    },
    "specified platform is unknown"
);

CFG_PARAM( Sku,
    "SKU name",
    "",
    {"CM_RT_SKU",""},
    "undefined",
    [](auto& p) {
        p.set(GfxEmu::Utils::toLower(p.getStr ()));
        const auto it = GfxEmu::Platform::Sku::StaticData_nameToInt ().find(GfxEmu::Utils::toUpper(p.getStr ()));
        if(it == GfxEmu::Platform::Sku::StaticData_nameToInt ().end ()) {
            p.setSubValue(GfxEmu::Platform::Sku::UNDEFINED);
            return p.isSettingDefaults () ? true : false;
        }
        p.setSubValue(it->second);
        return true;
    },
    "specified sku unknown"
);

CFG_PARAM( ParallelThreads,
    "parallel work-items (kernel threads) limit",

    "Controls how many work-items shall be allowed to be in running state at the same time"

    "In multi-thread work-items scheduling mode "
    "the value of 1 is a special case in which kernel threads shall be scheduled in predefined sequential order."

    , {"CM_RT_PARALLEL_THREADS","--emu-parallel-threads"},
    std::thread::hardware_concurrency(),
    [](auto& p) {
        if(p.getInt () > std::thread::hardware_concurrency())
            GFX_EMU_MESSAGE(fCfg,
                "notice: there are less HW cores available (%u) than requested to run parallel OS threads (%u). "
                "This is not advised as will lead to a poorer performance.\n",
                    std::thread::hardware_concurrency(), p.getInt ());
        return p.getInt() > 0;
    },
    "parallel threads number must be > 0"
);

CFG_PARAM( ResidentGroups,
    "resident work-groups limit",
    "how many work-group resources shall be allocated for parallel/concurrent execution",
    {"CM_RT_RESIDENT_GROUPS","--emu-resident-groups"},
    1,
    [](auto& p) {return p.getInt() > 0;},
    "resident groups number must be > 0"
);

CFG_PARAM( RetainTmpFiles,
    "retain tmp files",
    "retain tmp files",
    {"EMU_RETAIN_TMP_FILES", ""},
    false
);

CFG_PARAM( BacktraceOnTermination,
    "always print backtrace on error termination",
    "always print backtrace on error termination",
    {"EMU_BACKTRACE_ON_TERMINATION", ""},
    false
);
