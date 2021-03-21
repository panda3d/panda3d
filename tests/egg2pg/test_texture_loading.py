from direct.showbase.ShowBase import ShowBase
import sys,os
from panda3d.core import Filename

def test_textures_loaded_correctly():
    base = ShowBase()
    # Get the location of the 'py' file I'm running:
    mydir = os.path.abspath(sys.path[0])
    # Convert that to panda's unix-style notation.
    mydir = Filename.fromOsSpecific(mydir).getFullpath()
    
    # Now load the model:
    model = loader.loadModel(mydir + "/egg2pg/plane.egg")
    #load the model
    #get the plane
    result = model.find('**/Plane')
    #the plane egg has different texture states that use the same image
    #since these texture states are different, they should come back as different pointers
    geoms = result.node().getGeomStates()
    #textures 4 and 5 are identical. therefore only 4 should be in the file.
    base.destroy()
    base = None
    assert len(geoms) == 4
    