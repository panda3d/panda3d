import pytest
pytest.importorskip('tkinter')
Pmw = pytest.importorskip('Pmw')
from direct.showbase.ShowBase import ShowBase
from direct.tkpanels.Placer import Placer


def test_Placer(window, tk_toplevel):
    base = ShowBase()
    base.start_direct()
    root = Pmw.initialise()
    widget = Placer()
    base.destroy()
