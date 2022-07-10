import sys
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.ShowBase import ShowBase
from panda3d.core import *
from direct.actor.Actor import Actor
from direct.interval.IntervalGlobal import *
from panda3d.navigation import NavMeshNode, NavMeshQuery
from panda3d.navmeshgen import NavMeshBuilder


# Function to put instructions on the screen.
def add_instructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(1, 1, 1, 1), scale=.05,
                        shadow=(0, 0, 0, 1), parent=base.a2dTopLeft,
                        pos=(0.08, -pos - 0.04), align=TextNode.A_left)


# Function to put title on the screen.
def add_title(text):
    return OnscreenText(text=text, style=1, fg=(1, 1, 1, 1), scale=.07,
                        parent=base.a2dBottomRight, align=TextNode.A_right,
                        pos=(-0.1, 0.09), shadow=(0, 0, 0, 1))


class NavigationDemo(ShowBase):

    def __init__(self):
        ShowBase.__init__(self)

        # Setting up light for better view.
        plight = PointLight('plight')
        plight.set_color((0.9, 0.9, 0.9, 0.5))
        plnp = self.render.attach_new_node(plight)
        plnp.set_pos(10, 20, 0)
        self.render.set_light(plnp)
        dlight = DirectionalLight('dlight')
        dlight.set_color((0.8, 0.8, 0.8, 1))
        dlnp = self.render.attach_new_node(dlight)
        dlnp.set_hpr(0, -60, 0)
        self.render.set_light(dlnp)

        # Loading the model
        self.scene = self.loader.loadModel("models/nav_test.egg")
        self.scene.set_scale(5)
        self.scene.flatten_light()
        self.scene.reparent_to(self.render)

        self.setFrameRateMeter(1)

        maze = self.scene.find('**/maze')
        maze.setX(-10)
        self.scene.flatten_light()
        self.movement_loop = LerpPosInterval(maze, 3, maze.get_pos() + (20, 0, 0))
        self.movement_loop.loop()

        #obstacle = self.loader.loadModel("models/panda-model")
        #obstacle.reparent_to(self.render)
        #obstacle.set_scale(0.01)
        #self.movement_loop = LerpPosInterval(obstacle, 3, (20, 0, 0))
        #self.movement_loop.loop()

        # NavMeshBuilder is a class that is responsible for building the polygon meshes and navigation meshes.
        self.builder = NavMeshBuilder()
        # Take NodePath as input. This method only uses the collision nodes that are under this node.
        self.builder.from_coll_node_path(self.scene, tracked_node=True)

        #self.builder.from_node_path(obstacle, tracked_node=True)

        self.builder.actor_height = 10
        self.builder.actor_radius = 4
        self.builder.actor_max_climb = 2
        self.builder.tile_size = 64
        self.builder.cell_size = 1
        self.navmesh = self.builder.build()

        self.navmeshnode = NavMeshNode("scene", self.navmesh)
        self.navmeshnodepath: NodePath = self.scene.attach_new_node(self.navmeshnode)

        self.accept("m", self.toggle_nav_mesh)

        # Uncomment the line below to save the generated navmesh to file.
        # self.navmeshnodepath.write_bam_file("scene_navmesh.bam")

        # Uncomment the following section to read the generated navmesh from file.
        # self.navmeshnodepath.remove_node()
        # self.navmeshnodepath = self.loader.loadModel("scene_navmesh.bam")
        # self.navmeshnodepath.reparent_to(self.scene)
        # self.navmeshnode: NavMeshNode = self.navmeshnodepath.node()
        # self.navmesh = self.navmeshnode.get_nav_mesh()

        self.destinationMarker = self.loader.loadModel("misc/objectHandles.egg")
        self.destinationMarker.set_scale(3)
        self.destinationMarker.reparent_to(self.scene)

        self.pandaActor = Actor("models/panda-model",
                                {"walk": "models/panda-walk4"})
        self.pandaActor.set_scale(0.01)
        self.pandaActor.reparent_to(self.scene)
        self.pandaActor.loop("walk")

        self.pandaSequence = Sequence()
        self.pandaSequence.start()

        self.lineNodePath = NodePath()

        # Setup mouse picking.
        self.pickerNode = CollisionNode('mouseRay')
        self.pickerNP = self.camera.attachNewNode(self.pickerNode)
        self.pickerNode.setFromCollideMask(GeomNode.getDefaultCollideMask())
        self.pickerRay = CollisionRay()
        self.pickerNode.addSolid(self.pickerRay)
        self.clickTraverser = CollisionTraverser()
        self.clickHandler = CollisionHandlerQueue()
        self.clickTraverser.addCollider(self.pickerNP, self.clickHandler)

        # Use the mouse click traverser to pull the panda to the ground.
        self.pickerRay.setDirection(0, 0, -1)
        self.clickTraverser.traverse(self.scene)
        if self.clickHandler.getNumEntries() > 0:
            self.clickHandler.sortEntries()
            collision_entry = self.clickHandler.getEntry(0)
            self.pos1 = collision_entry.getSurfacePoint(self.scene)

        # Initialize the NavMeshQuery that we will use.
        self.query = NavMeshQuery(self.navmesh)

        # Snap the start position to the navmesh.
        self.query.nearestPoint(self.pos1)

        self.pandaActor.set_pos(self.pos1)
        self.destinationMarker.set_pos(self.pos1)
        self.pos2 = self.pos1

        # Set up camera for better initial view.
        self.trackball.node().set_pos(-5.6501, 152.272, -8.68381)
        self.trackball.node().set_hpr(54.3341, 16.1934, -22.4152)

        # Set up info text
        self.title = add_title("Panda3D Tutorial: Navigation (NavMesh generation and usage)")
        self.inst1 = add_instructions(0.06, "[ESC]: Quit")
        self.inst2 = add_instructions(0.12, "[Left Mouse]: Pan")
        self.inst3 = add_instructions(0.18, "[Middle Mouse]: Rotate")
        self.inst4 = add_instructions(0.24, "[Right Mouse]: Zoom")
        self.inst5 = add_instructions(0.30, "[Shift + Left Click]: Set Navigation Destination")
        self.inst6 = add_instructions(0.36, "[M]: Toggle Navmesh Visibility")

        self.accept("escape", sys.exit)
        self.accept('shift-mouse1', self.handle_mouse_click)

        taskMgr.doMethodLater(0.25, self.update, 'NavMesh-update')

    def update(self, task):
        self.navmesh.update()
        return task.again

    def toggle_nav_mesh(self):
        if self.navmeshnodepath.is_hidden():
            self.navmeshnodepath.show()
        else:
            self.navmeshnodepath.hide()

    def handle_mouse_click(self):
        if self.mouseWatcherNode.is_button_down(KeyboardButton.shift()):
            mpos = self.mouseWatcherNode.get_mouse()
            # Send a ray from the mouse click location.
            self.pickerRay.set_from_lens(self.camNode, mpos.x, mpos.y)

            # Find out what it hits.
            self.clickTraverser.traverse(self.scene)
            # Make sure we hit something.
            if self.clickHandler.get_num_entries() > 0:
                # Get the closest object first.
                self.clickHandler.sort_entries()
                # Get the closest object.
                collision_entry = self.clickHandler.get_entry(0)

                self.navmesh.update()

                # Find the point on the geometry that we hit relative to the scene.
                self.pos2 = collision_entry.get_surface_point(self.scene)

                self.pos1 = self.pandaActor.get_pos()
                self.pandaSequence.finish()

                self.query.nearest_point(self.pos2)
                self.destinationMarker.set_pos(self.pos2)
                path = self.query.find_smooth_path(self.pos1, self.pos2)
                path_points = list(path.points)
                self.pathLine = LineSegs()
                self.pathLine.set_color(0, 1, 0)
                self.pathLine.set_thickness(5)

                for point in path_points:
                    self.pathLine.draw_to(point)

                self.pathLine.drawTo(self.pos2)
                self.lineNode = self.pathLine.create()
                self.lineNodePath.remove_node()
                self.lineNodePath = self.scene.attach_new_node(self.lineNode)
                self.lineNodePath.set_z(1)

                self.pandaSequence = Sequence()
                current_dir = self.pandaActor.get_hpr()
                path_points.append(self.pos2)
                for i in range(len(path_points) - 1):
                    new_hpr = Vec3(Vec2(0, -1).signed_angle_deg(path_points[i + 1].xy - path_points[i].xy), current_dir[1], current_dir[2])
                    self.pandaSequence.append(self.pandaActor.hprInterval(0, new_hpr))

                    speed = 20
                    dist = (path_points[i + 1] - path_points[i]).length()
                    self.pandaSequence.append(self.pandaActor.posInterval(dist/speed, path_points[i+1], path_points[i]))
                    current_dir = new_hpr

                self.pandaSequence.start()
                self.pos1 = self.pos2


app = NavigationDemo()
app.run()
