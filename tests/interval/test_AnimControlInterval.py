from panda3d.core import (
    AnimBundle,
    AnimControlCollection,
    PartBundle,
    PartSubset,
)

from direct.interval.AnimControlInterval import AnimControlInterval


def _make_controls(frame_rate, num_frames):
    anim_bundle = AnimBundle("anim", frame_rate, num_frames)
    part_bundle = PartBundle("part")
    control = part_bundle.bindAnim(anim_bundle, -1, PartSubset())
    controls = AnimControlCollection()
    controls.storeAnim(control, "anim")
    return controls


def test_get_current_frame_matches_frame_rate():
    # A 48-frame animation at 24 fps lasts 2 seconds.
    interval = AnimControlInterval(_make_controls(24.0, 48))
    interval.start()

    # getCurrentFrame must report startFrame + currT * frameRate (the frame
    # privStep poses), not currT * numFrames.
    for t in (0.5, 1.0, 1.5):
        interval.privStep(t)
        assert interval.getCurrentFrame() == interval.startFrame + t * 24.0
