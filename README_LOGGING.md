# EMU message logging and debug API.

 Table of contents

- [EMU message logging and debug API.](#emu-message-logging-and-debug-api)
  - [Message topic channels, importance levels, minimal importance level.](#message-topic-channels-importance-levels-minimal-importance-level)
  - [API usage examples and output:](#api-usage-examples-and-output)
    - [Trigger debug breakpoint](#trigger-debug-breakpoint)
    - [Output backtrace](#output-backtrace)
    - [Assertions [in Debug build only]](#assertions-in-debug-build-only)
    - [Message output APIs have general form of](#message-output-apis-have-general-form-of)
    - [General (Release + Debug builds) messages.](#general-release--debug-builds-messages)
    - [Errors and program termination.](#errors-and-program-termination)
    - [Program termination with error printout](#program-termination-with-error-printout)
    - [Setting common message prefix for a program lexical scope.](#setting-common-message-prefix-for-a-program-lexical-scope)
    - [Debug build only messages](#debug-build-only-messages)

## Message topic channels, importance levels, minimal importance level.

API provides **common way to print logging messages** across EMU codebase.
Messages can (but not obligated to) be attributed to particular **topic channels** and certain **importance levels**.
Information about corresponding message topic channel and importance is being automatically prepended to a message while output.

**At the build- and runtime** a user can set particular topic channels to be enabled or disabled, also the minimal importance level of messages displayed can be set. See [common/emu_log_flags.h](./common/emu_log_flags.h) for compile-time settings,
see [runtime configuration readme](./README_CONFIG.md) for runtime configuration settings.

## API usage examples and output:

Include header:

>    #include <emu_log.h>

For flags constants (not necessary if using flag names inside GFX_EMU_... macros):

    using namespace GfxEmu::Log::Flags;

----
### Trigger debug breakpoint

   #include <emu_utils.h>

   GfxEmu::Utils::debugBreak ();
    

### Output backtrace

   GfxEmu::Log::printBacktrace ();


----
### Assertions [in Debug build only]
----

   GFX_EMU_ASSERT(condition);

   GFX_EMU_ASSERT_MESSAGE(condition, "Message %u", 42);

In case of failure message with additional filename and line number information printed. GfxEmu::Utils::debugBreak () is invoked.

----
### Message output APIs have general form of 
----

- MACRO ([FLAGS,] MESSAGE_FORMAT [, ARGS])
- MACRO_AT ([FLAGS,] MESSAGE_FORMAT [, ARGS]) // **provides file:line information**

Optional **FLAGS**, if present, specify **topic channel**-s and/or **importance level**, e.g.

**Examples**

    fShim
    fShim | fCritical

May also use multiple topic channels:

    fShim | fKernelLaunch
    fShim | fKernelLaunch | fCritical


**MESSAGE_FORMAT** is a printf-compatible format string.

Optional **ARGS** are for MESSAGE_FORMAT (as in printf also).

----
### General (Release + Debug builds) messages.
----

    GFX_EMU_MESSAGE([FLAGS,] MESSAGE_FORMAT [, ARGS])
    GFX_EMU_MESSAGE_AT([FLAGS,] MESSAGE_FORMAT [, ARGS])

**Example:** 

    GFX_EMU_MESSAGE("message %u\n", 42);

**Output:** 

    message 42.

----
#### Warnings (Release + Debug builds, always displayed).
----

    GFX_EMU_WARNING_MESSAGE([FLAGS,] MESSAGE_FORMAT [, ARGS])
    GFX_EMU_WARNING_MESSAGE_AT([FLAGS,] MESSAGE_FORMAT [, ARGS])

**Example:** 

    GFX_EMU_WARNING_MESSAGE(fShim, "message %u\n", 42);

**Output:**

    [shim] *** Warning: message 42.

----
### Errors and program termination.
----

    GFX_EMU_ERROR_MESSAGE([FLAGS,] MESSAGE_FORMAT [, ARGS])
    GFX_EMU_ERROR_MESSAGE_AT([FLAGS,] MESSAGE_FORMAT [, ARGS])

**Example:**

    GFX_EMU_ERROR_MESSAGE("message %u.\n", 42);

**Output:**

    *** Error (at <file>:<line>): message 42.

**Example**

    GFX_EMU_ERROR_MESSAGE(fShim, "message %u.\n", 42);

**Output:**

    [shim] *** Error (at <file>:<line>): message 42.

----
### Program termination with error and backtrace printout

     GFX_EMU_FAIL_WITH_MESSAGE ("message %u.\n", 42);

**Output:**

    *** Error: message 42.
    <finishes execution, prints backtrace if EMU_BACKTRACE_ON_TERMINATION env variable is set>

----
### Setting common message prefix for a program lexical scope.
----

    void func()
    {
        GFX_EMU_MESSAGE_SCOPE_PREFIX("<this string shall be appended to all the messages printed"
            " from the current scope> ");
        ...
    }

----
### Debug build only messages
----

    GFX_EMU_DEBUG_MESSAGE([FLAGS,] MESSAGE_FORMAT [, ARGS])
    GFX_EMU_DEBUG_MESSAGE_AT([FLAGS,] MESSAGE_FORMAT [, ARGS])
    
**Example**

    GFX_EMU_DEBUG_MESSAGE_AT("message %u.\n", 42);

**Output:**

    (at <file>:<line>) message 42.

**Example**

    GFX_EMU_DEBUG_MESSAGE_AT(fShim, "message %u.\n", 42);

**Output:**

    [shim] (at <file>:<line>) message 42.

**Example**

    GFX_EMU_DEBUG_MESSAGE_AT(fShim | fInfo, "message %u.\n", 42);

**Output:**

    [shim,info] (at <file>:<line>) message 42.

