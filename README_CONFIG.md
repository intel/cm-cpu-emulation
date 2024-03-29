
# Configuration information.

- [Configuration information.](#configuration-information)
  - [Abnormal termination handling configuration.](#abnormal-termination-handling-configuration)
      - [ENV: EMU\_CATCH\_TERMINATING\_SIGNALS](#env-emu_catch_terminating_signals)
      - [ENV: EMU\_BACKTRACE\_ON\_TERMINATION](#env-emu_backtrace_on_termination)
    - [Logging configuration.](#logging-configuration)
      - [ENV: EMU\_LOG\_FILE](#env-emu_log_file)
      - [ENV: EMU\_LOG\_CHANNELS](#env-emu_log_channels)
      - [ENV: EMU\_LOG\_LEVEL](#env-emu_log_level)
    - [HW configuration choice.](#hw-configuration-choice)
      - [ENV: CM\_RT\_PLATFORM (string)](#env-cm_rt_platform-string)
      - [ENV: CM\_RT\_SKU (string)](#env-cm_rt_sku-string)
  - [Controls for kernel threads scheduling modes.](#controls-for-kernel-threads-scheduling-modes)
  - [Kernel threads as operating system threads mode.](#kernel-threads-as-operating-system-threads-mode)
      - [ENV: CM\_RT\_PARALLEL\_THREADS        // OS-threads mode semantics](#env-cm_rt_parallel_threads---------os-threads-mode-semantics)
      - [ENV: CM\_RT\_RESIDENT\_GROUPS         // OS-threads mode semantics](#env-cm_rt_resident_groups----------os-threads-mode-semantics)

### Abnormal termination handling configuration.

#### ENV: EMU_CATCH_TERMINATING_SIGNALS

(bool, detault: false)

Catch terminating signals. If enabled also sets EMU_BACKTRACE_ON_TERMINATION=true

#### ENV: EMU_BACKTRACE_ON_TERMINATION

(bool, detault: false)

Print backtrace on termination in case of an error.

### Logging configuration.

#### ENV: EMU_LOG_FILE 

(string, detault: "")

A file where log messages get redirected. File name must contain "word" characters.
By default log messages are displayed to stdout file stream.

#### ENV: EMU_LOG_CHANNELS

(string, default: ".")

Evaluated left to right comma-separated list of regex patterns matching channel names to be enabled
or disabled (when a pattern is prefixed with '~' character, see below for examples).

The list of logging channel names:

- config
- kernel support
- kernel launch
- dbg symb
- [see here for more](common/emu_log_flags.h)

> E.g. export EMU_LOG_CHANNELS="sched|kernel|dbg symb|~support" 
> will match the following debug channels: "sched", "kernel launch", "dbg symb".
> "kernel-support" channel shall be excluded from the output.

> E.g. export EMU_LOG_CHANNELS=".,~config" will output all the logging channel messages except for "config" channel. 

Warnings can be separately controlled with "warn" flag:

> E.g. export EMU_LOG_CHANNELS="warn" will enable warning messages on all the channels. 

NB: this setting will not affect sticky messages display.

#### ENV: EMU_LOG_LEVEL

(regex string to match minimal log level name, default: "info")

> E.g. export EMU_LOG_LEVEL="info" will match log level "info" so that only info-level messages are displayed. 

The list of logging levels:

- extra
- detail
- info
- [see here for more](common/emu_log_flags.h)

### HW configuration choice.

#### ENV: CM_RT_PLATFORM (string)

> HW platform parameters to use, e.g. "SKL"

#### ENV: CM_RT_SKU (string)

> Platform-specific SKU name, e.g. "GT1"

----
## Controls for kernel threads scheduling modes.

EMU provides the following modes of operation:

- a) Kernel threads (work-items) are scheduled as operating system-managed threads.

 
----

## Kernel threads as operating system threads mode.

----

During kernel run there shall be created up to **CM_RT_RESIDENT_GROUPS * \<TASK-SPECIFIC WORKGROUP SIZE>** operating system threads. **CM_RT_PARALLEL_THREADS** controls how many shall be allowed to be in running state simultaneously (=1 mode is specifically for debugging, see below). Operating system threads are being spawned in **lazy mode** (when first chosen by scheduler for execution) and run in detached state to occupy only the necessary amount of system resources, freeing those as soon as thread is complete.

#### ENV: CM_RT_PARALLEL_THREADS        // OS-threads mode semantics

(int, default: max hardware concurrency)

> Override the number of kernel threads allowed to be in running state simultaneously.
> **CM_RT_PARALLEL_THREADS=1** is a special case used **primarily for debugging purposes**. Kernel threads run (or resumed after barrier recycle) **in predefined sequential order in this mode**.

#### ENV: CM_RT_RESIDENT_GROUPS         // OS-threads mode semantics

(int, default: 1)

> Override the number of resident thread groups that shall be spawned during kernel execution.


