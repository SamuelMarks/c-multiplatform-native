import os
import subprocess
import sys
import xml.etree.ElementTree as ET
from unittest.mock import MagicMock, mock_open, patch

import pytest

sys.path.insert(
    0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "scripts"))
)
import importlib

update_shields = importlib.import_module("update-shields")


def test_run_cmd_success():
    with patch("subprocess.check_call") as mock_call:
        update_shields.run_cmd(["echo", "hello"], cwd="/some/path")
        mock_call.assert_called_once_with(["echo", "hello"], cwd="/some/path")


def test_run_cmd_failure():
    with patch("subprocess.check_call") as mock_call:
        mock_call.side_effect = subprocess.CalledProcessError(1, "echo")
        with pytest.raises(subprocess.CalledProcessError):
            update_shields.run_cmd(["echo", "hello"])


def test_ensure_coverage_data():
    with patch("update-shields.run_cmd") as mock_run_cmd:
        update_shields.ensure_coverage_data()
        assert mock_run_cmd.call_count == 3


def test_get_test_coverage_success():
    with patch("subprocess.run") as mock_run:
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = "some output\nlines: 85.5%\nmore output"
        mock_run.return_value = mock_result

        assert update_shields.get_test_coverage() == 85.5


def test_get_test_coverage_zero_lines_triggers_build():
    with (
        patch("subprocess.run") as mock_run,
        patch("update-shields.ensure_coverage_data") as mock_ensure,
    ):
        mock_res1 = MagicMock()
        mock_res1.returncode = 0
        mock_res1.stdout = "lines: 0.0% (0 out of 1129)\n"
        mock_res2 = MagicMock()
        mock_res2.returncode = 0
        mock_res2.stdout = "lines: 100.0% (1129 out of 1129)\n"
        mock_run.side_effect = [mock_res1, mock_res2]

        assert update_shields.get_test_coverage() == 100.0
        mock_ensure.assert_called_once()


def test_get_test_coverage_no_match():
    with patch("subprocess.run") as mock_run:
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = "some output\nno lines matched\nmore output"
        mock_run.return_value = mock_result

        assert update_shields.get_test_coverage() == 0.0


def test_get_test_coverage_failure():
    with patch("subprocess.run") as mock_run:
        mock_result = MagicMock()
        mock_result.returncode = 1
        mock_run.return_value = mock_result

        assert update_shields.get_test_coverage() == 0.0


def test_get_test_coverage_exception():
    with patch("subprocess.run") as mock_run:
        mock_run.side_effect = Exception("error")
        assert update_shields.get_test_coverage() == 0.0


def test_has_text_or_children():
    assert update_shields._has_text_or_children(None) is False
    el = ET.Element("brief")
    assert update_shields._has_text_or_children(el) is False
    el.text = "  some text  "
    assert update_shields._has_text_or_children(el) is True


@patch("update-shields.run_cmd")
@patch("glob.glob")
@patch("xml.etree.ElementTree.parse")
def test_get_doc_coverage_100_percent(mock_parse, mock_glob, mock_run_cmd):
    mock_glob.return_value = ["docs/xml/file1.xml"]

    mock_tree = MagicMock()
    mock_root = ET.Element("root")

    compounddef = ET.SubElement(mock_root, "compounddef", kind="class")
    brief = ET.SubElement(compounddef, "briefdescription")
    ET.SubElement(brief, "para")

    ET.SubElement(mock_root, "compounddef", kind="dir")

    memberdef = ET.SubElement(compounddef, "memberdef", kind="function")
    brief_mem = ET.SubElement(memberdef, "briefdescription")
    ET.SubElement(brief_mem, "para")

    enumdef = ET.SubElement(compounddef, "memberdef", kind="enum")
    brief_enum = ET.SubElement(enumdef, "briefdescription")
    ET.SubElement(brief_enum, "para")
    ev = ET.SubElement(enumdef, "enumvalue")
    ev_brief = ET.SubElement(ev, "briefdescription")
    ET.SubElement(ev_brief, "para")

    ET.SubElement(compounddef, "memberdef", kind="friend")

    mock_tree.getroot.return_value = mock_root
    mock_parse.return_value = mock_tree

    assert update_shields.get_doc_coverage() == 100.0


