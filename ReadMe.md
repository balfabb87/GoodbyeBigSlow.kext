Goodbye Big Slow
==================

This kernel extension is based on [DisableTurboBoost.kext](https://github.com/nanoant/DisableTurboBoost.kext) and lets you disable [Intel Turbo Boost Technology](http://www.intel.com/content/www/us/en/architecture-and-technology/turbo-boost/turbo-boost-technology.html) and BD Prochot functionality present on latest ***Intel*** Core processors running MacOS ***Big Sur***.

Abstract
----------

__Turbo Boost__ technology provides automatic CPU over-clocking in certain situations where there is no risk of overheating the CPU.  Unfortunately it has nasty effect of ruining various OpenCL and [OpenMP benchmark scores](http://openmp.org/forum/viewtopic.php?f=3&t=1289&p=5166&hilit=turbo+boost#p5166).

As observed Turbo Boost triggers when process is occupying 100% of single CPU core, while other CPU cores are idle.  However when all CPU cores are occupied, Turbo Boost doesn't trigger as it would cause CPU overheat.  As a result parallel tasks performance running on all CPU cores does not scale relatively to number of cores, i.e. for 4 core i5 CPU OpenMP program running on all 4 cores is only ~3x faster than 1 single core version.

Disabling Turbo Boost makes CPU run same clock regardless of cores occupation, therefore we get desired close to 4x speedup when running 4 cores OpenMP program vs 1 core.

This program implements modification of `MSR` CPU register responsible for Turbo Boost control as described in [Table 34-10. MSRs Supported by Intel Processors Based on Intel Microarchitecture Code Name Sandy Bridge (Contd.) - Intel® 64 and IA-32 Architectures Software Developer’s Manual Volume 3C](http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html).

__BD Prochot__ stands for Bi-directional Prochot, which is a signal used by peripherals to warn the CPU that they're running hot and thus initiate termal throttling.  Apple uses this in their laptops to throttle (very, VERY agressively) their CPUs when the laptops are running with a missing/dead battery or other hardware faults.

Prerequisites
---------------

[Xcode](https://developer.apple.com/technologies/tools/) with __Command Line Tools__ is required to compile this module.

Command Line Tools are available as extra add-on from `Preferences > Downloads` of Xcode 4.3 or newer, installer option on Xcode 4.2 or older or [separate download](https://developer.apple.com/downloads).  Separate download does now require Xcode to build this project.

Usage
-------

1. Run `csrutil disable` or `csrutil enable --without kext` in [recovery mode](https://support.apple.com/en-us/HT201314)
2. Run `make` to build the kext bundle
3. Run `make install` to load and disable both Turbo Boost and BD Prochot
4. Run `make uninstall` to unload and re-enable Turbo Boost and BD Prochot
5. Remember to check `System Preferences > Security & Privacy > General > Allow System software from developer "Unidentified - GoodbyeBigSlow"`

To check whether your CPU has been successfully unthrottled, install [Intel Power Gadget](https://www.intel.com/content/www/us/en/developer/articles/tool/power-gadget.html) and watch the stats:

![statistics of working cpu](other/cpu-stats.png)

When using GoodbyeBigSlow.kext, it is strongly recommended to monitor power consumption at the wall with a Kill-a-Watt meter or similar device and make sure that you don't exceed the power capabilities of your power adapter.  Use of GoodbyeBigSlow.kext to bypass these throttling schemes is at your own risk and can result in permanent damage to your power adapter or computer or both which may not be covered by your warranty.

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
