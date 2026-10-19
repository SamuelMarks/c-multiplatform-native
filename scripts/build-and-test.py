#!/usr/bin/env python3
"""Build and test orchestration script for c-multiplatform-native.

This module automates the configuration, compilation, and testing of the
c-multiplatform-native library across various platforms and toolchains
(macOS, Linux, Windows, MinGW, Cygwin, MSVC via Wine).
"""

import os
import sys
import subprocess
import shutil
from typing import Optional, List, Dict


def run_cmd(
    cmd: List[str], env: Optional[Dict[str, str]] = None, cwd: Optional[str] = None
) -> None:
    """Run a shell command as a subprocess.

    Args:
        cmd: A list of strings representing the command and its arguments.
        env: A dictionary of environment variables to pass to the subprocess.
             Defaults to None (inherits from the current process).
        cwd: The working directory in which to execute the command. Defaults to None.

    Raises:
        SystemExit: If the command returns a non-zero exit code.
    """
    print(f"Running: {' '.join(cmd)}")
    res = subprocess.run(cmd, env=env, cwd=cwd, text=True)
    if res.returncode != 0:
        print(f"Command failed with exit code {res.returncode}: {' '.join(cmd)}")
        sys.exit(res.returncode)


def is_tool(name: str) -> bool:
    """Check if a tool is available on the system PATH.

    Args:
        name: The name of the executable to search for.

    Returns:
        True if the tool is found, False otherwise.
    """
    return shutil.which(name) is not None


def check_mingw() -> bool:
    """Check if the system's gcc is actually MinGW.

    Returns:
        True if gcc is available and identifies as MinGW, False otherwise.
    """
    if not is_tool("gcc"):
        return False
    res = subprocess.run(["gcc", "-v"], capture_output=True, text=True)
    return "mingw" in res.stderr.lower()


def check_cygwin() -> bool:
    """Check if the system's gcc is actually Cygwin.

    Returns:
        True if gcc is available and identifies as Cygwin, False otherwise.
    """
    if not is_tool("gcc"):
        return False
    res = subprocess.run(["gcc", "-v"], capture_output=True, text=True)
    return "cygwin" in res.stderr.lower()


def get_msvc_wine_path() -> Optional[str]:
    """Discover the path to an MSVC installation for use with Wine.

    Checks the MSVC_WINE_PATH environment variable first, then attempts
    to find it in common local paths (e.g., ~/my_msvc, /opt/msvc).

    Returns:
        The path to the MSVC installation if found, None otherwise.
    """
    path = os.environ.get("MSVC_WINE_PATH")
    if path and os.path.exists(path):
        return path
    home = os.path.expanduser("~")
    candidates = [f"{home}/my_msvc", f"{home}/my_msvc/opt/msvc", "/opt/msvc"]
    for c in candidates:
        if os.path.isdir(f"{c}/bin/x64"):
            return c
    return None


def build_and_test(
    target_name: str, cmake_args: List[str], env: Optional[Dict[str, str]] = None
) -> None:
    """Configure, build, and test a specific CMake target.

    Args:
        target_name: The name of the target being built (used for the build directory).
        cmake_args: Additional arguments to pass to the CMake configuration step.
        env: A dictionary of environment variables to pass to the subprocesses.
    """
    build_dir = f"build_{target_name}"
    print(f"\n{'=' * 60}\nBuilding target: {target_name}\n{'=' * 60}")

    if env is not None:
        env = env.copy()
        for git_var in ("GIT_INDEX_FILE", "GIT_DIR", "GIT_WORK_TREE"):
            env.pop(git_var, None)

    cmd_config = [
        "cmake",
        "-B",
        build_dir,
        "-S",
        ".",
        "-DBUILD_TESTING=ON",
    ] + cmake_args
    run_cmd(cmd_config, env=env)

    for root_dir, _, files in os.walk(build_dir):
        for fn in files:
            if fn.endswith(".gcda"):
                os.remove(os.path.join(root_dir, fn))

    cmd_build = ["cmake", "--build", build_dir, "--parallel"]
    run_cmd(cmd_build, env=env)

    if env is None:
        test_env = os.environ.copy()
    else:
        test_env = env.copy()

    if target_name == "msvc_wine":
        # Prevent wine from hanging by disabling graphics drivers and debug logs
        test_env["WINEDLLOVERRIDES"] = "winemac.drv=d;x11drv=d"
        test_env["WINEDEBUG"] = "-all"
        test_env["DISPLAY"] = ""
    elif target_name == "appleclang":
        # Allow OOM tests to pass gracefully under AddressSanitizer
        test_env["ASAN_OPTIONS"] = "allocator_may_return_null=1"

    cmd_test = [
        "ctest",
        "--test-dir",
        build_dir,
        "--output-on-failure",
        "--timeout",
        "120",
    ]
    run_cmd(cmd_test, env=test_env)


