Goodbye, Big Slow! 
====================

This kernel extension (kext) is based on [NoBatteryNoProblem.kext](https://github.com/balecrim/NoBatteryNoProblem.kext) and lets you disable the **BD PROCHOT** thermal throttling on *some* ***Intel Core*** processors running MacOS ***Big Sur***.  (Welcome to share your story on other systems.)

This kext implements modification of an *undocumented* [MSR (Model-Specific Register)](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html#Intel®_64_and_IA-32_Architectures_Software_Developer's_Manual) bit responsible for [BD PROCHOT (Bi-Directional Processor Hot)](https://www.intel.com/content/www/us/en/products/docs/processors/core/core-technical-resources.html) which is a signal used by peripherals to warn the CPU that they're running hot and thus initiate thermal throttling.  Apple™ uses this in their laptops to throttle very, VERY agressively their CPUs when the laptops are running with a missing/damaged/dead battery or other hardware faults.

Unlike [DisableTurboBoost.kext](https://github.com/nanoant/DisableTurboBoost.kext), by default this kext does not fuss with Intel Turbo Boost Technology which dynamically increases the processor's frequency as needed by taking advantage of thermal and power headroom to give you a burst of speed when you need it, and increased energy efficiency when you don't.  Disabling this functionality might prevent your computer from shutting down due to surges of voltage change.

You may have also noticed other slowdowns by background processes like `syspolicyd`, `trustd`, `taskgated`, `MRT`, `WindowServer`, `VTDecoderXPCService` but this kext is not going to solve them.

Prerequisites
---------------

* An administrator account on your system.

* Command Line Tools for [Xcode](https://developer.apple.com/technologies/tools/) are required to compile this module.

Command Line Tools are available as extra add-on from `Preferences > Downloads` of Xcode 4.3 or newer, installer option on Xcode 4.2 or older or [separate download](https://developer.apple.com/downloads).  ***Separate download does not require the monstrous Xcode toolchain (up to ~20 gigabytes) to build this project.***

Installation
--------------

For the impatient: skip step 2-4 by downloading [the pre-compiled kext](https://github.com/jakwings/GoodbyeBigSlow.kext/releases) and putting it into `/Library/Extensions`.

1. Run `csrutil disable` or `csrutil enable --without kext` in [Recovery Mode](https://support.apple.com/kb/HT201314).
2. Run `git clone https://github.com/jakwings/GoodbyeBigSlow.kext ; cd ./GoodbyeBigSlow.kext`
3. Run `make` to build the kext bundle.
4. Run `make uninstall` to unload the old kext (if there is any) from the OS.
5. Run `make install` to load this kext which will try to de-assert `PROCHOT`.
6. Remember to check `System Preferences > Security & Privacy > General > Allow System software from developer "Unidentified - GoodbyeBigSlow"`.
7. Shut down the computer.
8. Reset the [System Management Controller (SMC)](https://support.apple.com/kb/HT201295).
9. Reboot the computer and optionally run `sudo pmset -a hibernatemode 0 standby 0 autopoweroff 0` to prevent system sleep.
10. If the kext causes kernel panic, in Recovery Mode run `kmutil trigger-panic-medic` to unapprove this kext, or remove the kext (may need to `diskutil apfs unlockVolume` if FileVault is enabled).

Repeat step 7-9 if throttling reoccurs in the future.

If desired, Intel Turbo Boost Technology can be disabled by modifying boot-args in Recovery Mode:

```
# show the current boot-args (may not exist)
$ nvram boot-args
nvram: Error getting variable - 'boot-args': (iokit/common) data was not found
# append GoodbyeBigSlow=-turbo to the original values
$ nvram boot-args="...original values... GoodbyeBigSlow=-turbo"
# show the updated boot-args
$ nvram boot-args
```

In the same way, to disable Enhanced Intel SpeedStep Technology (management of processor power consumption via performance state transitions):

```
# append GoodbyeBigSlow=-speedstep to the original values
$ nvram boot-args="...original values... GoodbyeBigSlow=-speedstep"
# or in addition to disabling Turbo Boost (values separated by ":")
$ nvram boot-args="...original values... GoodbyeBigSlow=-turbo:-speedstep"
```

Please do not modify `/System/Library/Extensions/IOPlatformPluginFamily.kext` unless you are really desperate, I don't do that while developing this program.

After installation, the MSR modification will happen **at boot time only** unless you manually reload this kext:

    sudo kextunload -v 4 -b jakwings.kext.GoodbyeBigSlow
    sudo kextload   -v 4 -b jakwings.kext.GoodbyeBigSlow

To build for other versions of Mac OS, try passing `MACOS_VERSION_MIN` to `make`:

    make MACOS_VERSION_MIN=10.7  # untested; only for x86-64

You don't have to remember uninstalling this kext after transferring your data to non-Intel based Apple™ hardware because it recognizes and instructs only Intel CPUs.  Your system may not even load this kext due to missing compiled data for the new architecture (Apple Silicon Macs).

Diagnostics
-------------

To see whether the kext is successfully loaded:

```
$ kextstat -a -b jakwings.kext.GoodbyeBigSlow
...
Index Refs Address            Size       Wired      Architecture       Name (Version) UUID <Linked Against>
  154    0 0xffffff7f9cd60000 0x1000     0x1000     x86_64             jakwings.kext.GoodbyeBigSlow (2022.5.31) F47EE514-72F5-382F-AB54-7FE2A3003277 <8 5 3>
...
```

To view the log messages on system boot, [keep holding Command-V before you do](https://support.apple.com/kb/HT201255).

To view the log messages when you have logged in:

```
$ log show --predicate '(sender == "GoodbyeBigSlow")' --style syslog --info --debug --source --timezone UTC
...
Timestamp                       (process)[PID]
2022-05-31 09:52:09.369368+0000  localhost kernel[0]: (GoodbyeBigSlow) <GoodbyeBigSlow`GoodbyeBigSlow::init(OSDictionary*)> [GoodbyeBigSlow] Initializing ...
2022-05-31 09:52:09.369390+0000  localhost kernel[0]: (GoodbyeBigSlow) <GoodbyeBigSlow`GoodbyeBigSlow::init(OSDictionary*)> [GoodbyeBigSlow] Initializing ... Success
2022-05-31 09:52:09.369446+0000  localhost kernel[0]: (GoodbyeBigSlow) <GoodbyeBigSlow`GoodbyeBigSlow::probe(IOService*, int*)> [GoodbyeBigSlow] Probing ...
2022-05-31 09:52:09.369456+0000  localhost kernel[0]: (GoodbyeBigSlow) <GoodbyeBigSlow`GoodbyeBigSlow::probe(IOService*, int*)> [GoodbyeBigSlow] Probing ... Success
2022-05-31 09:52:09.369874+0000  localhost kernel[0]: (GoodbyeBigSlow) <GoodbyeBigSlow`GoodbyeBigSlow::start(IOService*)> [GoodbyeBigSlow] Starting ...
2022-05-31 09:52:09.372283+0000  localhost kernel[0]: (GoodbyeBigSlow) <GoodbyeBigSlow`GoodbyeBigSlow::start(IOService*)> [GoodbyeBigSlow] De-asserting Processor Hot ...
2022-05-31 09:52:09.373317+0000  localhost kernel[0]: (GoodbyeBigSlow) <GoodbyeBigSlow`GoodbyeBigSlow::start(IOService*)> [GoodbyeBigSlow] De-asserting Processor Hot ... Success
2022-05-31 09:52:09.373321+0000  localhost kernel[0]: (GoodbyeBigSlow) <GoodbyeBigSlow`GoodbyeBigSlow::start(IOService*)> [GoodbyeBigSlow] Starting ... Success
2022-05-31 10:06:49.814212+0800  localhost kernel[0]: (GoodbyeBigSlow) <GoodbyeBigSlow`GoodbyeBigSlow::stop(IOService*)> [GoodbyeBigSlow] Stopping ...
2022-05-31 10:06:49.814313+0800  localhost kernel[0]: (GoodbyeBigSlow) <GoodbyeBigSlow`GoodbyeBigSlow::free()> [GoodbyeBigSlow] Freeing ...
...

# to view more verbose logs
$ log show --predicate '(eventMessage CONTAINS "GoodbyeBigSlow") OR (process == "kernelmanagerd") OR (process == "kcgend")' --style syslog --info --debug --source --timezone UTC
...
[omitted here]
...
```

To check the soft or hard power limits imposed by the system:

```
$ sh "$THIS_REPO/other/show_plimits.sh"
hw.busfrequency = 100000000
machdep.xcpm.hard_plimit_max_100mhz_ratio = 30
machdep.xcpm.hard_plimit_min_100mhz_ratio = 8
machdep.xcpm.soft_plimit_min_100mhz_ratio = 8
machdep.xcpm.soft_plimit_max_100mhz_ratio = 30
[NOTE] The actual processor speed may be higher.
PowerStatus.CPU.Time = 100%
PowerStatus.CPU.Speed = 100%
PowerStatus.CPU.Available = 8/8
[NOTE] The actual clock rate may be higher.
PLimits.Version = 3
PLimits.CPU.now = P0 (3000MHz)
PLimits.CPU.min = P22 (800MHz)
PLimits.CPU.max = P0 (3000MHz)
PLimits.iGPU.now = P0
PLimits.iGPU.min = P19
PLimits.iGPU.max = P0
PLimits.iGPU.SingleSlice.min = P19
PLimits.iGPU.SingleSlice.max = P0
PLimits.Idle.now = P0
PLimits.Idle.min = P100
PLimits.Idle.max = P0
[NOTE] The normal performance state is P0.
```

To check whether your CPU has been successfully unthrottled, install [Intel Power Gadget](https://www.intel.com/content/www/us/en/developer/articles/tool/power-gadget.html) (for MacOS 10.11 and later) and watch the stats:

![statistics of working cpu](other/cpu-stats.png)

To display the stats on the menu bar, install [MenuMeter](https://github.com/yujitach/MenuMeters) (with auto-update off) instead of some spyware-like system monitoring tools like [Stats](https://github.com/jakwings/exelban-stats-no-aggressive-user-data-collection).[app](https://github.com/exelban/stats/issues/714) [¹](https://github.com/exelban/stats/pull/858) [²](https://github.com/exelban/stats/pull/742) [³](https://github.com/exelban/stats/commit/08d8d84cebf9078d7692999c243386c887d6ee14) [⁴](https://github.com/exelban/stats/commit/c5c4e4df3db0737b749ea91f903c8cf0f1ecd6aa#data_still_sent_despite_--omit).  Or you can try my fork without the stealthy behavior: https://github.com/jakwings/mac-stats/releases

To find out whether your CPU has the MSR `MSR_POWER_CTL = 1FCH` if the installer is out of date:

1.  retrieve the specifications of your CPU [here](https://ark.intel.com/content/www/us/en/ark/search/featurefilter.html)
2.  download the PDF document "[Intel® 64 and IA-32 architectures software developer's manual volume 4: Model-specific registers](https://cdrdv2.intel.com/v1/dl/getContent/671098)" from https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
3.  open the PDF document, locate at the section ("2.x MSRs in ...") that matches your CPU architecture and read very carefully to which table(s) it refers and see if MSR with address `1FCH` is listed there
4.  this is all for not halting your computer by invalid read/write operation on the MSR
5.  for other CPUs, try https://github.com/calasanmarko/TurboMac (try manual install without modifying [/System](https://support.apple.com/guide/security/signed-system-volume-security-secd698747c9) first) or https://apple.stackexchange.com/

Last, a quote from https://apple.stackexchange.com/a/393369 "Why does a MacBook throttle without a battery?":

> The CPU and GPU can draw more power than the AC adapter can provide as the battery serves as a capacitor and reserve. It smoothes out the voltage when a surge is needed. If the system over draws, voltage drops a little and subtle <ins>computing errors happen</ins>. If the voltage drops too far, the <ins>system shuts down entirely and abruptly</ins>.

When using GoodbyeBigSlow.kext, it is strongly recommended to monitor power consumption at the wall with a Kill-a-Watt meter or similar device and make sure that you don't exceed the power capabilities of your power adapter.  Use of GoodbyeBigSlow.kext to bypass this throttling scheme is at your own risk and can result in permanent damage to your power adapter or computer or both which may not be covered by your warranty.

License
---------

Copyright (c) 2012 [Adam Strzelecki](https://github.com/nanoant/DisableTurboBoost.kext)

Copyright (c) 2015 [Bernardo Alecrim](https://github.com/balecrim/NoBatteryNoProblem.kext)

Copyright (c) 2022 [Jak.W](https://github.com/jakwings/GoodbyeBigSlow.kext)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
