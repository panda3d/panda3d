import pytest
pytest.importorskip('tkinter')
Pmw = pytest.importorskip('Pmw')
from direct.showbase.ShowBase import ShowBase
from direct.tkpanels.Placer import Placer


def test_Placer(tk_toplevel):
    try:
        base = ShowBase()
    except Exception as ex:
        if str(ex) == 'Could not open window.':
            pytest.skip(str(ex))
        else:
            raise

    base.start_direct()
    root = Pmw.initialise()
    widget = Placer()
    base.destroy()
