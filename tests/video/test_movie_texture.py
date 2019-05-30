from panda3d.core import MovieVideo
from panda3d.core import MovieTexture
from panda3d.core import Filename
import os
import pytest
from panda3d.core import PandaSystem
from panda3d.core import MovieTexture

def check_ffmpeg():
    movie_path = os.path.join(os.path.dirname(__file__), "small.mp4")
    movie_path = Filename.from_os_specific(movie_path) #platform independent path
    reference_file = MovieVideo.get(movie_path)
    reference_file.open()
    system = PandaSystem.get_global_ptr()
    has_ffmpeg = 'FFmpeg' in system.systems #checks whether ffmpeg is loaded
    if has_ffmpeg is True:
        return True
    else:
        return False

@pytest.mark.skipif(check_ffmpeg() is False, reason="skip when ffmpeg is not available")
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
