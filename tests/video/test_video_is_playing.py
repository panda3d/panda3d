from panda3d.core import MovieVideo
from panda3d.core import MovieTexture
import os
#This test checks isPlaying() and stop() functionality
def test_video_is_playing(): 
    movie_path = os.path.join(os.path.dirname(__file__), "small.webm")
    reference_file = MovieVideo.get(movie_path)	    
    reference_texture = MovieTexture(reference_file)
    MovieTexture.play(reference_texture)
    assert MovieTexture.isPlaying(reference_texture) is True #checks whether the video is playing or not
    MovieTexture.stop(reference_texture)
    assert MovieTexture.isPlaying(reference_texture) is False
