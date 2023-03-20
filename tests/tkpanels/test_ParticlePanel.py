import pytest
Pmw = pytest.importorskip('Pmw')
from direct.showbase.ShowBase import ShowBase
from direct.tkpanels.ParticlePanel import ParticlePanel


def test_ParticlePanel(tk_toplevel):
    base = ShowBase(windowType='none')
    root = Pmw.initialise()
    pp = ParticlePanel()
    base.pp=pp
    #ve = VectorEntry(Toplevel(), relief = GROOVE)
    #ve.pack()
    base.destroy()
