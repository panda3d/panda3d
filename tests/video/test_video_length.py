from panda3d.core import MovieVideo
from panda3d.core import MovieVideoCursor
import os

def test_video_length(): 
    movie_path = os.path.join(os.path.dirname(__file__), "../../samples/media-player/PandaSneezes.ogv")
    reference_file = MovieVideo.get(movie_path)	    
    cursor = MovieVideo.open(reference_file)
    length_video = MovieVideoCursor.length(cursor)
    assert length_video == 14