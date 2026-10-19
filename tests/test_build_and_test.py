import os
import sys
import subprocess
import shutil
import pytest
from unittest.mock import patch, MagicMock, call

# Ensure we can import the script
import importlib

build_and_test = importlib.import_module("build-and-test")


def test_run_cmd_success():
    with patch("subprocess.run") as mock_run:
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_run.return_value = mock_result

        build_and_test.run_cmd(["echo", "hello"])
        mock_run.assert_called_once_with(
            ["echo", "hello"], env=None, cwd=None, text=True
        )


def test_run_cmd_failure():
    with patch("subprocess.run") as mock_run, patch("sys.exit") as mock_exit:
        mock_result = MagicMock()
        mock_result.returncode = 1
        mock_run.return_value = mock_result

        build_and_test.run_cmd(["false"])
        mock_exit.assert_called_once_with(1)


def test_is_tool():
    with patch("shutil.which") as mock_which:
        mock_which.return_value = "/usr/bin/gcc"
        assert build_and_test.is_tool("gcc") is True

        mock_which.return_value = None
        assert build_and_test.is_tool("nonexistent") is False


def test_check_mingw():
    with patch("shutil.which") as mock_which, patch("subprocess.run") as mock_run:
        # Case: gcc not found
        mock_which.return_value = None
        assert build_and_test.check_mingw() is False

        # Case: gcc found, is mingw
        mock_which.return_value = "/usr/bin/gcc"
        mock_result = MagicMock()
        mock_result.stderr = "something mingw something"
        mock_run.return_value = mock_result
        assert build_and_test.check_mingw() is True

        # Case: gcc found, not mingw
        mock_result.stderr = "ubuntu linux gcc"
        assert build_and_test.check_mingw() is False


def test_check_cygwin():
    with patch("shutil.which") as mock_which, patch("subprocess.run") as mock_run:
        # Case: gcc not found
        mock_which.return_value = None
        assert build_and_test.check_cygwin() is False

        # Case: gcc found, is cygwin
        mock_which.return_value = "/usr/bin/gcc"
        mock_result = MagicMock()
        mock_result.stderr = "something cygwin something"
        mock_run.return_value = mock_result
        assert build_and_test.check_cygwin() is True

        # Case: gcc found, not cygwin
        mock_result.stderr = "ubuntu linux gcc"
        assert build_and_test.check_cygwin() is False


def test_get_msvc_wine_path():
    with (
        patch.dict(os.environ, {"MSVC_WINE_PATH": "/custom/path"}),
        patch("os.path.exists", return_value=True),
    ):
        assert build_and_test.get_msvc_wine_path() == "/custom/path"

    with patch.dict(os.environ, {}, clear=True), patch("os.path.isdir") as mock_isdir:
        # Case: found in first candidate
        def side_effect_isdir(path):
            return path.endswith("my_msvc/bin/x64")

        mock_isdir.side_effect = side_effect_isdir
        assert "my_msvc" in build_and_test.get_msvc_wine_path()

        # Case: found in second candidate
        def side_effect_isdir2(path):
            return path.endswith("my_msvc/opt/msvc/bin/x64")

        mock_isdir.side_effect = side_effect_isdir2
        assert "opt/msvc" in build_and_test.get_msvc_wine_path()

        # Case: found in third candidate
        def side_effect_isdir3(path):
            return path == "/opt/msvc/bin/x64"

        mock_isdir.side_effect = side_effect_isdir3
        assert build_and_test.get_msvc_wine_path() == "/opt/msvc"

        # Case: not found anywhere
        mock_isdir.side_effect = lambda x: False
        assert build_and_test.get_msvc_wine_path() is None


@patch("os.remove")
@patch("os.walk")
@patch("build-and-test.run_cmd")
def test_build_and_test(mock_run_cmd, mock_walk, mock_remove):
    mock_walk.return_value = [("build_mytarget", [], ["foo.gcda", "bar.o"])]
    build_and_test.build_and_test("mytarget", ["-DABC=1"], {"MY_ENV": "1"})
    mock_remove.assert_called_once_with(os.path.join("build_mytarget", "foo.gcda"))

    assert mock_run_cmd.call_count == 3
    calls = mock_run_cmd.call_args_list
    assert "cmake" in calls[0][0][0]
    assert "-DBUILD_TESTING=ON" in calls[0][0][0]
    assert "-DABC=1" in calls[0][0][0]
    assert calls[0][1]["env"] == {"MY_ENV": "1"}

    assert calls[1][0][0] == ["cmake", "--build", "build_mytarget", "--parallel"]
    assert calls[2][0][0] == [
        "ctest",
        "--test-dir",
        "build_mytarget",
        "--output-on-failure",
        "--timeout",
        "120",
    ]


