"""Offline build/test entry point. Python 3.9+, Arm GNU GCC, optional host C compiler.

Examples: python tools/build.py --test-host --build
          python tools/build.py --build --gcc-bin C:/path/to/bin
No downloads, hardware access, option-byte writes, or flashing occur here.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def public_text(value, bindir=None):
    """Keep distributable diagnostics independent of local checkout/user paths."""
    result = str(value).replace("\\", "/")
    replacements = [(ROOT.as_posix(), ".")]
    if bindir is not None:
        replacements.append((bindir.parent.as_posix(), "<arm-toolchain>"))
    for original, replacement in sorted(replacements, key=lambda item: -len(item[0])):
        result = re.sub(re.escape(original), lambda match: replacement, result,
                        flags=re.IGNORECASE if os.name == "nt" else 0)
    return result


def run(args, *, env=None, cwd=ROOT):
    result = subprocess.run([str(a) for a in args], cwd=cwd, env=env,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            text=True, errors="replace")
    print(result.stdout, end="", flush=True)
    if result.returncode:
        raise RuntimeError(f"Command failed ({result.returncode}): {args[0]}")
    return result.stdout


def gcc_bin(explicit):
    exe = "arm-none-eabi-gcc.exe" if os.name == "nt" else "arm-none-eabi-gcc"
    candidates = []
    if explicit:
        candidates.append(Path(explicit))
    elif os.environ.get("ARM_GCC_BIN"):
        candidates.append(Path(os.environ["ARM_GCC_BIN"]))
    else:
        found = shutil.which(exe)
        if found:
            candidates.append(Path(found).parent)
        if os.name == "nt":
            packages = Path(os.environ.get("LOCALAPPDATA", "")) / "Arduino15/packages"
            candidates.extend(sorted(packages.glob("*/tools/arm-none-eabi-gcc/*/bin"), reverse=True))
    for candidate in candidates:
        if (candidate / exe).is_file():
            return candidate
    raise RuntimeError("Arm GCC not found. Set ARM_GCC_BIN or pass --gcc-bin (bin directory).")


def source_manifest():
    files = []
    for folder in ("src", "include", "tests", "tools"):
        files.extend(p for p in (ROOT / folder).rglob("*")
                     if p.is_file() and p.suffix in (".c", ".h", ".S", ".py"))
    files.append(ROOT / "stm32g030k8.ld")
    return {str(p.relative_to(ROOT)).replace("\\", "/"): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in sorted(files)}


def verify_images(binary, hexfile, symbols):
    data = binary.read_bytes()
    def symbol(name):
        return symbols[name]
    assert 184 <= len(data) <= 65536, "Binary exceeds Flash or lacks vector table"
    stack, reset = struct.unpack_from("<II", data)
    systick = struct.unpack_from("<I", data, 15 * 4)[0]
    assert stack == symbol("_estack") == 0x20002000
    assert reset == symbol("Reset_Handler") | 1
    assert systick == symbol("SysTick_Handler") | 1
    assert 0x08000000 <= (reset & ~1) < 0x08000000 + len(data)
    assert 0x08000000 <= (systick & ~1) < 0x08000000 + len(data)
    assert symbol("_end") + 2048 <= stack
    assert symbol("_sdata") == 0x20000000 <= symbol("_edata") <= symbol("_sbss") <= symbol("_ebss") <= symbol("_end")
    assert "malloc" not in symbols, "Unexpected heap allocation"
    memory, upper, ended = {}, 0, False
    for line in hexfile.read_text().splitlines():
        assert line.startswith(":"), "Bad HEX record"
        rec = bytes.fromhex(line[1:])
        assert sum(rec) % 256 == 0 and len(rec) == rec[0] + 5, "Bad HEX checksum/length"
        count, offset, kind = rec[0], int.from_bytes(rec[1:3], "big"), rec[3]
        payload = rec[4:4 + count]
        assert not ended, "HEX data after EOF"
        if kind == 0:
            for i, value in enumerate(payload):
                address = upper + offset + i
                assert 0x08000000 <= address < 0x08010000 and address not in memory
                memory[address] = value
        elif kind == 4:
            assert count == 2
            upper = int.from_bytes(payload, "big") << 16
        elif kind == 1:
            ended = True
        elif kind == 5:
            assert count == 4
        else:
            raise AssertionError(f"Unexpected HEX record type {kind}")
    assert ended and min(memory) == 0x08000000
    assert max(memory) == 0x08000000 + len(data) - 1
    assert bytes(memory.get(0x08000000 + i, 0xff) for i in range(len(data))) == data
    return {"flash_image_bytes": len(data), "flash_capacity_bytes": 65536,
            "static_ram_bytes": symbol("_end") - 0x20000000,
            "ram_capacity_bytes": 8192, "reserved_stack_bytes": 2048,
            "initial_sp": f"0x{stack:08X}", "reset_vector": f"0x{reset:08X}",
            "systick_vector": f"0x{systick:08X}", "hex_bin_match": True}


def build(explicit):
    bindir = gcc_bin(explicit)
    suffix = ".exe" if os.name == "nt" else ""
    def tool(name):
        return bindir / ("arm-none-eabi-" + name + suffix)
    out = Path("build/target")
    release = Path("release")
    (ROOT / out).mkdir(parents=True, exist_ok=True)
    (ROOT / release).mkdir(exist_ok=True)
    version = run([tool("gcc"), "--version"]).splitlines()[0]
    common = ["-mcpu=cortex-m0plus", "-mthumb", "-mfloat-abi=soft", "-Os", "-g3",
              "-ffunction-sections", "-fdata-sections", "-fno-common", "-fstack-usage",
              "-Wall", "-Wextra", "-Werror", "-Wshadow", "-I", "include"]
    # Relative inputs keep compiler diagnostics and .su files portable. GCC also
    # records its working directory in DWARF, so map both Windows path styles.
    prefix_maps = []
    for directory, replacement in ((ROOT, "."), (bindir.parent, "arm-toolchain")):
        paths = {str(directory), directory.as_posix()}
        # Windows GCC lowercases paths to its built-in system headers.
        if os.name == "nt":
            paths.update(path.lower() for path in list(paths))
        prefix_maps.extend("-ffile-prefix-map=" + path + "=" + replacement
                           for path in sorted(paths))
    objects = []
    for name in ("main.c", "board.c", "buttons.c", "app.c", "snake.c", "is31fl3737.c", "startup.S"):
        obj = out / (Path(name).stem + ".o")
        flags = ["-std=c11"] if name.endswith(".c") else [
            "-Wa,--debug-prefix-map=" + path + "=." for path in
            sorted({str(ROOT), ROOT.as_posix()})]
        run([tool("gcc"), *common, *prefix_maps, *flags, "-c", Path("src") / name, "-o", obj])
        objects.append(obj)
    stem = release / "SnakeBusinessCard_RevB_v1.0.0"
    # Preserve the full dotted version in each artifact name.
    elf, binary, hexfile = (Path(str(stem) + ext) for ext in (".elf", ".bin", ".hex"))
    mapfile = Path(str(stem) + ".map")
    link = run([tool("gcc"), *common, "-nostartfiles", "--specs=nano.specs", "--specs=nosys.specs",
                *objects, "-T", "stm32g030k8.ld", "-Wl,--gc-sections",
                "-Wl,--print-memory-usage", "-Wl,-Map=" + str(mapfile), "-Wl,--cref",
                "-Wl,--start-group", "-lc", "-lgcc", "-lnosys", "-Wl,--end-group", "-o", elf])
    raw = (ROOT / elf).read_bytes()
    assert raw[:7] == b"\x7fELF\x01\x01\x01" and struct.unpack_from("<H", raw, 18)[0] == 40
    nm = subprocess.check_output([str(tool("nm")), "-n", str(elf)], text=True, cwd=ROOT)
    symbols = {parts[2]: int(parts[0], 16) for line in nm.splitlines()
               if len(parts := line.split()) == 3 and all(c in "0123456789abcdefABCDEF" for c in parts[0])}
    # Old binutils gives an empty .data section a RAM LMA. Exclude it so
    # --gap-fill cannot create a 384 MB Flash-to-RAM gap in the burn image.
    omit = ["--remove-section=.data"] if symbols["_sdata"] == symbols["_edata"] else []
    run([tool("objcopy"), *omit, "--gap-fill", "0xff", "-O", "binary", elf, binary])
    run([tool("objcopy"), *omit, "--gap-fill", "0xff", "-O", "ihex", elf, hexfile])
    size = run([tool("size"), elf])
    image = verify_images(ROOT / binary, ROOT / hexfile, symbols)
    # Linker maps expose the library installation location; use a documented
    # logical prefix while retaining object names, addresses and section sizes.
    map_path = ROOT / mapfile
    map_path.write_text(public_text(map_path.read_text(), bindir), encoding="utf-8")
    for artifact in (elf, binary, hexfile, mapfile):
        normalized = (ROOT / artifact).read_bytes().lower().replace(b"\\", b"/")
        for directory in (ROOT, bindir.parent):
            assert directory.as_posix().lower().encode() not in normalized, \
                "Local build path remained in distributable artifact: " + artifact.name
    stack_usage = "".join(public_text(p.read_text(), bindir) for p in sorted((ROOT / out).glob("*.su")))
    (ROOT / out / "stack_usage.txt").write_text(stack_usage, encoding="utf-8")
    manifest = {"version": "1.0.0", "board": "SnakeBusinessCard Rev B / STM32G030K8T6",
                "identity": "Alex Parker (fictional example)",
                "compiler": version, "build_flags": common,
                "debug_path_mapping": "checkout directory -> .; toolchain installation -> arm-toolchain",
                "map_path_mapping": "toolchain installation -> <arm-toolchain>",
                "c_standard": "c11", "linker_script": "stm32g030k8.ld",
                "link_specs": ["nano.specs", "nosys.specs", "-nostartfiles", "--gc-sections"],
                "image_verification": image, "source_sha256": source_manifest(),
                "artifacts": {p.name: {"bytes": (ROOT / p).stat().st_size, "sha256": hashlib.sha256((ROOT / p).read_bytes()).hexdigest()}
                              for p in (elf, binary, hexfile, mapfile)},
                "hardware_tested": False}
    (ROOT / release / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    (ROOT / release / "build_report.txt").write_text(public_text(version + "\n" + link + size + "\n" +
                                               json.dumps(image, indent=2) + "\n\nCompiler stack usage (bytes per function):\n" + stack_usage, bindir), encoding="utf-8")
    print("PASS: ARM image, Flash/RAM bounds, vector table, HEX checksums and HEX/BIN equivalence.")


def msvc_environment():
    env = dict(os.environ)
    if shutil.which("cl.exe"):
        return env
    base = Path(os.environ.get("ProgramFiles(x86)", "C:/Program Files (x86)"))
    vswhere = base / "Microsoft Visual Studio/Installer/vswhere.exe"
    candidates = []
    if vswhere.exists():
        found = subprocess.check_output([str(vswhere), "-latest", "-products", "*", "-requires",
                                         "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                                         "-property", "installationPath"], text=True).strip()
        if found:
            candidates.append(Path(found) / "VC/Auxiliary/Build/vcvars64.bat")
    candidates.append(base / "Microsoft Visual Studio/2022/BuildTools/VC/Auxiliary/Build/vcvars64.bat")
    for script in candidates:
        if script.exists():
            # cmd uses its own quote grammar, not the C argv grammar used by
            # subprocess.list2cmdline. Pass this fixed shell command directly.
            output = subprocess.check_output(f'cmd.exe /d /s /c "call "{script}" >nul && set"',
                                             env=env, text=True, errors="replace")
            for line in output.splitlines():
                if "=" in line and not line.startswith("="):
                    key, value = line.split("=", 1)
                    env[key.upper()] = value
            return env
    raise RuntimeError("MSVC not found; install C++ Build Tools or use a host GCC/Clang on PATH.")


def test_host(sanitize):
    out = ROOT / "build/host"
    out.mkdir(parents=True, exist_ok=True)
    tests = {"snake": ["snake.c"], "is31fl3737": ["is31fl3737.c"],
             "app": ["snake.c", "buttons.c", "app.c"]}
    host_cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
    msvc = os.name == "nt" and not host_cc
    env = msvc_environment() if msvc else dict(os.environ)
    if not msvc and not host_cc:
        raise RuntimeError("A native host C compiler is required for --test-host.")
    log = []
    for name, sources in tests.items():
        exe = out / ("test_" + name + (".exe" if os.name == "nt" else ""))
        files = [ROOT / "src" / source for source in sources] + [ROOT / "tests" / ("test_" + name + ".c")]
        if msvc:
            compiler = shutil.which("cl.exe", path=env.get("PATH"))
            if not compiler:
                raise RuntimeError("MSVC environment did not expose cl.exe.")
            command = [compiler, "/nologo", "/std:c11", "/W4", "/WX", "/Od", "/Zi",
                       "/I" + str(ROOT / "include"), *files, "/Fe:" + str(exe),
                       "/Fo:" + str(out) + "\\", "/Fd:" + str(out / (name + ".pdb"))]
            if sanitize:
                command.append("/fsanitize=address")
            command.extend(["/link", "/INCREMENTAL:NO"])
        else:
            command = [host_cc, "-std=c11", "-Wall", "-Wextra", "-Werror", "-g", "-O1",
                       "-I", ROOT / "include", *files, "-o", exe]
            if sanitize:
                command.extend(["-fsanitize=address,undefined", "-fno-omit-frame-pointer"])
        display_command = [Path(command[0]).name, *[public_text(x) for x in command[1:]]]
        log.append("COMMAND: " + subprocess.list2cmdline(display_command))
        log.append(public_text(run(command, env=env)))
        log.append(public_text(run([exe], env=env)))
    report = {"passed": True, "address_sanitizer": sanitize,
              "undefined_behavior_sanitizer": sanitize and not msvc,
              "compiler": "MSVC" if msvc else Path(host_cc).name,
              "tests": list(tests), "source_sha256": source_manifest()}
    for destination in (out, ROOT / "verification"):
        destination.mkdir(exist_ok=True)
        (destination / "test_report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        (destination / "test_report.txt").write_text("\n".join(log), encoding="utf-8")
    print("PASS: all portable host tests" + (" with AddressSanitizer." if sanitize else "."))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", action="store_true")
    parser.add_argument("--test-host", action="store_true")
    parser.add_argument("--sanitize", action="store_true", help="AddressSanitizer for host tests")
    parser.add_argument("--gcc-bin")
    args = parser.parse_args()
    try:
        if not args.build and not args.test_host:
            args.build = True
        if args.test_host:
            test_host(args.sanitize)
        if args.build:
            build(args.gcc_bin)
    except (RuntimeError, AssertionError, KeyError, OSError, subprocess.CalledProcessError) as error:
        print(f"FAILED: {error}", file=sys.stderr)
        sys.exit(1)
