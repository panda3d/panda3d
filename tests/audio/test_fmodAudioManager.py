import os

import pytest
from panda3d.core import (
    AudioManager,
    AudioSound,
    Filename,
    FilterProperties,
    load_prc_file_data,
)


@pytest.fixture(scope="module")
def fmod_audiomgr():
    load_prc_file_data("", "audio-library-name p3fmod_audio")
    mgr = AudioManager.create_AudioManager()
    if "fmod" not in str(mgr).lower():
        pytest.skip("FMOD audio backend is not available in this build")
    if not mgr.is_valid():
        pytest.skip("FMOD audio manager failed to initialize")
    return mgr


def _wav_path():
    return Filename.from_os_specific(
        os.path.join(os.path.dirname(__file__), "wav_test.wav"))


@pytest.fixture
def fmod_sound(fmod_audiomgr):
    """Loads wav_test.wav under the FMOD manager. Skips if loading fails."""
    sound = fmod_audiomgr.get_sound(_wav_path())
    if str(sound).startswith("NullAudioSound"):
        pytest.skip("FMOD sound loading returned NullAudioSound")
    return sound


# -----------------------------------------------------------------------------
# Manager lifecycle
# -----------------------------------------------------------------------------

def test_manager_lifecycle(fmod_audiomgr):
    fmod_audiomgr.set_cache_limit(8)
    assert fmod_audiomgr.get_cache_limit() == 8
    fmod_audiomgr.clear_cache()


def test_configure_filters_empty(fmod_audiomgr):
    assert fmod_audiomgr.configure_filters(FilterProperties())


DSP_BUILDERS = {
    "lowpass":    lambda fp: fp.add_lowpass(5000.0, 1.0),
    "highpass":   lambda fp: fp.add_highpass(500.0, 1.0),
    "echo":       lambda fp: fp.add_echo(0.5, 0.5, 500.0, 0.5),
    "flange":     lambda fp: fp.add_flange(0.45, 0.55, 1.0, 0.1),
    "distort":    lambda fp: fp.add_distort(0.5),
    "normalize":  lambda fp: fp.add_normalize(5000.0, 0.1, 20.0),
    "parameq":    lambda fp: fp.add_parameq(8000.0, 1.0, 1.0),
    "pitchshift": lambda fp: fp.add_pitchshift(1.0, 1024.0, 4.0),
    "chorus":     lambda fp: fp.add_chorus(0.5, 0.5, 0.5, 0.5, 3.0, 0.8, 3.0),
    "sfxreverb":  lambda fp: fp.add_sfxreverb(),
    "compress":   lambda fp: fp.add_compress(-30.0, 50.0, 50.0, 0.0),
    "fader":      lambda fp: fp.add_fader(0.0),
    "limiter":    lambda fp: fp.add_limiter(1000.0, 0.0, 0.0, 0.0),
    "pan":        lambda fp: fp.add_pan(),
    "tremolo":    lambda fp: fp.add_tremolo(),
    "delay":      lambda fp: fp.add_delay(),
}


@pytest.mark.parametrize("dsp_name", sorted(DSP_BUILDERS.keys()))
def test_configure_filters_single_dsp(fmod_audiomgr, dsp_name):
    """Each FMOD-supported DSP type can be installed individually."""
    fp = FilterProperties()
    DSP_BUILDERS[dsp_name](fp)
    assert fmod_audiomgr.configure_filters(fp)
    # Reset chain so subsequent parametrized iterations start clean.
    assert fmod_audiomgr.configure_filters(FilterProperties())


def test_configure_filters_stacked_chain(fmod_audiomgr):
    """Heterogeneous chains, and reconfiguration with a different stack,
    both succeed (exercises the tear-down + rebuild path)."""
    fp1 = FilterProperties()
    fp1.add_lowpass(5000.0, 1.0)
    fp1.add_echo(0.5, 0.5, 500.0, 0.5)
    fp1.add_fader(0.0)
    fp1.add_limiter(1000.0, 0.0, 0.0, 0.0)
    assert fmod_audiomgr.configure_filters(fp1)

    fp2 = FilterProperties()
    fp2.add_highpass(500.0, 1.0)
    fp2.add_pan()
    assert fmod_audiomgr.configure_filters(fp2)
    assert fmod_audiomgr.configure_filters(FilterProperties())


# -----------------------------------------------------------------------------
# Sound loading + caching
# -----------------------------------------------------------------------------

def test_load_wav_file(fmod_audiomgr):
    sound = fmod_audiomgr.get_sound(_wav_path())
    if str(sound).startswith("NullAudioSound"):
        pytest.skip("FMOD sound loading returned NullAudioSound")
    assert "wav_test" in sound.get_name()
    assert sound.length() >= 0.0
    assert sound.status() == AudioSound.READY