def main() -> None:
    """Main execution block to autodetect OS and toolchains and run tests."""
    for git_var in ("GIT_INDEX_FILE", "GIT_DIR", "GIT_WORK_TREE"):
        os.environ.pop(git_var, None)

    is_windows = os.name == "nt"
    is_macos = sys.platform == "darwin"
    is_linux = not is_windows and not is_macos

    ran_any = False

    if is_windows and is_tool("cl.exe"):
        build_and_test("msvc_native", [])
        ran_any = True

    if not is_windows and is_tool("wine"):
        msvc_path = get_msvc_wine_path()
        if msvc_path:
            env = os.environ.copy()
            env["PATH"] = f"{msvc_path}/bin/x64:" + env.get("PATH", "")

            build_msvc_wine_sh = os.path.expanduser("~/repos/c-ci/build_msvc_wine.sh")
            if os.path.exists(build_msvc_wine_sh):
                for git_var in ("GIT_INDEX_FILE", "GIT_DIR", "GIT_WORK_TREE"):
                    env.pop(git_var, None)
                env.setdefault("PARALLEL_JOBS", "8")
                env.setdefault(
                    "WINEPREFIX", os.path.expanduser("~/.wine_cmultiplatform")
                )
                ninja_arg = " -GNinja" if is_tool("ninja") else ""
                msvc_cmd = [
                    "bash",
                    "-c",
                    f"bash {build_msvc_wine_sh}{ninja_arg} > wine_hook.log 2>&1; R=$?; cat wine_hook.log; rm -f wine_hook.log; exit $R",
                ]
                run_cmd(msvc_cmd, env=env)
                if is_tool("wineserver"):
                    subprocess.run(
                        ["wineserver", "-k9"],
                        env=env,
                        stdout=subprocess.DEVNULL,
                        stderr=subprocess.DEVNULL,
                    )
            else:
                winepath = f"{os.getcwd()}/build_msvc_wine;{msvc_path}/bin/x64;{msvc_path}/VC/Redist/MSVC/14.51.36231/debug_nonredist/x64/Microsoft.VC145.DebugCRT;{msvc_path}/Windows Kits/10/bin/10.0.26100.0/x64/ucrt"
                env["WINEPATH"] = winepath
                env["WINEDEBUG"] = "-all"
                env["WINE_AUTO_INSTALL"] = "0"

                build_and_test(
                    "msvc_wine",
                    [
                        "-DCMAKE_SYSTEM_NAME=Windows",
                        "-DCMAKE_C_COMPILER=cl",
                        "-DCMAKE_CXX_COMPILER=cl",
                        "-DCMAKE_CROSSCOMPILING_EMULATOR=wine",
                    ],
                    env=env,
                )
            ran_any = True

    if is_macos:
        env = os.environ.copy()
        if is_tool("clang"):
            env["CC"] = "clang"
            env["CXX"] = "clang++"
        env["ASAN_OPTIONS"] = "allocator_may_return_null=1"
        build_and_test(
            "appleclang",
            [
                "-DCMN_ENABLE_COVERAGE=ON",
                "-DCMN_ENABLE_ASAN=ON",
                "-DCMN_ENABLE_UBSAN=ON",
            ],
            env=env,
        )
        ran_any = True

    if is_linux:
        if is_tool("gcc"):
            env = os.environ.copy()
            env["CC"] = "gcc"
            env["CXX"] = "g++"
            build_and_test("gcc", ["-DCMN_ENABLE_COVERAGE=ON"], env=env)
            ran_any = True
        elif is_tool("clang"):
            env = os.environ.copy()
            env["CC"] = "clang"
            env["CXX"] = "clang++"
            build_and_test("clang", ["-DCMN_ENABLE_COVERAGE=ON"], env=env)
            ran_any = True

    if is_windows and check_mingw():
        env = os.environ.copy()
        env["CC"] = "gcc"
        env["CXX"] = "g++"
        build_and_test("mingw", ["-G", "MinGW Makefiles"], env=env)
        ran_any = True

    if is_windows and check_cygwin():
        env = os.environ.copy()
        env["CC"] = "gcc"
        env["CXX"] = "g++"
        build_and_test("cygwin", ["-G", "Unix Makefiles"], env=env)
        ran_any = True

    if not ran_any:
        print("No suitable toolchains found for your OS!")
        sys.exit(1)


if __name__ == "__main__":
    main()
