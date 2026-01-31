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


@pytest.fixture(scope='session')
def graphics_pipe():
    from panda3d.core import GraphicsPipeSelection

    pipe = GraphicsPipeSelection.get_global_ptr().make_default_pipe()

    if pipe is None or not pipe.is_valid():
        pytest.skip("GraphicsPipe is invalid")

    yield pipe


@pytest.fixture(scope='session')
def graphics_engine():
    from panda3d.core import GraphicsEngine

    engine = GraphicsEngine.get_global_ptr()
    yield engine

    # This causes GraphicsEngine to also terminate the render threads.
    engine.remove_all_windows()


@pytest.fixture
def window(graphics_pipe, graphics_engine):
    from panda3d.core import GraphicsPipe, FrameBufferProperties, WindowProperties

    fbprops = FrameBufferProperties.get_default()
    winprops = WindowProperties.get_default()

    win = graphics_engine.make_output(
        graphics_pipe,
        'window',
        0,
        fbprops,
        winprops,
        GraphicsPipe.BF_require_window
    )
    graphics_engine.open_windows()

    if win is None:
        pytest.skip("GraphicsPipe cannot make windows")

    yield win

    if win is not None:
        graphics_engine.remove_window(win)


@pytest.fixture(scope='module')
def gsg(graphics_pipe, graphics_engine):
    "Returns a windowless GSG that can be used for offscreen rendering."
    from panda3d.core import GraphicsPipe, FrameBufferProperties, WindowProperties

    fbprops = FrameBufferProperties()
    #fbprops.force_hardware = True

    buffer = graphics_engine.make_output(
        graphics_pipe,
        'buffer',
        0,
        fbprops,
        WindowProperties.size(32, 32),
        GraphicsPipe.BF_refuse_window
    )
    graphics_engine.open_windows()

    if buffer is None:
        pytest.skip("GraphicsPipe cannot make offscreen buffers")

    yield buffer.gsg

    if buffer is not None:
        graphics_engine.remove_window(buffer)


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
