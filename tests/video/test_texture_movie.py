from panda3d.core import MovieVideo
from panda3d.core import MovieTexture
import os

def test_texture_loop_count(): 
    movie_path = os.path.join(os.path.dirname(__file__), "small.webm")
    reference_file = MovieVideo.get(movie_path)	    
    reference_texture = MovieTexture(reference_file)
    assert reference_texture.getLoopCount() == 1

#This test checks isPlaying() and stop() functionality
def test_video_is_playing(): 
    movie_path = os.path.join(os.path.dirname(__file__), "small.webm")
    reference_file = MovieVideo.get(movie_path)	    
    reference_texture = MovieTexture(reference_file)
    reference_texture.play()
    assert reference_texture.is_playing() is True #checks whether the video is playing or not
    reference_texture.stop()
    assert reference_texture.is_playing() is False
