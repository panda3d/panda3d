from panda3d.core import MovieVideo
from panda3d.core import MovieTexture
from panda3d.core import Filename
from panda3d.core import PandaSystem
from panda3d.core import MovieTexture

import pytest
import os


def check_ffmpeg():
    # Make sure video plug-ins are loaded
    MovieVideo.get("test.mp4")

    system = PandaSystem.get_global_ptr()
    return 'FFmpeg' in system.systems #checks whether ffmpeg is loaded


@pytest.mark.skipif(not check_ffmpeg(), reason="skip when ffmpeg is not available")
class Test_Texture_Movie():
    def test_texture_loop_count(self):
        movie_path = os.path.join(os.path.dirname(__file__), "small.mp4")
        movie_path = Filename.from_os_specific(movie_path) # enables Platform independent testing
        reference_file = MovieVideo.get(movie_path)
        reference_texture = MovieTexture(reference_file)
        assert reference_texture.get_loop_count() == 1

    def test_video_is_playing(self): #This test checks isPlaying() and stop() functionality
        movie_path = os.path.join(os.path.dirname(__file__), "small.mp4")
        movie_path = Filename.from_os_specific(movie_path) # enables Platform independent testing
        reference_file = MovieVideo.get(movie_path)
        reference_texture = MovieTexture(reference_file)
        reference_texture.play()
        assert reference_texture.is_playing() is True
        reference_texture.stop()
        assert reference_texture.is_playing() is False

    def test_play_rate(self):
        movie_path = os.path.join(os.path.dirname(__file__), "small.mp4")
        movie_path = Filename.from_os_specific(movie_path) # enables Platform independent testing
        reference_file = MovieVideo.get(movie_path)
        reference_texture = MovieTexture(reference_file)
        assert reference_texture.get_play_rate() == 1
        reference_texture.set_play_rate(2)
        assert reference_texture.get_play_rate() == 2

    def test_video_texture_length(self):
        movie_path = os.path.join(os.path.dirname(__file__), "small.mp4")
        movie_path = Filename.from_os_specific(movie_path) # enables Platform independent testing
        reference_file = MovieVideo.get(movie_path)
        reference_texture = MovieTexture(reference_file)
        assert reference_texture.get_video_length() == 32.480 #got info from mkvinfo