@patch("update-shields.run_cmd")
@patch("glob.glob")
@patch("xml.etree.ElementTree.parse")
def test_get_doc_coverage_partial_and_undoc_enum(mock_parse, mock_glob, mock_run_cmd):
    mock_glob.return_value = ["docs/xml/file1.xml"]

    mock_tree = MagicMock()
    mock_root = ET.Element("root")

    compounddef = ET.SubElement(mock_root, "compounddef", kind="struct")
    ET.SubElement(compounddef, "compoundname").text = "UndocStruct"
    ET.SubElement(compounddef, "briefdescription")
    ET.SubElement(compounddef, "detaileddescription")

    enumdef = ET.SubElement(compounddef, "memberdef", kind="enum")
    ET.SubElement(enumdef, "name").text = "MyEnum"
    brief_enum = ET.SubElement(enumdef, "briefdescription")
    ET.SubElement(brief_enum, "para")
    ev = ET.SubElement(enumdef, "enumvalue")
    ET.SubElement(ev, "briefdescription")
    ET.SubElement(ev, "detaileddescription")

    mock_tree.getroot.return_value = mock_root
    mock_parse.return_value = mock_tree

    assert update_shields.get_doc_coverage() == 0.0


@patch("update-shields.run_cmd")
@patch("glob.glob")
def test_get_doc_coverage_empty(mock_glob, mock_run_cmd):
    mock_glob.return_value = []
    assert update_shields.get_doc_coverage() == 100.0


@patch("update-shields.run_cmd")
@patch("glob.glob")
def test_get_doc_coverage_ignore_index(mock_glob, mock_run_cmd):
    mock_glob.return_value = ["docs/xml/index.xml"]
    assert update_shields.get_doc_coverage() == 100.0


@patch("update-shields.run_cmd")
def test_get_doc_coverage_exception(mock_run_cmd):
    mock_run_cmd.side_effect = Exception("doxygen failed")
    assert update_shields.get_doc_coverage() == 0.0


@patch("update-shields.run_cmd")
@patch("glob.glob")
@patch("xml.etree.ElementTree.parse")
def test_get_doc_coverage_parse_exception(mock_parse, mock_glob, mock_run_cmd):
    mock_glob.return_value = ["docs/xml/file1.xml"]
    mock_parse.side_effect = Exception("parse error")
    assert update_shields.get_doc_coverage() == 100.0


def test_get_color():
    assert update_shields.get_color(100.0) == "brightgreen"
    assert update_shields.get_color(90.0) == "brightgreen"
    assert update_shields.get_color(89.9) == "yellow"
    assert update_shields.get_color(75.0) == "yellow"
    assert update_shields.get_color(74.9) == "orange"
    assert update_shields.get_color(50.0) == "orange"
    assert update_shields.get_color(49.9) == "red"
    assert update_shields.get_color(0.0) == "red"


@patch("os.path.exists")
def test_update_readme_replace_existing_both(mock_exists):
    mock_exists.return_value = True
    content = "# Title\n\n![Test Coverage](https://img.shields.io/badge/Test_Coverage-10.0%25-red) ![Doc Coverage](https://img.shields.io/badge/Doc_Coverage-10.0%25-red)\nSome text"

    with patch("builtins.open", mock_open(read_data=content)) as m:
        update_shields.update_readme(85.0, 45.0)

        m.assert_called_with("README.md", "w")
        handle = m()
        written = "".join([c[0][0] for c in handle.write.call_args_list])
        assert (
            "![Test Coverage](https://img.shields.io/badge/Test_Coverage-85.0%25-yellow)"
            in written
        )
        assert (
            "![Doc Coverage](https://img.shields.io/badge/Doc_Coverage-45.0%25-red)"
            in written
        )
        assert "10.0" not in written


