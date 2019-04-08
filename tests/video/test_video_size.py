from panda3d.core import MovieVideo
from panda3d.core import MovieVideoCursor
import os

def test_video_size(): 
    movie_path = os.path.join(os.path.dirname(__file__), "small.webm")
    reference_file = MovieVideo.get(movie_path)	    
    cursor = MovieVideo.open(reference_file)
    horizontal_size = MovieVideoCursor.sizeX(cursor)
    vertical_size = MovieVideoCursor.sizeY(cursor)
    assert horizontal_size == 560 #found the height and width using mkvinfo
    assert vertical_size == 320