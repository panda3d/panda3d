"""Undocumented Module"""

__all__ = []


if __name__ == "__main__":
    from direct.directbase import DirectStart
    from pandac.PandaModules import *
    from IntervalGlobal import *
    from direct.actor.Actor import *

    from direct.directutil import Mopath

    boat = loader.loadModel('models/misc/smiley')
    boat.reparentTo(render)

    donald = Actor()
    donald.loadModel("phase_6/models/char/donald-wheel-1000")
    donald.loadAnims({"steer":"phase_6/models/char/donald-wheel-wheel"})
    donald.reparentTo(boat)

    dock = loader.loadModel('models/misc/smiley')
    dock.reparentTo(render)

    sound = loader.loadSfx('phase_6/audio/sfx/SZ_DD_waterlap.mp3')
    foghorn = loader.loadSfx('phase_6/audio/sfx/SZ_DD_foghorn.mp3')

    mp = Mopath.Mopath()
    mp.loadFile(Filename('phase_6/paths/dd-e-w'))

    # Set up the boat
    boatMopath = MopathInterval(mp, boat, 'boatpath')
    boatTrack = Track([boatMopath], 'boattrack')
    BOAT_START = boatTrack.getIntervalStartTime('boatpath')
    BOAT_END = boatTrack.getIntervalEndTime('boatpath')

    # This will create an anim interval that is posed every frame
    donaldSteerInterval = ActorInterval(donald, 'steer')
    # This will create an anim interval that is started at t = 0 and then
    # loops for 10 seconds
    donaldLoopInterval = ActorInterval(donald, 'steer', loop=1, duration = 10.0)
    donaldSteerTrack = Track([donaldSteerInterval, donaldLoopInterval],
                             name = 'steerTrack')

    # Make the dock lerp up so that it's up when the boat reaches the end of
    # its mopath
    dockLerp = LerpPosHprInterval(dock, 5.0,
                                  pos=Point3(0, 0, -5),
                                  hpr=Vec3(0, 0, 0),
                                  name='dock-lerp')
    # We need the dock's state to be defined before the lerp
    dockPos = PosHprInterval(dock, dock.getPos(), dock.getHpr(), 1.0, 'dockpos')
    dockUpTime = BOAT_END - dockLerp.getDuration()
    hpr2 = Vec3(90.0, 90.0, 90.0)
    dockLerp2 = LerpHprInterval(dock, 3.0, hpr2, name='hpr-lerp')
    dockTrack = Track([dockLerp2, dockPos, dockLerp], 'docktrack')
    dockTrack.setIntervalStartTime('dock-lerp', dockUpTime)
    dockTrack.setIntervalStartTime('hpr-lerp', BOAT_START)

    # Start the water sound 5 seconds after the boat starts moving
    waterStartTime = BOAT_START + 5.0
    waterSound = SoundInterval(sound, name='watersound')
    soundTrack = Track([waterSound], 'soundtrack')
    soundTrack.setIntervalStartTime('watersound', waterStartTime)

    # Throw an event when the water track ends
    eventTime = soundTrack.getIntervalEndTime('watersound')
    waterDone = EventInterval('water-is-done')
    waterEventTrack = Track([waterDone])
    waterEventTrack.setIntervalStartTime('water-is-done', eventTime)

    def handleWaterDone():
        print 'water is done'

    # Interval can handle its own event
    messenger.accept('water-is-done', waterDone, handleWaterDone)

    foghornStartTime = BOAT_START + 4.0
    foghornSound = SoundInterval(foghorn, name='foghorn')
    soundTrack2 = Track([(foghornStartTime, foghornSound)], 'soundtrack2')

    mtrack = MultiTrack([boatTrack, dockTrack, soundTrack, soundTrack2, waterEventTrack,
                         donaldSteerTrack])
    # Print out MultiTrack parameters
    print(mtrack)

    ### Using lambdas and functions ###
    # Using a lambda
    i1 = FunctionInterval(lambda: base.transitions.fadeOut())
    i2 = FunctionInterval(lambda: base.transitions.fadeIn())

    def caughtIt():
        print 'Caught here-is-an-event'

    class DummyAcceptor(DirectObject):
        pass

    da = DummyAcceptor()
    i3 = AcceptInterval(da, 'here-is-an-event', caughtIt)

    i4 = EventInterval('here-is-an-event')

    i5 = IgnoreInterval(da, 'here-is-an-event')

    # Using a function
    def printDone():
        print 'done'

    i6 = FunctionInterval(printDone)

    # Create track
    t1 = Track([
        # Fade out
        (0.0, i1),
        # Fade in
        (2.0, i2),
        # Accept event
        (4.0, i3),
        # Throw it,
        (5.0, i4),
        # Ignore event
        (6.0, i5),
        # Throw event again and see if ignore worked
        (7.0, i4),
        # Print done
        (8.0, i6)], name = 'demo')

    print(t1)

    ### Specifying interval start times during track construction ###
    # Interval start time can be specified relative to three different points:
    # PREVIOUS_END
    # PREVIOUS_START
    # TRACK_START

    startTime = 0.0
    def printStart():
        global startTime
        startTime = globalClock.getFrameTime()
        print 'Start'

    def printPreviousStart():
        global startTime
        currTime = globalClock.getFrameTime()
        print 'PREVIOUS_END %0.2f' % (currTime - startTime)

    def printPreviousEnd():
        global startTime
        currTime = globalClock.getFrameTime()
        print 'PREVIOUS_END %0.2f' % (currTime - startTime)

    def printTrackStart():
        global startTime
        currTime = globalClock.getFrameTime()
        print 'TRACK_START %0.2f' % (currTime - startTime)

    def printArguments(a, b, c):
        print 'My args were %d, %d, %d' % (a, b, c)

    i1 = FunctionInterval(printStart)
    # Just to take time
    i2 = LerpPosInterval(camera, 2.0, Point3(0, 10, 5))
    # This will be relative to end of camera move
    i3 = FunctionInterval(printPreviousEnd)
    # Just to take time
    i4 = LerpPosInterval(camera, 2.0, Point3(0, 0, 5))
    # This will be relative to the start of the camera move
    i5 = FunctionInterval(printPreviousStart)
    # This will be relative to track start
    i6 = FunctionInterval(printTrackStart)
    # This will print some arguments
    # This will be relative to track start
    i7 = FunctionInterval(printArguments, extraArgs = [1, 10, 100])
    # Create the track, if you don't specify offset type in tuple it defaults to
    # relative to TRACK_START (first entry below)
    t2 = Track([(0.0, i1),                 # i1 start at t = 0, duration = 0.0
                (1.0, i2, TRACK_START),    # i2 start at t = 1, duration = 2.0
                (2.0, i3, PREVIOUS_END),   # i3 start at t = 5, duration = 0.0
                (1.0, i4, PREVIOUS_END),   # i4 start at t = 6, duration = 2.0
                (3.0, i5, PREVIOUS_START), # i5 start at t = 9, duration = 0.0
                (10.0, i6, TRACK_START),   # i6 start at t = 10, duration = 0.0
                (12.0, i7)],               # i7 start at t = 12, duration = 0.0
               name = 'startTimeDemo')

    print(t2)

    # Play tracks
    # mtrack.play()
    # t1.play()
    # t2.play()


    def test(n):
        lerps = []
        for i in range(n):
            lerps.append(LerpPosHprInterval(dock, 5.0,
                                            pos=Point3(0, 0, -5),
                                            hpr=Vec3(0, 0, 0),
                                            startPos=dock.getPos(),
                                            startHpr=dock.getHpr(),
                                            name='dock-lerp'))
            lerps.append(EventInterval("joe"))
        t = Track(lerps)
        mt = MultiTrack([t])
        # return mt

    test(5)
    run()
