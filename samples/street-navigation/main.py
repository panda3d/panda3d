from direct.showbase.ShowBase import ShowBase
from panda3d import navigation
from panda3d.core import PointLight,DirectionalLight

class MyApp(ShowBase):

    def __init__(self):
        ShowBase.__init__(self)

        # Setting up light for better view.
        plight = PointLight('plight')
        plight.setColor((20, 0.9, 0.9, 1))
        plnp = render.attachNewNode(plight)
        plnp.setPos(10, 20, 0)
        render.setLight(plnp)
        dlight = DirectionalLight('dlight')
        dlight.setColor((0.8, 0.5, 0.5, 1))
        dlnp = render.attachNewNode(dlight)
        dlnp.setHpr(0, -60, 0)
        render.setLight(dlnp)

        # Loading the model
        self.scene = self.loader.loadModel("models/street.obj")
        self.scene.reparentTo(self.render)
        self.scene.setP(90)
        self.scene.flatten_light()
        self.scene.setScale(0.25, 0.25, 0.25)
        self.scene.setPos(-8, 42, 0)

        # NavMeshBuilder is a class that is responsible for building the polygon meshes and navigation meshes.
        self.builder = navigation.NavMeshBuilder()
        # Take Nodepath as input. Nodepath should contain the required geometry.
        self.builder.fromNodePath(self.scene)

        self.builder.build()

        # Code to attach the polymesh generated to the scene graph
        self.node1 = self.builder.drawPolyMeshGeom()
        self.node = self.scene.attachNewNode(self.node1)
        self.node.setColor(0,0,1)
        
        


app = MyApp()
app.run()