@patch("os.path.exists")
def test_update_readme_insert_after_header(mock_exists):
    mock_exists.return_value = True
    content = "# Title\n\nSome text"

    with patch("builtins.open", mock_open(read_data=content)) as m:
        update_shields.update_readme(95.0, 80.0)

        m.assert_called_with("README.md", "w")
        handle = m()
        written = "".join([c[0][0] for c in handle.write.call_args_list])
        assert (
            "# Title\n\n![Test Coverage](https://img.shields.io/badge/Test_Coverage-95.0%25-brightgreen) ![Doc Coverage](https://img.shields.io/badge/Doc_Coverage-80.0%25-yellow)\n\nSome text"
            in written
        )


@patch("os.path.exists")
def test_update_readme_insert_no_header(mock_exists):
    mock_exists.return_value = True
    content = "Just some text"

    with patch("builtins.open", mock_open(read_data=content)) as m:
        update_shields.update_readme(100.0, 100.0)

        m.assert_called_with("README.md", "w")
        handle = m()
        written = "".join([c[0][0] for c in handle.write.call_args_list])
        assert written.startswith(
            "![Test Coverage](https://img.shields.io/badge/Test_Coverage-100.0%25-brightgreen) ![Doc Coverage](https://img.shields.io/badge/Doc_Coverage-100.0%25-brightgreen)\n\nJust some text"
        )


@patch("os.path.exists")
def test_update_readme_missing_test_shield(mock_exists):
    mock_exists.return_value = True
    content = "# Title\n\n![Doc Coverage](https://img.shields.io/badge/Doc_Coverage-10.0%25-red)\nSome text"

    with patch("builtins.open", mock_open(read_data=content)) as m:
        update_shields.update_readme(85.0, 45.0)

        m.assert_called_with("README.md", "w")
        handle = m()
        written = "".join([c[0][0] for c in handle.write.call_args_list])
        assert (
            "![Doc Coverage](https://img.shields.io/badge/Doc_Coverage-45.0%25-red) ![Test Coverage](https://img.shields.io/badge/Test_Coverage-85.0%25-yellow)"
            in written
        )
        assert "10.0" not in written


@patch("os.path.exists")
def test_update_readme_missing_doc_shield(mock_exists):
    mock_exists.return_value = True
    content = "# Title\n\n![Test Coverage](https://img.shields.io/badge/Test_Coverage-10.0%25-red)\nSome text"

    with patch("builtins.open", mock_open(read_data=content)) as m:
        update_shields.update_readme(85.0, 45.0)

        m.assert_called_with("README.md", "w")
        handle = m()
        written = "".join([c[0][0] for c in handle.write.call_args_list])
        assert (
            "![Test Coverage](https://img.shields.io/badge/Test_Coverage-85.0%25-yellow) ![Doc Coverage](https://img.shields.io/badge/Doc_Coverage-45.0%25-red)"
            in written
        )
        assert "10.0" not in written


@patch("os.path.exists")
def test_update_readme_create_new(mock_exists):
    mock_exists.return_value = False

    with patch("builtins.open", mock_open(read_data="")) as m:
        update_shields.update_readme(50.0, 50.0)

        assert m.call_count == 3
        handle = m()
        written = "".join([c[0][0] for c in handle.write.call_args_list])
        assert (
            "![Test Coverage](https://img.shields.io/badge/Test_Coverage-50.0%25-orange)"
            in written
        )


@patch("update-shields.get_test_coverage")
@patch("update-shields.get_doc_coverage")
@patch("update-shields.update_readme")
def test_main(mock_update, mock_get_doc, mock_get_test):
    mock_get_test.return_value = 100.0
    mock_get_doc.return_value = 100.0

    update_shields.main()

    mock_update.assert_called_once_with(100.0, 100.0)


def test_main_execution():
    import runpy

    with (
        patch("subprocess.run") as mock_run,
        patch("subprocess.check_call"),
        patch("glob.glob", return_value=[]),
        patch("os.path.exists", return_value=False),
        patch("builtins.open", mock_open(read_data="# Title\n")),
    ):
        mock_run.return_value.returncode = 0
        mock_run.return_value.stdout = "lines: 100.0%\n"
        runpy.run_path(update_shields.__file__, run_name="__main__")
