"""Regression checks for distributable artifact path privacy on Windows/Linux."""
import importlib.util
from pathlib import Path, PurePosixPath, PureWindowsPath
import unittest

spec = importlib.util.spec_from_file_location(
    "firmware_build", Path(__file__).resolve().parents[1] / "tools/build.py")
build = importlib.util.module_from_spec(spec)
spec.loader.exec_module(build)


class ArtifactPathTests(unittest.TestCase):
    def check(self, data, toolchain="/usr", home="/home/runner"):
        build.verify_public_paths(data, "example.elf",
                                  checkout=PurePosixPath("/work/card"),
                                  user_home=PurePosixPath(home),
                                  toolchain=PurePosixPath(toolchain))

    def test_system_runtime_paths_are_allowed(self):
        for toolchain in ("/usr", "/usr/local", "/"):
            with self.subTest(toolchain=toolchain):
                self.check(b"/usr/lib/gcc/arm-none-eabi/runtime.c\x00"
                           b"/usr/local/lib/runtime.c\x00", toolchain=toolchain)

    def test_private_paths_are_rejected(self):
        for data in (b"/work/card/firmware/src/main.c\x00", b"/work/card\x00",
                     b"/home/runner/toolchain/include/stdint.h\x00",
                     b"/opt/private-gcc/lib/libgcc.a\x00"):
            with self.subTest(data=data), self.assertRaises(AssertionError):
                self.check(data, toolchain="/opt/private-gcc")

    def test_windows_paths_are_case_insensitive(self):
        for data in (b"c:/users/example/gcc/lib/libgcc.a\x00",
                     b"C:\\USERS\\EXAMPLE\\gcc\\lib\\libgcc.a\x00"):
            with self.subTest(data=data), self.assertRaises(AssertionError):
                build.verify_public_paths(data, "example.elf",
                                          checkout=PureWindowsPath("D:/work/card"),
                                          user_home=PureWindowsPath("C:/Users/Example"),
                                          toolchain=PureWindowsPath("C:/Users/Example/gcc"))

    def test_sibling_prefixes_and_root_home_are_allowed(self):
        self.check(b"/work/card-example/main.c\x00/home/runner-tools/lib.c\x00")
        self.check(b"/usr/lib/runtime.c\x00", home="/")


if __name__ == "__main__":
    unittest.main()
