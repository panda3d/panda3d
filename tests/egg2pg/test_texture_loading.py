from panda3d.core import Filename, NodePath
from panda3d import core
from direct.showbase.Loader import Loader
import pytest

egg = pytest.importorskip("panda3d.egg")

EGG_SYNTAX_DATA = """
<CoordinateSystem> { Z-Up }

<Texture> Texture1 {
 "../models/maps/panda-model.jpg"
 <Scalar> format { RGB }
 <Scalar> alpha { OFF }
 <Scalar> compression { OFF }
 <Scalar> envtype { MODULATE }
 <Scalar> anisotropic-degree { 0 }
 <Scalar> wrapu { REPEAT }
 <Scalar> wrapv { REPEAT }
 <Scalar> minfilter { LINEAR }
 <Scalar> magfilter { LINEAR }
 <Scalar> quality-level { FASTEST }
}

<Texture> Texture2 {
 "../models/maps/panda-model.jpg"
 <Scalar> format { RGB }
 <Scalar> alpha { OFF }
 <Scalar> compression { OFF }
 <Scalar> envtype { MODULATE }
 <Scalar> anisotropic-degree { 0 }
 <Scalar> wrapu { REPEAT }
 <Scalar> wrapv { REPEAT }
 <Scalar> minfilter { LINEAR }
 <Scalar> magfilter { LINEAR }
 <Scalar> quality-level { FASTEST }
}

<Group>  Plane {
 <VertexPool> Plane {
  <Vertex> 0 { -1 -1 0 
  <UV> { 0 0 }
  }
  <Vertex> 1 { 1 -1 0 
  <UV> { 1 0 }
  }
  <Vertex> 2 { 1 1 0 
  <UV> { 1 1 }
  }
  <Vertex> 3 { -1 1 0 
  <UV> { 0 1 }
  }
  <Vertex> 4 { -1 1.40914 0 
  <UV> { 0 0 }
  }
  <Vertex> 5 { 1 1.40914 0 
  <UV> { 1 0 }
  }
  <Vertex> 6 { 1 3.40914 0 
  <UV> { 1 1 }
  }
  <Vertex> 7 { -1 3.40914 0 
  <UV> { 0 1 }
  }
 }

 <Polygon> 0 { 
  <TRef> { Texture1}
  <Normal> { 0 0 1 }
  <VertexRef> { 0 1 2 3 <Ref> { Plane } }
  }
  
 <Polygon> 1 { 
  <TRef> { Texture2 }
  <Normal> { 0 0 1 }
  <VertexRef> { 4 5 6 7 <Ref> { Plane } }
  }
}
"""

def read_egg_string(string):
    """Reads an EggData from a string."""
    stream = core.StringStream(string.encode('utf-8'))
    data = egg.EggData()
    assert data.read(stream)
    return data

def test_load_model_texture():
    data = read_egg_string(EGG_SYNTAX_DATA) 
    #model = loader.load_model(Filename(temp_model))
    #assert model
    #assert isinstance(model, NodePath)
    #assert model.name == 'model'


#import pytest
#from panda3d import core
#from direct.showbase.ShowBase import ShowBase
#from panda3d.core import TextureAttrib

#base = ShowBase()

#musicBox = loader.loadModel('plane')
#result = musicBox.find('**/Plane')

#Geom0 = result.node().getGeomState(0)
#texture0 = Geom0.getAttrib(TextureAttrib).getTexture()

#Geom1 = result.node().getGeomState(1)
#texture1 = Geom1.getAttrib(TextureAttrib).getTexture()

#print(texture0)
#print(texture1)