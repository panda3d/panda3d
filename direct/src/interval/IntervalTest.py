from PandaModules import *
from DirectSessionGlobal import *
from LerpInterval import *
from WaitInterval import *

import Mopath
import MopathInterval
import SoundInterval
import Track
import MultiTrack
import IntervalPlayer

AudioManager.spawnUpdate()

boat = loader.loadModel('models/directmodels/smiley')
boat.reparentTo(render)

dock = loader.loadModel('models/directmodels/smiley')
dock.reparentTo(render)

mp = Mopath.Mopath()
mp.loadFile(Filename('phase_6/paths/dd-e-w'))

boatMopath = MopathInterval.MopathInterval('boatpath', mp, boat)

sound = loader.loadSound('phase_6/audio/sfx/SZ_DD_waterlap.mp3')
waterSound = SoundInterval.SoundInterval('watersound', sound)

pos = Point3(0, 0, -5)
hpr = Vec3(0, 0, 0)
dockLerp = LerpPosHprInterval('lerp', dock, pos, hpr, 5.0)

boatTrack = Track.Track([boatMopath])
dockWaitTime = boatMopath.getDuration() - dockLerp.getDuration()
dockTrack = Track.Track([Wait(dockWaitTime), dockLerp])
postSoundWaitTime = 3.0
preSoundWaitTime = boatMopath.getDuration() - (waterSound.getDuration() + postSoundWaitTime)
soundTrack = Track.Track([Wait(preSoundWaitTime), waterSound, Wait(postSoundWaitTime)])

mtrack = MultiTrack.MultiTrack([boatTrack, dockTrack, soundTrack])

player = IntervalPlayer.IntervalPlayer(globalClock)
player.addInterval(mtrack)
