import os
import sys
import pytest
from panda3d import core
from direct.showbase.ShowBase import ShowBase


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


def pytest_configure(config):
    """Initialize the failure collector."""
    config._github_summary_failures = []
    config._github_summary_counts = {
        "passed": 0,
        "failed": 0,
        "skipped": 0,
        "xfailed": 0,
    }


@pytest.hookimpl(hookwrapper=True)
def pytest_runtest_makereport(item, call):
    """Collect information about test outcomes."""
    outcome = yield
    report = outcome.get_result()

    if report.when == "call":
        counts = item.config._github_summary_counts

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
            item.config._github_summary_failures.append(failure_info)
        elif report.skipped:
            if hasattr(report, "wasxfail"):
                counts["xfailed"] += 1
            else:
                counts["skipped"] += 1

    elif report.when == "setup" and report.skipped:
        # Handle skip during setup (e.g., skipif, skip markers)
        item.config._github_summary_counts["skipped"] += 1


def pytest_sessionfinish(session, exitstatus):
    """Write GitHub step summary if GITHUB_STEP_SUMMARY is set."""
    summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
    if not summary_path:
        return

    failures = session.config._github_summary_failures
    counts = session.config._github_summary_counts

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
        lines.append(f"### :white_check_mark: All tests passed (Python {sys.version_info.major}.{sys.version_info.minor})\n\n")
    else:
        lines.append(f"### :x: Test failures (Python {sys.version_info.major}.{sys.version_info.minor})\n\n")

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

    with open(summary_path, "a", encoding="utf-8") as f:
        f.writelines(lines)


def _escape_html(text: str) -> str:
    """Escape HTML special characters for safe rendering in markdown."""
    return (
        text.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
    )
