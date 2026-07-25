import pytest
from panda3d import core
from direct.motiontrail.MotionTrail import MotionTrail


@pytest.fixture
def motion_trail():
    trail = MotionTrail('test', core.NodePath('parent'))
    # Update continuously so check_for_update is only gated by enable/pause.
    trail.sampling_time = 0.0
    trail.last_update_time = 0.0
    return trail


def test_check_for_update_respects_enable(motion_trail):
    # Enabled and not paused: due for an update.
    motion_trail.enable_motion_trail(True)
    assert motion_trail.check_for_update(1.0) is True

    # Disabling must prevent updates (per the enable_motion_trail docstring).
    motion_trail.enable_motion_trail(False)
    assert motion_trail.check_for_update(1.0) is False


def test_check_for_update_respects_pause(motion_trail):
    motion_trail.enable_motion_trail(True)

    motion_trail.pause_motion_trail(1.0)
    assert motion_trail.check_for_update(1.0) is False

    motion_trail.resume_motion_trail(1.0)
    assert motion_trail.check_for_update(1.0) is True
