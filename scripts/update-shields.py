"""Script to update README.md with test and documentation coverage shields.

This module runs `gcovr` and `doxygen` to calculate coverage metrics and
updates the `README.md` file with corresponding badges.
"""

import glob
import os
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from typing import Any, List


def run_cmd(cmd: List[str], **kwargs: Any) -> None:
    """Run a shell command as a subprocess.

    Args:
        cmd: A list of strings representing the command and its arguments.
        **kwargs: Additional keyword arguments passed to `subprocess.check_call`.

    Raises:
        subprocess.CalledProcessError: If the command returns a non-zero exit code.
    """
    print(f"Running: {' '.join(cmd)}")
    subprocess.check_call(cmd, **kwargs)


def ensure_coverage_data() -> None:
    """Build and run tests with coverage enabled to generate .gcda files."""
    build_dir = "build_appleclang" if sys.platform == "darwin" else "build_gcc"
    env = os.environ.copy()
    env["ASAN_OPTIONS"] = "allocator_may_return_null=1"
    run_cmd(
        [
            "cmake",
            "-B",
            build_dir,
            "-S",
            ".",
            "-DBUILD_TESTING=ON",
            "-DCMN_ENABLE_COVERAGE=ON",
        ],
        env=env,
    )
    run_cmd(["cmake", "--build", build_dir, "--parallel"], env=env)
    run_cmd(
        ["ctest", "--test-dir", build_dir, "--output-on-failure", "--timeout", "120"],
        env=env,
    )


def get_test_coverage() -> float:
    """Calculate the test coverage using gcovr.

    Runs `gcovr -s -r .` and parses the output for line coverage percentage.
    If no coverage data is present (`(0 out of `), builds and runs the test
    suite with coverage enabled before re-running `gcovr`.

    Returns:
        The test coverage as a float percentage (0.0 to 100.0).
        Returns 0.0 if gcovr fails or is not found.
    """
    gcovr_cmd = [
        "gcovr",
        "-s",
        "-r",
        ".",
        "-f",
        "src/.*",
        "-f",
        "include/.*",
        "-e",
        "src/backends/.*",
    ]
    try:
        result = subprocess.run(
            gcovr_cmd,
            capture_output=True,
            text=True,
        )
        if result.returncode == 0 and "(0 out of " in result.stdout:
            ensure_coverage_data()
            result = subprocess.run(
                gcovr_cmd,
                capture_output=True,
                text=True,
            )
        if result.returncode != 0:
            print("gcovr failed or not found, fallback to 0%")
            return 0.0

        match = re.search(r"lines:\s+([\d\.]+)%", result.stdout)
        if match:
            return float(match.group(1))
    except Exception as e:
        print(f"Error getting test coverage: {e}")
    return 0.0


def _has_text_or_children(elem: Any) -> bool:
    """Check whether an XML element has non-empty text or child elements.

    Args:
        elem: An ElementTree Element or None.

    Returns:
        True if the element has text or children, False otherwise.
    """
    if elem is None:
        return False
    if elem.text and elem.text.strip():
        return True
    return len(list(elem)) > 0


def get_doc_coverage() -> float:
    """Calculate the documentation coverage using Doxygen XML output.

    Runs `doxygen Doxyfile` and parses the resulting XML files in `docs/xml`
    to count documented vs. total entities (classes, structs, files, groups,
    members, enum values, and function parameters).

    Returns:
        The documentation coverage as a float percentage (0.0 to 100.0).
    """
    try:
        run_cmd(["doxygen", "Doxyfile"], cwd="docs")
        total = 0
        documented = 0

        xml_files = glob.glob("docs/xml/*.xml")
        for f in xml_files:
            if f.endswith("index.xml"):
                continue
            try:
                tree = ET.parse(f)
                root = tree.getroot()
                for compounddef in root.findall(".//compounddef"):
                    kind = compounddef.get("kind")
                    if kind in ["class", "struct", "file", "group"]:
                        total += 1
                        brief = compounddef.find("briefdescription")
                        deta = compounddef.find("detaileddescription")

                        if _has_text_or_children(brief) or _has_text_or_children(deta):
                            documented += 1
                        else:
                            name = compounddef.find("compoundname")
                            name_str = (
                                name.text
                                if name is not None and name.text
                                else "Unknown"
                            )
                            print(f"Undocumented compound: {name_str}")

                    for memberdef in compounddef.findall(".//memberdef"):
                        mkind = memberdef.get("kind")
                        if mkind == "friend":
                            continue
                        total += 1
                        brief = memberdef.find("briefdescription")
                        deta = memberdef.find("detaileddescription")

                        is_doc = _has_text_or_children(brief) or _has_text_or_children(
                            deta
                        )
                        if mkind == "enum":
                            for ev in memberdef.findall("enumvalue"):
                                ev_brief = ev.find("briefdescription")
                                ev_deta = ev.find("detaileddescription")
                                if not (
                                    _has_text_or_children(ev_brief)
                                    or _has_text_or_children(ev_deta)
                                ):
                                    is_doc = False
                        if is_doc:
                            documented += 1
                        else:
                            name = memberdef.find("name")
                            name_str = (
                                name.text
                                if name is not None and name.text
                                else "Unknown"
                            )
                            print(f"Undocumented member: {name_str}")
            except Exception:
                pass

        if total == 0:
            return 100.0
        return (documented / float(total)) * 100.0
    except Exception as e:
        print(f"Error getting doc coverage: {e}")
    return 0.0


