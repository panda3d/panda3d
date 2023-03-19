import pytest
pytest.importorskip('Pmw')
from direct.tkwidgets import AppShell


def test_TestAppShell():
    test = AppShell.TestAppShell(balloon_state='none')
