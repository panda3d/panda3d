from direct.showbase.ShowBase import ShowBase
from panda3d.core import Plane, Vec3, Point3, CardMaker, MouseButton
from direct.task import Task
from direct.actor.Actor import Actor
from direct.interval.IntervalGlobal import Sequence
from panda3d import navigation
from panda3d import navmeshgen
from panda3d.core import PointLight,DirectionalLight
from panda3d.core import LPoint3
from panda3d.core import PTA_LVecBase3
from panda3d.core import LVecBase3
from direct.actor.Actor import Actor
from panda3d.core import LineSegs
from panda3d.core import NodePath
import math
import time


class MyApp(ShowBase):

    def __init__(self):
        ShowBase.__init__(self)


        # Setting up light for better view.
        plight = PointLight('plight')
        plight.setColor((0.9, 0.9, 0.9, 0.5))
        plnp = render.attachNewNode(plight)
        plnp.setPos(10, 20, 0)
        render.setLight(plnp)
        dlight = DirectionalLight('dlight')
        dlight.setColor((0.8, 0.5, 0.5, 1))
        dlnp = render.attachNewNode(dlight)
        dlnp.setHpr(0, -60, 0)
        render.setLight(dlnp)

        # Loading the model
        self.scene = self.loader.loadModel("village.obj")
        self.scene.reparentTo(self.render)
        self.scene.setP(90)
        
        self.scene.setScale(0.25, 0.25, 0.25)
        self.scene.flatten_light()
        self.scene.setPos(-8, 42, 0)
        
        #mouseclick
        #self.disableMouse()
        self.camera.setPos(0, 60, 500)
        self.camera.lookAt(0, 0, 0)
        z = 0
        self.plane = Plane(Vec3(0, 0, 1), Point3(0, 0, z))
        # self.model = loader.loadModel("jack")
        # self.model.reparentTo(render)
        cm = CardMaker("blah")
        cm.setFrame(-100, 100, -100, 100)
        #self.scene.attachNewNode(cm.generate()).lookAt(0, 0, -1)
        taskMgr.add(self.__getMousePos, "_YourClass__getMousePos")
        #mouseclick end 


        #Here start commenting
        #NavMeshBuilder is a class that is responsible for building the polygon meshes and navigation meshes.
        self.builder = navmeshgen.NavMeshBuilder()
        self.structBuilder = navmeshgen.BuildSettings()
        self.structBuilder.cell_size = 1;
        # Take Nodepath as input. Nodepath should contain the required geometry.
        self.builder.fromNodePath(self.scene)

        #self.builder.setActorRadius(5)
        self.builder.set_actor_height(10)
        self.navmesh = self.builder.build()

        # Code to attach the polymesh generated to the scene graph
        self.node1 = self.navmesh.drawNavMeshGeom()
        # self.node = self.scene.attachNewNode(self.node1)
        # self.node.setColor(0,0,1)
        
        self.navmeshnode = navigation.NavMeshNode("firstnavmeshnode",self.navmesh)
        self.navmeshnodepath = self.scene.attachNewNode(self.navmeshnode)
        print("writing BAM")
        #self.navmeshnodepath.write_bam_file("firstnavmeshnode.bam")


        # print("start read")
        #self.navmesh = loader.loadModel("firstnavmeshnode.bam")
        #print(type(self.navmesh))
        self.tempActor = Actor("models/panda", {"walk" : "models/panda-walk"})
        self.tempActor.reparentTo(self.scene)
        
        # print(type(self.navmesh.node().getNavMesh()))
        # self.node1 = self.navmesh.node().drawNavMeshGeom()
        # self.node = self.scene.attachNewNode(self.node1)
        # self.node.setColor(0,0,1)
        
        

        self.pos1 = self.tempActor.getPos()
        self.pos2 = self.pos1
        self.query = navigation.NavMeshQuery(self.navmesh)
        
        # #self.query.setNavQuery(self.navmesh)
        self.query.nearestPoint(self.pos1)
        # print(pos1)
        self.tempActor.setPos(self.pos1)

        self.tempActor2 = Actor("models/panda", {"walk" : "models/panda-walk"})
        self.tempActor2.reparentTo(self.scene)

        # Load and transform the panda actor.
        self.pandaActor = Actor("models/panda-model",
                                {"walk": "models/panda-walk4"})
        self.pandaActor.setScale(0.01, 0.01, 0.01)
        self.pandaActor.reparentTo(self.scene)
        self.pandaActor.loop("walk")
        
        self.pandaSequence = Sequence()
        self.pandaSequence.start()

        self.lineNodePath = NodePath()
        self.lineNodePath2 = NodePath()

    def __getMousePos(self,task):
        if base.mouseWatcherNode.hasMouse() and base.mouseWatcherNode.isButtonDown(MouseButton.one()):
            mpos = base.mouseWatcherNode.getMouse()
            pos3d = Point3()
            nearPoint = Point3()
            farPoint = Point3()
            base.camLens.extrude(mpos, nearPoint, farPoint)
            if self.plane.intersectsLine(pos3d,
                self.scene.getRelativePoint(camera, nearPoint),
                self.scene.getRelativePoint(camera, farPoint)):
                #print("Mouse ray intersects ground plane at ", pos3d)
                #print("pos2 ", self.pos2)
                if abs(self.pos2[0]-pos3d[0]) < 1:
                    #print("not running", self.pos2[0]-pos3d[0])
                    return task.again
                #print("running it")
                self.pos2 = pos3d
                
                self.pos1 = self.pandaActor.get_pos()
                self.pandaSequence.finish()
                
                self.query.nearestPoint(self.pos2)
                # print(pos2)
                self.tempActor2.setPos(self.pos2)
                path = self.query.findPath(self.pos1, self.pos2);
                print("path.size(): ", len(path));
                self.pathLine = LineSegs()
                #self.pathLine.reset()
                self.pathLine.setColor(0,1,0)
                self.pathLine.setThickness(5)

                self.pathLine2 = LineSegs()
                #self.pathLine2.reset()
                self.pathLine2.setColor(1,1,0)
                self.pathLine2.setThickness(5)

                
                for i in range(len(path)):
                    self.pathLine.drawTo(path[i])

                self.pathLine.drawTo(self.pos2)
                self.lineNode = self.pathLine.create()
                self.lineNodePath.remove_node()
                #self.lineNodePath = NodePath()
                self.lineNodePath = self.scene.attachNewNode(self.lineNode)
                self.lineNodePath.setZ(1)

                path = self.query.findStraightPath(self.pos1, self.pos2, 0);
                print("path.size(): ", len(path));

                
                for i in range(len(path)):
                    self.pathLine2.drawTo(path[i])

                
                self.lineNode2 = self.pathLine2.create()
                self.lineNodePath2.remove_node()
                #self.lineNodePath2 = NodePath()
                self.lineNodePath2 = self.scene.attachNewNode(self.lineNode2)
                self.lineNodePath2.setZ(2)
                
                self.pandaSequence = Sequence()
                current_dir = self.pandaActor.get_hpr()
                for i in range(len(path)-1):
                    print(i, path[i], path[i+1])
                    vec = [ path[i+1][0] - path[i][0] , path[i+1][1] - path[i][1] ]
                    cosval = -1 * vec[1] / math.sqrt( vec[0]**2 + vec[1]**2 )
                    theta = math.acos(cosval) * 180/math.pi
                    if vec[0] < 0:
                        theta = 360 - theta
                    new_dir = LPoint3(theta, current_dir[1], current_dir[2])
                    rotate_speed = 90
                    #rotate_time = abs(theta-current_dir[0])/rotate_speed
                    rotate_time = 0
                    self.pandaSequence.append(self.pandaActor.hprInterval(rotate_time, new_dir, current_dir))

                    speed = 6
                    dist = math.dist(path[i+1],path[i])
                    print(dist)
                    self.pandaSequence.append(self.pandaActor.posInterval(dist/speed, path[i+1], path[i]))
                    current_dir = new_dir

                self.pandaSequence.start()
                self.pos1 = self.pos2
        return task.again



app = MyApp()
app.run()
