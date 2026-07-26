from direct.interval.ActorInterval import ActorInterval


class _AnimControl:
    def __init__(self, frame_rate, num_frames):
        self._frame_rate = frame_rate
        self._num_frames = num_frames
        self.posed_frame = None

    def getFrameRate(self):
        return self._frame_rate

    def getNumFrames(self):
        return self._num_frames

    def pose(self, frame):
        self.posed_frame = frame


class _Actor:
    def __init__(self, control):
        self._control = control

    def getName(self):
        return "test-actor"

    def getAnimControls(self, anim_name, partName=None, lodName=None):
        return [self._control]


def test_get_current_frame_matches_posed_frame():
    # A 48-frame animation at 24 fps lasts 2 seconds.
    control = _AnimControl(frame_rate=24.0, num_frames=48)
    interval = ActorInterval(_Actor(control), "walk")
    interval.start()

    # getCurrentFrame must report the frame privStep actually poses, i.e.
    # startFrame + currT * frameRate -- not currT * numFrames.
    for t in (0.5, 1.0, 1.5):
        interval.privStep(t)
        assert interval.getCurrentFrame() == interval.startFrame + t * 24.0
        assert interval.getCurrentFrame() == control.posed_frame
