from PandaModules import *
from DirectSessionGlobal import *
from IntervalGlobal import *

import Mopath
import IntervalPlayer

boat = loader.loadModel('models/directmodels/smiley')
boat.reparentTo(render)

dock = loader.loadModel('models/directmodels/smiley')
dock.reparentTo(render)

sound = loader.loadSound('phase_6/audio/sfx/SZ_DD_waterlap.mp3')

mp = Mopath.Mopath()
mp.loadFile(Filename('phase_6/paths/dd-e-w'))

# Set up the boat
boatMopath = MopathInterval('boatpath', mp, boat)
boatTrack = Track.Track([boatMopath], 'boattrack')
BOAT_START = boatTrack.getIntervalStartTime('boatpath')
BOAT_END = boatTrack.getIntervalEndTime('boatpath')

# Make the dock lerp up so that it's up when the boat reaches the end of
# its mopath
pos = Point3(0, 0, -5)
hpr = Vec3(0, 0, 0)
dockLerp = LerpPosHprInterval('lerp', dock, pos, hpr, 5.0)
# We need the dock's state to be defined before the lerp
dockPos = PosHprInterval('dockpos', dock, dock.getPos(), dock.getHpr(), 1.0)
dockUpTime = BOAT_END - dockLerp.getDuration()
dockLerp.setStartTime(dockUpTime, TRACK_START)
dockTrack = Track.Track([dockPos, dockLerp], 'docktrack')

# Start the water sound 5 seconds after the boat starts moving
waterStartTime = BOAT_START + 5.0
waterSound = SoundInterval('watersound', sound, loop=1)
waterSound.setStartTime(waterStartTime, TRACK_START)
soundTrack = Track.Track([waterSound], 'soundtrack')

mtrack = MultiTrack.MultiTrack([boatTrack, dockTrack, soundTrack])

player = IntervalPlayer.IntervalPlayer(globalClock)
player.addInterval(mtrack)
