from PandaModules import *
from DirectSessionGlobal import *
from IntervalGlobal import *

import Mopath
import IntervalPlayer

#AudioManager.spawnUpdate()

boat = loader.loadModel('models/directmodels/smiley')
boat.reparentTo(render)

dock = loader.loadModel('models/directmodels/smiley')
dock.reparentTo(render)

sound = loader.loadSound('phase_6/audio/sfx/SZ_DD_waterlap.mp3')

mp = Mopath.Mopath()
mp.loadFile(Filename('phase_6/paths/dd-e-w'))

boatMopath = MopathInterval('boatpath', mp, boat)

boatTrack = Track.Track([boatMopath], 'boattrack')

# Make the dock lerp up so that it's up when the boat reaches the end of
# its mopath
pos = Point3(0, 0, -5)
hpr = Vec3(0, 0, 0)
dockLerp = LerpPosHprInterval('lerp', dock, pos, hpr, 5.0)
dockUpTime = boatTrack.getTrackRelativeEndTime('boatpath') - dockLerp.getDuration()
dockLerp.setStartTime(dockUpTime, Interval.Interval.TrackStartRelative)
dockTrack = Track.Track([dockLerp], 'docktrack')

# Start the water sound 5 seconds after the boat starts moving
waterStartTime = boatTrack.getTrackRelativeStartTime('boatpath') + 5.0
waterSound = SoundInterval('watersound', sound, loop=1)
waterSound.setStartTime(waterStartTime, Interval.Interval.TrackStartRelative)
soundTrack = Track.Track([waterSound], 'soundtrack')

mtrack = MultiTrack.MultiTrack([boatTrack, dockTrack, soundTrack])

player = IntervalPlayer.IntervalPlayer(globalClock)
player.addInterval(mtrack)
