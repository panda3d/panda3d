import os
import subprocess
import sys
import xml.etree.ElementTree as ET

import pytest
from panda3d import core
from direct.showbase.ShowBase import ShowBase


@pytest.fixture(scope="session")
def vfs():
    return core.VirtualFileSystem.get_global_ptr()


@pytest.fixture
def ramdir():
    """Fixture yielding a fresh ramdisk directory."""

    vfs = core.VirtualFileSystem.get_global_ptr()
    mount = core.VirtualFileMountRamdisk()
    dir = core.Filename.temporary("/virtual", "ram.")
    assert vfs.mount(mount, dir, 0)

    yield dir
    vfs.unmount(mount)


@pytest.fixture
def base():
    base = ShowBase(windowType='none')
    yield base
    base.destroy()


@pytest.fixture
def tk_toplevel():
    tk = pytest.importorskip('tkinter')

    if sys.platform == 'darwin' and not core.ConfigVariableBool('want-tk', False):
        pytest.skip('"want-tk" must be true to use tkinter with Panda3D on macOS')
    try:
        root = tk.Toplevel()
    except tk.TclError as e:
        pytest.skip(str(e))
    yield root
    root.destroy()


# C++ test suite integration.  Collects tests/<package>/test_*.cxx files and
# exposes each Catch2 TEST_CASE in the run_cxx_tests binary as a pytest item
# anchored to its source file, so that directory and file selection, -k
# filtering and reporting work the same way as for the Python tests.

def _find_cxx_tests_binary():
    import panda3d
    root = os.path.dirname(os.path.dirname(os.path.abspath(panda3d.__file__)))
    path = os.path.join(root, "bin", "run_cxx_tests")
    if sys.platform in ("win32", "cygwin"):
        path += ".exe"
    if os.path.isfile(path):
        return path
    return None


_cxx_tests_binary = _find_cxx_tests_binary()


def _run_cxx_tests(args):
    bin_dir = os.path.dirname(_cxx_tests_binary)
    root = os.path.dirname(bin_dir)
    env = dict(os.environ)

    # The binary carries no rpath, so point the loader at the built libs.
    for var in ("LD_LIBRARY_PATH", "DYLD_LIBRARY_PATH"):
        env[var] = os.pathsep.join((os.path.join(root, "lib"), env.get(var, "")))
    env["PATH"] = os.pathsep.join((bin_dir, env.get("PATH", "")))

    return subprocess.run([_cxx_tests_binary] + args, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, text=True, env=env)


def _cxx_test_index(config):
    """Maps (package dir, file name) to a list of (test name, line number),
    obtained from the test listing of the run_cxx_tests binary."""
    index = getattr(config, "_cxx_test_index", None)
    if index is None:
        index = {}
        proc = _run_cxx_tests(["--list-tests", "--reporter", "xml"])
        proc.check_returncode()
        for case in ET.fromstring(proc.stdout).iter("TestCase"):
            source_info = case.find("SourceInfo")
            file = source_info.find("File").text.replace("\\", "/")
            key = tuple(file.split("/")[-2:])
            line = int(source_info.find("Line").text)
            index.setdefault(key, []).append((case.find("Name").text, line))
        config._cxx_test_index = index
    return index


class CxxTestItem(pytest.Item):
    def __init__(self, *, line, **kwargs):
        super().__init__(**kwargs)
        self._line = line

    def runtest(self):
        # A quoted test spec is matched verbatim, so that names containing
        # characters that are special in specs (commas, brackets) work.
        spec = '"' + self.name.replace("\\", "\\\\").replace('"', '\\"').replace(",", "\\,") + '"'
        proc = _run_cxx_tests([spec])

        if proc.returncode == 4:
            # Catch2 exits with 4 when all selected test cases were skipped.
            pytest.skip(proc.stdout)
        elif proc.returncode != 0:
            raise CxxTestFailure(proc.stdout)

    def repr_failure(self, excinfo):
        if isinstance(excinfo.value, CxxTestFailure):
            return str(excinfo.value)
        return super().repr_failure(excinfo)

    def reportinfo(self):
        return self.path, self._line - 1, self.name


class CxxTestFailure(Exception):
    pass


class CxxTestFile(pytest.File):
    def collect(self):
        if _cxx_tests_binary is None:
            yield CxxSkippedFileItem.from_parent(
                self, name=self.path.stem,
                reason="run_cxx_tests binary not found")
            return

        key = (self.path.parent.name, self.path.name)
        cases = _cxx_test_index(self.config).get(key)
        if cases is None:
            # Either the binary predates this file, or its test cases were
            # compiled out.
            yield CxxSkippedFileItem.from_parent(
                self, name=self.path.stem,
                reason="no test cases from this file in the run_cxx_tests "
                       "binary; is it up to date?")
            return

        for name, line in cases:
            yield CxxTestItem.from_parent(self, name=name, line=line)


class CxxSkippedFileItem(pytest.Item):
    def __init__(self, *, reason, **kwargs):
        super().__init__(**kwargs)
        self._reason = reason

    def runtest(self):
        pytest.skip(self._reason)