@patch("os.walk", return_value=[])
@patch("build-and-test.run_cmd")
def test_build_and_test_variants(mock_run_cmd, mock_walk):
    build_and_test.build_and_test("msvc_wine", [], None)
    build_and_test.build_and_test("appleclang", [], None)
    assert mock_run_cmd.call_count == 6


@patch("build-and-test.build_and_test")
@patch("build-and-test.is_tool")
@patch("build-and-test.check_mingw")
@patch("build-and-test.check_cygwin")
@patch("build-and-test.get_msvc_wine_path")
@patch("build-and-test.os.name", "nt")
@patch("build-and-test.sys.platform", "win32")
def test_main_windows_msvc(
    mock_get_msvc_wine_path,
    mock_check_cygwin,
    mock_check_mingw,
    mock_is_tool,
    mock_build_and_test,
):
    # Windows native MSVC
    def is_tool_side_effect(tool):
        return tool == "cl.exe"

    mock_is_tool.side_effect = is_tool_side_effect
    mock_check_mingw.return_value = False
    mock_check_cygwin.return_value = False

    build_and_test.main()
    mock_build_and_test.assert_called_once_with("msvc_native", [])


@patch("build-and-test.build_and_test")
@patch("build-and-test.is_tool")
@patch("build-and-test.check_mingw")
@patch("build-and-test.check_cygwin")
@patch("build-and-test.get_msvc_wine_path")
@patch("build-and-test.os.name", "posix")
@patch("build-and-test.sys.platform", "linux")
@patch("os.path.exists", return_value=False)
@patch("os.getcwd", return_value="/work")
def test_main_linux_wine(
    mock_getcwd,
    mock_exists,
    mock_get_msvc_wine_path,
    mock_check_cygwin,
    mock_check_mingw,
    mock_is_tool,
    mock_build_and_test,
):
    def is_tool_side_effect(tool):
        return tool == "wine"

    mock_is_tool.side_effect = is_tool_side_effect
    mock_get_msvc_wine_path.return_value = "/opt/msvc"

    build_and_test.main()

    mock_build_and_test.assert_called_once()
    args = mock_build_and_test.call_args[0]
    kwargs = mock_build_and_test.call_args[1]
    assert args[0] == "msvc_wine"
    assert "-DCMAKE_CROSSCOMPILING_EMULATOR=wine" in args[1]
    assert "/opt/msvc/bin/x64" in kwargs["env"]["PATH"]
    assert kwargs["env"]["WINE_AUTO_INSTALL"] == "0"


@patch("build-and-test.run_cmd")
@patch("build-and-test.build_and_test")
@patch("build-and-test.is_tool")
@patch("build-and-test.check_mingw")
@patch("build-and-test.check_cygwin")
@patch("build-and-test.get_msvc_wine_path")
@patch("build-and-test.os.name", "posix")
@patch("build-and-test.sys.platform", "linux")
@patch("subprocess.run")
@patch("os.path.exists", return_value=True)
def test_main_linux_wine_with_script(
    mock_exists,
    mock_sub_run,
    mock_get_msvc_wine_path,
    mock_check_cygwin,
    mock_check_mingw,
    mock_is_tool,
    mock_build_and_test,
    mock_run_cmd,
):
    def is_tool_side_effect(tool):
        return tool in ("wine", "wineserver", "ninja")

    mock_is_tool.side_effect = is_tool_side_effect
    mock_get_msvc_wine_path.return_value = "/opt/msvc"

    build_and_test.main()
    mock_run_cmd.assert_called_once()
    mock_sub_run.assert_called_once()

    # Also test when wineserver/ninja are not installed and when msvc_path is None
    mock_is_tool.side_effect = lambda tool: tool in ("wine", "gcc")
    build_and_test.main()
    mock_get_msvc_wine_path.return_value = None
    build_and_test.main()


