import pytest
pytest.importorskip('tkinter')
pytest.importorskip('Pmw')
from direct.tkwidgets import VectorWidgets


def test_VectorEntry(tk_toplevel):
    root = tk_toplevel
    root.title('Vector Widget demo')

    ve = VectorWidgets.VectorEntry(root)
    ve.pack()
    v3e = VectorWidgets.Vector3Entry(root)
    v3e.pack()
    v4e = VectorWidgets.Vector4Entry(root)
    v4e.pack()
    ce = VectorWidgets.ColorEntry(root)
    ce.pack()