def pytest_addoption(parser):
    parser.addoption(
        "--no-cxx-tests", action="store_true", default=False,
        help="Do not collect C++ tests")


def pytest_collect_file(file_path, parent):
    if (file_path.suffix == ".cxx" and file_path.name.startswith("test_") and
            not parent.config.getoption("--no-cxx-tests")):
        return CxxTestFile.from_parent(parent, path=file_path)


def _test_category(item):
    """Returns the reporting category for a test item: "cxx" for tests coming
    from the C++ test suite, "python" for everything else."""
    if isinstance(item, (CxxTestItem, CxxSkippedFileItem)):
        return "cxx"
    return "python"


def pytest_configure(config):
    """Initialize the failure collectors, kept separately per category so that
    the C++ and Python test suites are reported apart from each other."""
    config._github_summary_failures = {"python": [], "cxx": []}
    config._github_summary_counts = {
        "python": {"passed": 0, "failed": 0, "skipped": 0, "xfailed": 0},
        "cxx": {"passed": 0, "failed": 0, "skipped": 0, "xfailed": 0},
    }


@pytest.hookimpl(hookwrapper=True)
def pytest_runtest_makereport(item, call):
    """Collect information about test outcomes."""
    outcome = yield
    report = outcome.get_result()

    category = _test_category(item)
    counts = item.config._github_summary_counts[category]

    if report.when == "call":
        if report.passed:
            if hasattr(report, "wasxfail"):
                # xpass - passed but was expected to fail (treat as passed for now)
                counts["passed"] += 1
            else:
                counts["passed"] += 1
        elif report.failed:
            counts["failed"] += 1
            failure_info = {
                "nodeid": report.nodeid,
                "location": report.location,
                "longrepr": str(report.longrepr) if report.longrepr else None,
                "sections": report.sections,
                "duration": report.duration,
            }
            item.config._github_summary_failures[category].append(failure_info)
        elif report.skipped:
            if hasattr(report, "wasxfail"):
                counts["xfailed"] += 1
            else:
                counts["skipped"] += 1

    elif report.when == "setup" and report.skipped:
        # Handle skip during setup (e.g., skipif, skip markers)
        counts["skipped"] += 1


def _summary_lines(counts, failures, label):
    """Builds the GitHub step summary lines for one test category."""
    lines = []

    # Build status parts for the summary line
    status_parts = []
    if counts["failed"]:
        status_parts.append(f"**{counts['failed']}** failed")
    if counts["passed"]:
        status_parts.append(f"**{counts['passed']}** passed")
    if counts["skipped"]:
        status_parts.append(f"**{counts['skipped']}** skipped")
    if counts["xfailed"]:
        status_parts.append(f"**{counts['xfailed']}** xfailed")

    total = sum(counts.values())
    status_line = ", ".join(status_parts) + f" (**{total}** total)\n"

    # Header with overall status
    if counts["failed"] == 0:
        lines.append(f"### :white_check_mark: All tests passed ({label})\n\n")
    else:
        lines.append(f"### :x: Test failures ({label})\n\n")

    lines.append(status_line)
    lines.append("\n")

    # Each failure in a collapsible section
    for failure in failures:
        nodeid = failure["nodeid"]
        duration = failure["duration"]

        lines.append(f"<details>\n<summary><code>{_escape_html(nodeid)}</code> ({duration:.2f}s)</summary>\n")

        # Traceback / longrepr
        if failure["longrepr"]:
            lines.append("\n#### Traceback\n")
            lines.append("```python\n")
            lines.append(failure["longrepr"])
            if not failure["longrepr"].endswith("\n"):
                lines.append("\n")
            lines.append("```\n")

        # Captured output sections (stdout, stderr, log, etc.)
        for section_name, section_content in failure["sections"]:
            if section_content.strip():
                lines.append(f"\n#### {_escape_html(section_name)}\n")
                lines.append("```\n")
                lines.append(section_content)
                if not section_content.endswith("\n"):
                    lines.append("\n")
                lines.append("```\n")

        lines.append("\n</details>\n\n")

    return lines


def pytest_sessionfinish(session, exitstatus):
    """Write GitHub step summary if GITHUB_STEP_SUMMARY is set."""
    summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
    if not summary_path:
        return

    config = session.config
    failures = config._github_summary_failures
    counts = config._github_summary_counts

    lines = []

    # Report the C++ suite separately, if it was collected.
    if not config.getoption("--no-cxx-tests") and sum(counts["cxx"].values()):
        lines += _summary_lines(counts["cxx"], failures["cxx"], "C++")

    if config.getoption("--no-cxx-tests") or sum(counts["python"].values()):
        py_desc = f"Python {sys.version_info.major}.{sys.version_info.minor}"
        lines += _summary_lines(counts["python"], failures["python"], py_desc)

    with open(summary_path, "a", encoding="utf-8") as f:
        f.writelines(lines)


def _escape_html(text: str) -> str:
    """Escape HTML special characters for safe rendering in markdown."""
    return (
        text.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
    )