@patch("build-and-test.build_and_test")
@patch("build-and-test.is_tool")
@patch("build-and-test.os.name", "posix")
@patch("build-and-test.sys.platform", "darwin")
def test_main_macos(mock_is_tool, mock_build_and_test):
    def is_tool_side_effect(tool):
        return tool == "clang"

    mock_is_tool.side_effect = is_tool_side_effect

    build_and_test.main()
    mock_build_and_test.assert_called_once()
    assert mock_build_and_test.call_args[0][0] == "appleclang"
    assert "-DCMN_ENABLE_COVERAGE=ON" in mock_build_and_test.call_args[0][1]
    assert mock_build_and_test.call_args[1]["env"]["CC"] == "clang"


@patch("build-and-test.build_and_test")
@patch("build-and-test.is_tool")
@patch("build-and-test.os.name", "posix")
@patch("build-and-test.sys.platform", "linux")
def test_main_linux_gcc(mock_is_tool, mock_build_and_test):
    def is_tool_side_effect(tool):
        return tool == "gcc"

    mock_is_tool.side_effect = is_tool_side_effect

    build_and_test.main()
    mock_build_and_test.assert_called_once()
    assert mock_build_and_test.call_args[0][0] == "gcc"
    assert mock_build_and_test.call_args[1]["env"]["CC"] == "gcc"


@patch("build-and-test.build_and_test")
@patch("build-and-test.is_tool")
@patch("build-and-test.os.name", "posix")
@patch("build-and-test.sys.platform", "linux")
def test_main_linux_clang(mock_is_tool, mock_build_and_test):
    def is_tool_side_effect(tool):
        return tool == "clang"

    mock_is_tool.side_effect = is_tool_side_effect

    build_and_test.main()
    mock_build_and_test.assert_called_once()
    assert mock_build_and_test.call_args[0][0] == "clang"
    assert mock_build_and_test.call_args[1]["env"]["CC"] == "clang"


@patch("build-and-test.build_and_test")
@patch("build-and-test.is_tool")
@patch("build-and-test.check_mingw")
@patch("build-and-test.check_cygwin")
@patch("build-and-test.os.name", "nt")
@patch("build-and-test.sys.platform", "win32")
def test_main_windows_mingw(
    mock_check_cygwin, mock_check_mingw, mock_is_tool, mock_build_and_test
):
    mock_is_tool.return_value = False
    mock_check_mingw.return_value = True
    mock_check_cygwin.return_value = False

    build_and_test.main()
    mock_build_and_test.assert_called_once()
    assert mock_build_and_test.call_args[0][0] == "mingw"
    assert "-G" in mock_build_and_test.call_args[0][1]
    assert "MinGW Makefiles" in mock_build_and_test.call_args[0][1]
    assert mock_build_and_test.call_args[1]["env"]["CC"] == "gcc"


@patch("build-and-test.build_and_test")
@patch("build-and-test.is_tool")
@patch("build-and-test.check_mingw")
@patch("build-and-test.check_cygwin")
@patch("build-and-test.os.name", "nt")
@patch("build-and-test.sys.platform", "win32")
def test_main_windows_cygwin(
    mock_check_cygwin, mock_check_mingw, mock_is_tool, mock_build_and_test
):
    mock_is_tool.return_value = False
    mock_check_mingw.return_value = False
    mock_check_cygwin.return_value = True

    build_and_test.main()
    mock_build_and_test.assert_called_once()
    assert mock_build_and_test.call_args[0][0] == "cygwin"
    assert "-G" in mock_build_and_test.call_args[0][1]
    assert "Unix Makefiles" in mock_build_and_test.call_args[0][1]
    assert mock_build_and_test.call_args[1]["env"]["CC"] == "gcc"


@patch("build-and-test.is_tool")
@patch("build-and-test.check_mingw")
@patch("build-and-test.check_cygwin")
@patch("build-and-test.os.name", "unknown")
@patch("build-and-test.sys.platform", "unknown")
@patch("sys.exit")
def test_main_no_toolchains(
    mock_exit, mock_check_cygwin, mock_check_mingw, mock_is_tool
):
    mock_is_tool.return_value = False
    mock_check_mingw.return_value = False
    mock_check_cygwin.return_value = False

    build_and_test.main()
    mock_exit.assert_called_once_with(1)


@patch("build-and-test.main")
def test_main_execution(mock_main):
    import runpy

    with (
        patch("sys.exit"),
        patch("subprocess.run") as mock_run,
        patch("os.walk", return_value=[]),
    ):
        mock_run.return_value.returncode = 0
        with patch("shutil.which", return_value=None):
            runpy.run_path(build_and_test.__file__, run_name="__main__")
