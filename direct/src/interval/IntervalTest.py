from PandaModules import *
from DirectSessionGlobal import *
from IntervalGlobal import *

import Mopath
import IntervalPlayer

AudioManager.spawnUpdate()

boat = loader.loadModel('models/directmodels/smiley')
boat.reparentTo(render)

dock = loader.loadModel('models/directmodels/smiley')
dock.reparentTo(render)

mp = Mopath.Mopath()
mp.loadFile(Filename('phase_6/paths/dd-e-w'))

boatMopath = MopathInterval('boatpath', mp, boat)

sound = loader.loadSound('phase_6/audio/sfx/SZ_DD_waterlap.mp3')
waterSound = SoundInterval('watersound', sound)

pos = Point3(0, 0, -5)
hpr = Vec3(0, 0, 0)
dockLerp = LerpPosHprInterval('lerp', dock, pos, hpr, 5.0)

boatTrack = Track.Track([boatMopath], 'boattrack')
dockWaitTime = boatMopath.getDuration() - dockLerp.getDuration()
dockTrack = Track.Track([Wait(dockWaitTime), dockLerp], 'docktrack')
postSoundWaitTime = 3.0
preSoundWaitTime = boatMopath.getDuration() - (waterSound.getDuration() + postSoundWaitTime)
soundTrack = Track.Track([Wait(preSoundWaitTime), waterSound, Wait(postSoundWaitTime)], 'soundtrack')

mtrack = MultiTrack.MultiTrack([boatTrack, dockTrack, soundTrack])

player = IntervalPlayer.IntervalPlayer(globalClock)
player.addInterval(mtrack)
