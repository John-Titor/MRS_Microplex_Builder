MRS Microplex 7 Firmware Builder
================================
A wrapper around the (free) Freescale CodeWarrior for MCU using Wine for
non-Windows systems.


bootstrapping
=============

Wine setup
----------
A Wine install with the `wine` and `winetricks` commandline tools are required.
Homebrew users can `brew install wine winetricks`.

Configure wine with `winetricks mfc42`.

CW minimal install
------------------
Most of the bulk of the CW for MCU install is not required; it's possible to
just extract the toolchain and support files from the Eclipse installer
bundles.

Download the HCS08 updatesite bundle
`com.freescale.mcu11_1.hcs08_rs08.updatesite.zip` and unzip.

From the `binary` directory, unzip `com.freescale.hcs.buildtools_root_1.0.1`
and `com.freescale.hcs.architecture_root_1.0.9`.

Place the resulting MCU directory somewhere and point `CW_INSTALL_DIR` at it.

Note that `MCU_Build_Tools_Utilities.pdf` is not installed with this method.

CW full install
---------------
To install the full CW MCU application, download the CodeWarrior for MCU
Offline installer, e.g. `CW_MCU_v11.1_b181224_PE_Offline.exe` and run
`wine CW_MCU_v11.1_b181224_PE_Offline.exe` to install.

No need to install updates 1-4, as they have no relevant content for the MRS
CPU. There is no need to actually run CodeWarrior MCU; the HCS08 tools default
license is sufficient, and no setup is required.

The default CW_INSTALL_DIR value is set assuming this install method on a
default Wine prefix.

build tools
-----------
Building requires GNU Make 3.81 or newer.

flash tools
-----------
The build process produces an S-record file. This can be flashed using the
Microplex tools (requires a Windows host), or with [MRS Tools](https://github.com/John-Titor/MRS_tools).

how to use
==========

Create a branch. Make a directory `my_firmware` and put your application
sources in it. In `my_firmware/app.mk` set `APP_SRCS` to the list of source
files (relative to the top level) to build. 

Build with `make my_firmware`. The output S-records will be in
`build/my_firmware/my_firmware.sx`.

Additional firmwares can be hosted on the same branch by choosing another name
for the source directory; this will be reflected in the build path and output
filename.

Make changes to the framework on the mainline and rebase the app branch(es) as
appropriate.

app framework
-------------
Apps must implement the following functions, prototyped and documented in
`<app.h>`:

    void app_init(void)
    bool app_can_filter(HAL_can_message_t *msg)
    void app_can_receive(HAL_can_message_t *msg)
    void app_can_idle(bool did_idle)

The app must define the `app_main` protothread, which must yield regularly in
order for the framework to reset the watchdog and process incoming CAN messages.

    PT_DEFINE(app_main)
    {
        pt_begin(pt);
        for (;;) {
            <... app logic ...>
            pt_yield();
        }
        pt_end(pt);
    }

Additional protothreads may be declared / defined with `PT_DECLARE` and
`PT_DEFINE` respectively, and run from `app_main` with `PT_RUN`.

notes
=====

hardware
--------
See `include/HAL/7*.h` for hardware details and magic numbers.

args
----
 - compiler / linker options are in `resources/*.args`.
 - all sources build with the same args file
 - see documentation in `$(CW_INSTALL_DIR)/MCU/Help/PDF`, particularly
   `MCU_Build_Tools_Utilities.pdf` and `MCU_HCS08_compiler.pdf`.

linker file
-----------
The stock linker files don't work with the MRS ROM; the build will use
`resources/Microplex_7.prm` instead.

 - ROM is 0x2200 - 0xaf7f.
 - Branch directly to `_Startup` as there's no need to do any clock init.
 - More stack is nice.

code style
==========

See the `reformat` target in `Makefile`.

Comments and comment blocks should use C-style delimiters. Block comments
should be whole sentences; use capital letters and periods. One-liners
may be terse and un-capitalised.

Documentation goes in header files; only inline comments about the
implementation should be in the source.

Use `#pragma ONCE` instead of `#define` include guards.

Identifiers local to a file should be prefixed with `_`, unless another
rule applies.

Header include order:
 - Standard library headers
 - MCU headers
 - Local headers
 - HAL headers
 - Application headers

namespaces
----------
 - `HAL_`               Public HAL identifiers.
 - `_HAL_`              Private HAL identifiers (not to be referenced from app code).
 - `MRS_`               Public MRS bootrom identifiers.
 - `_MRS_`              Private MRS bootrom identifiers (not to be referenced from app code).
 - `app_`               Public app identifiers, to be supplied by the application.
 - `V<vector>_handler`  Interrupt handler for `<vector>`.

