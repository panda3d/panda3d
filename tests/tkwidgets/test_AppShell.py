import pytest
pytest.importorskip('tkinter')
pytest.importorskip('Pmw')
from direct.tkwidgets import AppShell


def test_TestAppShell(tk_toplevel):
    test = AppShell.TestAppShell(balloon_state='none')