def test_load_missing_file(fmod_audiomgr):
    sound = fmod_audiomgr.get_sound("/not/a/valid/file.wav")
    assert str(sound).startswith("NullAudioSound")


def test_cache_eviction_via_clear(fmod_audiomgr):
    fmod_audiomgr.set_cache_limit(4)

    sound1 = fmod_audiomgr.get_sound(_wav_path())
    if str(sound1).startswith("NullAudioSound"):
        pytest.skip("FMOD sound loading returned NullAudioSound")
    del sound1

    fmod_audiomgr.clear_cache()

    sound2 = fmod_audiomgr.get_sound(_wav_path())
    assert not str(sound2).startswith("NullAudioSound")


def test_uncache_sound_by_path(fmod_audiomgr):
    sound = fmod_audiomgr.get_sound(_wav_path())
    if str(sound).startswith("NullAudioSound"):
        pytest.skip("FMOD sound loading returned NullAudioSound")
    del sound

    fmod_audiomgr.uncache_sound(_wav_path())

    reloaded = fmod_audiomgr.get_sound(_wav_path())
    assert not str(reloaded).startswith("NullAudioSound")


# -----------------------------------------------------------------------------
# Sound property round-trips
# -----------------------------------------------------------------------------

@pytest.mark.parametrize("value", [0.0, 0.5, 1.0])
def test_sound_volume_round_trip(fmod_sound, value):
    fmod_sound.set_volume(value)
    assert fmod_sound.get_volume() == pytest.approx(value)


@pytest.mark.parametrize("value", [-1.0, 0.0, 1.0])
def test_sound_balance_round_trip(fmod_sound, value):
    fmod_sound.set_balance(value)
    assert fmod_sound.get_balance() == pytest.approx(value)


@pytest.mark.parametrize("value", [0.5, 1.0, 2.0])
def test_sound_play_rate_round_trip(fmod_sound, value):
    fmod_sound.set_play_rate(value)
    assert fmod_sound.get_play_rate() == pytest.approx(value)


def test_sound_loop_round_trip(fmod_sound):
    fmod_sound.set_loop(True)
    assert fmod_sound.get_loop() is True
    fmod_sound.set_loop(False)
    assert fmod_sound.get_loop() is False

    fmod_sound.set_loop_count(5)
    assert fmod_sound.get_loop_count() == 5


def test_sound_active_round_trip(fmod_sound):
    fmod_sound.set_active(False)
    assert fmod_sound.get_active() is False
    fmod_sound.set_active(True)
    assert fmod_sound.get_active() is True


# -----------------------------------------------------------------------------
# 3D / spatial
# -----------------------------------------------------------------------------

def test_3d_min_max_distance_round_trip(fmod_sound):
    fmod_sound.set_3d_min_distance(2.0)
    assert fmod_sound.get_3d_min_distance() == pytest.approx(2.0)
    fmod_sound.set_3d_max_distance(50.0)
    assert fmod_sound.get_3d_max_distance() == pytest.approx(50.0)


def test_set_3d_attributes_does_not_raise(fmod_sound):
    fmod_sound.set_3d_attributes(1.0, 2.0, 3.0, 0.1, 0.2, 0.3)


def test_listener_factors_round_trip(fmod_audiomgr):
    fmod_audiomgr.audio_3d_set_distance_factor(2.0)
    assert fmod_audiomgr.audio_3d_get_distance_factor() == pytest.approx(2.0)
    fmod_audiomgr.audio_3d_set_doppler_factor(1.5)
    assert fmod_audiomgr.audio_3d_get_doppler_factor() == pytest.approx(1.5)
    fmod_audiomgr.audio_3d_set_drop_off_factor(0.7)
    assert fmod_audiomgr.audio_3d_get_drop_off_factor() == pytest.approx(0.7)
    # Reset to defaults
    fmod_audiomgr.audio_3d_set_distance_factor(1.0)
    fmod_audiomgr.audio_3d_set_doppler_factor(1.0)
    fmod_audiomgr.audio_3d_set_drop_off_factor(1.0)


# -----------------------------------------------------------------------------
# Speaker mix
# -----------------------------------------------------------------------------

def test_speaker_mix_per_channel_overload(fmod_sound):
    fmod_sound.set_speaker_mix(AudioManager.SPK_front_left, 0.5)
    fmod_sound.set_speaker_mix(AudioManager.SPK_surround_left, 0.5)


def test_speaker_mix_legacy_8arg_overload(fmod_sound):
    fmod_sound.set_speaker_mix(0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8)
