from panda3d.core import PandaSystem
from panda3d.core import Filename
from panda3d.core import MovieTexture
from panda3d.core import MovieVideo
import os


def test_check_ffmpeg():
    movie_path = os.path.join(os.path.dirname(__file__), "Example.ogg")
    movie_path = Filename.from_os_specific(movie_path) #platform independent path
    reference_file = MovieVideo.get(movie_path)	    
    reference_texture = MovieTexture(reference_file)
    reference_texture.play() # plays the reference textture
    system = PandaSystem.get_global_ptr()
    has_ffmpeg = 'FFmpeg' in system.systems #checks whether ffmpeg is loaded
    assert has_ffmpeg is True
    reference_texture.stop()
