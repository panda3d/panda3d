from panda3d.core import MovieVideo
from panda3d.core import MovieVideoCursor
import pytest

def get_movie():
    movie_path = "../samples/media-player/PandaSneezes.ogv"
    getFile = MovieVideo.get(movie_path)
    return getFile #Obtains a MovieVideo that references a file

@pytest.fixture
def cursor_check():
    movie_path = "../samples/media-player/PandaSneezes.ogv"
    getFile = get_movie() #references the function above
    valid_cursor_check = MovieVideo.open(getFile) #Open this video, returns a MovieVideoCursor 


def test_loading():
    getFile = get_movie()
    FileReturn = MovieVideo.getFilename(getFile)
    assert FileReturn == FileReturn

def test_cursor_check(cursor_check):
    valid_cursor_check = cursor_check
    assert valid_cursor_check != None


    

