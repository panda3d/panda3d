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
