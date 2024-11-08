import pytest
pytest.importorskip('tkinter')
Pmw = pytest.importorskip('Pmw')
from direct.tkpanels.ParticlePanel import ParticlePanel


def test_ParticlePanel(base, tk_toplevel):
    root = Pmw.initialise()
    pp = ParticlePanel()
    base.pp=pp
    #ve = VectorEntry(Toplevel(), relief = GROOVE)
    #ve.pack()