def get_color(pct: float) -> str:
    """Determine the shield color based on coverage percentage.

    Args:
        pct: The coverage percentage (0.0 to 100.0).

    Returns:
        A string representing the color (e.g., 'brightgreen', 'yellow', 'orange', 'red').
    """
    if pct >= 90:
        return "brightgreen"
    if pct >= 75:
        return "yellow"
    if pct >= 50:
        return "orange"
    return "red"


def update_readme(test_cov: float, doc_cov: float) -> None:
    """Update the README.md file with coverage shields.

    Args:
        test_cov: The test coverage percentage.
        doc_cov: The documentation coverage percentage.
    """
    if not os.path.exists("README.md"):
        with open("README.md", "w") as f:
            f.write("# c-multiplatform-native\n\n")

    with open("README.md", "r") as f:
        content = f.read()

    test_color = get_color(test_cov)
    doc_color = get_color(doc_cov)

    test_shield = f"![Test Coverage](https://img.shields.io/badge/Test_Coverage-{test_cov:.1f}%25-{test_color})"
    doc_shield = f"![Doc Coverage](https://img.shields.io/badge/Doc_Coverage-{doc_cov:.1f}%25-{doc_color})"

    has_test_shield = re.search(
        r"!\[Test Coverage\]\(https://img\.shields\.io/badge/Test_Coverage-[^\)]+\)",
        content,
    )
    has_doc_shield = re.search(
        r"!\[Doc Coverage\]\(https://img\.shields\.io/badge/Doc_Coverage-[^\)]+\)",
        content,
    )

    if has_test_shield:
        content = re.sub(
            r"!\[Test Coverage\]\(https://img\.shields\.io/badge/Test_Coverage-[^\)]+\)",
            test_shield,
            content,
        )

    if has_doc_shield:
        content = re.sub(
            r"!\[Doc Coverage\]\(https://img\.shields\.io/badge/Doc_Coverage-[^\)]+\)",
            doc_shield,
            content,
        )

    if not has_test_shield and not has_doc_shield:
        shields_text = f"{test_shield} {doc_shield}\n"
        if re.search(r"^# .*?\n", content):
            content = re.sub(r"^(# .*?\n+)", r"\1" + shields_text + "\n", content)
        else:
            content = shields_text + "\n" + content
    elif not has_test_shield:
        content = re.sub(
            r"(!\[Doc Coverage\]\(https://img\.shields\.io/badge/Doc_Coverage-[^\)]+\))",
            r"\1 " + test_shield,
            content,
        )
    elif not has_doc_shield:
        content = re.sub(
            r"(!\[Test Coverage\]\(https://img\.shields\.io/badge/Test_Coverage-[^\)]+\))",
            r"\1 " + doc_shield,
            content,
        )

    with open("README.md", "w") as f:
        f.write(content)
    print("README.md updated with coverage shields.")


def main() -> None:
    """Main execution block to calculate coverage and update README.md."""
    test_cov = get_test_coverage()
    doc_cov = get_doc_coverage()
    print(f"Test Coverage: {test_cov:.1f}%")
    print(f"Doc Coverage: {doc_cov:.1f}%")
    update_readme(test_cov, doc_cov)


if __name__ == "__main__":
    main()
