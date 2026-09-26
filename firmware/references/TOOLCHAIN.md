# Build toolchain and runtime notices

The release was built offline with the existing GNU Tools for Arm Embedded Processors 9-2019-q4-major, GCC 9.2.1 (20191025), ARM/arm-9-branch revision 277599. Exact command flags and input hashes are in `../release/manifest.json`.

The project uses its own C source, minimal STM32G030 register map, startup assembly and linker script. No ST HAL or CMSIS source files are included. The compiler links its supplied newlib-nano routines and libgcc arithmetic helpers; the linker map identifies the pulled objects. No compiler installer is bundled.

`ARM_GNU_TOOLCHAIN_LICENSES.txt` is an unmodified copy of `share/doc/gcc-arm-none-eabi/license.txt` from that installed toolchain. It preserves the toolchain/runtime copyright notices and licenses, including the GCC Runtime Library Exception and newlib notices; its inclusion is not a declaration that all those component licenses apply to the project's original source.
