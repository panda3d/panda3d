from PandaModules import *
from DirectSessionGlobal import *
from LerpInterval import *

import Mopath
import MopathInterval
import SoundInterval
import Track

AudioManager.spawnUpdate()

smiley = loader.loadModel('models/directmodels/smiley')
smiley.reparentTo(render)

mp = Mopath.Mopath()
mp.loadFile(Filename('phase_6/paths/dd-e-w'))

mpi = MopathInterval.MopathInterval('boatpath', mp, smiley)

sound = loader.loadSound('phase_6/audio/sfx/SZ_DD_waterlap.mp3')
si = SoundInterval.SoundInterval('watersound', sound)

pos = Point3(0, 0, -5)
hpr = Vec3(0, 0, 0)
li = LerpPosHprInterval('lerp', smiley, pos, hpr, 5.0)
