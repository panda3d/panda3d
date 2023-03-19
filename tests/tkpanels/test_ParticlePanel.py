import Pmw
from direct.showbase.ShowBase import ShowBase
from direct.tkpanels.ParticlePanel import ParticlePanel


def test_ParticlePanel():
    base = ShowBase()
    root = Pmw.initialise()
    pp = ParticlePanel()
    base.pp=pp
    #ve = VectorEntry(Toplevel(), relief = GROOVE)
    #ve.pack()
    base.destroy()
