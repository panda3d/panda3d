from panda3d.core import MovieVideo
from panda3d.core import MovieTexture
from panda3d.core import Filename
import os

def test_texture_loop_count(): 
    movie_path = os.path.join(os.path.dirname(__file__), "small.webm")
    movie_path = Filename.from_os_specific(movie_path) # enables Platform independent testing
    reference_file = MovieVideo.get(movie_path)	    
    reference_texture = MovieTexture(reference_file)
    assert reference_texture.get_loop_count() == 1


def test_video_is_playing(): #This test checks isPlaying() and stop() functionality
    movie_path = os.path.join(os.path.dirname(__file__), "small.webm")
    movie_path = Filename.from_os_specific(movie_path) # enables Platform independent testing
    reference_file = MovieVideo.get(movie_path)	    
    reference_texture = MovieTexture(reference_file)
    reference_texture.play()
    assert reference_texture.is_playing() is True 
    reference_texture.stop()
    assert reference_texture.is_playing() is False

def test_play_rate():
    movie_path = os.path.join(os.path.dirname(__file__), "small.webm")
    movie_path = Filename.from_os_specific(movie_path) # enables Platform independent testing 
    reference_file = MovieVideo.get(movie_path)	    
    reference_texture = MovieTexture(reference_file)
    assert reference_texture.get_play_rate() == 1
    reference_texture.set_play_rate(2)
    assert reference_texture.get_play_rate() == 2
