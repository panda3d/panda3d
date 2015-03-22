#!/usr/bin/env python

"""
Author: Josh Enes
Last Updated: 2015-03-13

This is a demo of Panda's portal-culling system. It demonstrates loading
portals from an EGG file, and shows an example method of selecting the
current cell using geoms and a collision ray.
"""

# Some config options which can be changed.
ENABLE_PORTALS = True # Set False to disable portal culling and see FPS drop!
DEBUG_PORTALS = False # Set True to see visually which portals are used

# Load PRC data
from panda3d.core import loadPrcFileData
if ENABLE_PORTALS:
    loadPrcFileData('', 'allow-portal-cull true')
    if DEBUG_PORTALS:
        loadPrcFileData('', 'debug-portal-cull true')
loadPrcFileData('', 'window-title Portal Demo')
loadPrcFileData('', 'sync-video false')
loadPrcFileData('', 'show-frame-rate-meter true')
loadPrcFileData('', 'texture-minfilter linear-mipmap-linear')

# Import needed modules
import random
from direct.showbase.ShowBase import ShowBase
from direct.gui.OnscreenText import OnscreenText
from panda3d.core import PerspectiveLens, NodePath, LVector3, LPoint3, \
    TexGenAttrib, TextureStage, TransparencyAttrib, CollisionTraverser, \
    CollisionHandlerQueue, TextNode, CollisionRay, CollisionNode


def add_instructions(pos, msg):
    """Function to put instructions on the screen."""
    return OnscreenText(text=msg, style=1, fg=(1, 1, 1, 1), shadow=(0, 0, 0, 1),
                        parent=base.a2dTopLeft, align=TextNode.ALeft,
                        pos=(0.08, -pos - 0.04), scale=.05)

def add_title(text):
    """Function to put title on the screen."""
    return OnscreenText(text=text, style=1, pos=(-0.1, 0.09), scale=.08,
                        parent=base.a2dBottomRight, align=TextNode.ARight,
                        fg=(1, 1, 1, 1), shadow=(0, 0, 0, 1))


class Game(ShowBase):
    """Sets up the game, camera, controls, and loads models."""
    def __init__(self):
        ShowBase.__init__(self)
        self.cellmanager = CellManager(self)
        self.xray_mode = False
        self.show_model_bounds = False

        # Display instructions
        add_title("Panda3D Tutorial: Portal Culling")
        add_instructions(0.06, "[Esc]: Quit")
        add_instructions(0.12, "[W]: Move Forward")
        add_instructions(0.18, "[A]: Move Left")
        add_instructions(0.24, "[S]: Move Right")
        add_instructions(0.30, "[D]: Move Back")
        add_instructions(0.36, "Arrow Keys: Look Around")
        add_instructions(0.42, "[F]: Toggle Wireframe")
        add_instructions(0.48, "[X]: Toggle X-Ray Mode")
        add_instructions(0.54, "[B]: Toggle Bounding Volumes")

        # Setup controls
        self.keys = {}
        for key in ['arrow_left', 'arrow_right', 'arrow_up', 'arrow_down',
                    'a', 'd', 'w', 's']:
            self.keys[key] = 0
            self.accept(key, self.push_key, [key, 1])
            self.accept('shift-%s' % key, self.push_key, [key, 1])
            self.accept('%s-up' % key, self.push_key, [key, 0])
        self.accept('f', self.toggleWireframe)
        self.accept('x', self.toggle_xray_mode)
        self.accept('b', self.toggle_model_bounds)
        self.accept('escape', __import__('sys').exit, [0])
        self.disableMouse()

        # Setup camera
        lens = PerspectiveLens()
        lens.setFov(60)
        lens.setNear(0.01)
        lens.setFar(1000.0)
        self.cam.node().setLens(lens)
        self.camera.setPos(-9, -0.5, 1)
        self.heading = -95.0
        self.pitch = 0.0

        # Load level geometry
        self.level_model = self.loader.loadModel('models/level')
        self.level_model.reparentTo(self.render)
        self.level_model.setTexGen(TextureStage.getDefault(),
                                   TexGenAttrib.MWorldPosition)
        self.level_model.setTexProjector(TextureStage.getDefault(),
                                         self.render, self.level_model)
        self.level_model.setTexScale(TextureStage.getDefault(), 4)
        tex = self.loader.load3DTexture('models/tex_#.png')
        self.level_model.setTexture(tex)

        # Load cells
        self.cellmanager.load_cells_from_model('models/cells')
        # Load portals
        self.cellmanager.load_portals_from_model('models/portals')

        # Randomly spawn some models to test the portals
        self.models = []
        for dummy in range(0, 500):
            pos = LPoint3((random.random() - 0.5) * 6,
                         (random.random() - 0.5) * 6,
                         random.random() * 7)
            cell = self.cellmanager.get_cell(pos)
            if cell is None: # skip if the random position is not over a cell
                continue
            dist = self.cellmanager.get_dist_to_cell(pos)
            if dist > 1.5: # skip if the random position is too far from ground
                continue
            box = self.loader.loadModel('box')
            box.setScale(random.random() * 0.2 + 0.1)
            box.setPos(pos)
            box.setHpr(random.random() * 360,
                         random.random() * 360,
                         random.random() * 360)
            box.reparentTo(cell.nodepath)
            self.models.append(box)
        self.taskMgr.add(self.update, 'main loop')

    def push_key(self, key, value):
        """Stores a value associated with a key."""
        self.keys[key] = value

    def update(self, task):
        """Updates the camera based on the keyboard input. Once this is
        done, then the CellManager's update function is called."""
        delta = globalClock.getDt()
        move_x = delta * 3 * -self.keys['a'] + delta * 3 * self.keys['d']
        move_z = delta * 3 * self.keys['s'] + delta * 3 * -self.keys['w']
        self.camera.setPos(self.camera, move_x, -move_z, 0)
        self.heading += (delta * 90 * self.keys['arrow_left'] +
                         delta * 90 * -self.keys['arrow_right'])
        self.pitch += (delta * 90 * self.keys['arrow_up'] +
                       delta * 90 * -self.keys['arrow_down'])
        self.camera.setHpr(self.heading, self.pitch, 0)
        if ENABLE_PORTALS:
            self.cellmanager.update()
        return task.cont

    def toggle_xray_mode(self):
        """Toggle X-ray mode on and off. This is useful for seeing the
        effectiveness of the portal culling."""
        self.xray_mode = not self.xray_mode
        if self.xray_mode:
            self.level_model.setColorScale((1, 1, 1, 0.5))
            self.level_model.setTransparency(TransparencyAttrib.MDual)
        else:
            self.level_model.setColorScaleOff()
            self.level_model.setTransparency(TransparencyAttrib.MNone)

    def toggle_model_bounds(self):
        """Toggle bounding volumes on and off on the models."""
        self.show_model_bounds = not self.show_model_bounds
        if self.show_model_bounds:
            for model in self.models:
                model.showBounds()
        else:
            for model in self.models:
                model.hideBounds()


class CellManager(object):
    """Creates a collision ray and collision traverser to use for
    selecting the current cell."""
    def __init__(self, game):
        self.game = game
        self.cells = {}
        self.cells_by_collider = {}
        self.cell_picker_world = NodePath('cell_picker_world')
        self.ray = CollisionRay()
        self.ray.setDirection(LVector3.down())
        cnode = CollisionNode('cell_raycast_cnode')
        self.ray_nodepath = self.cell_picker_world.attachNewNode(cnode)
        self.ray_nodepath.node().addSolid(self.ray)
        self.ray_nodepath.node().setIntoCollideMask(0) # not for colliding into
        self.ray_nodepath.node().setFromCollideMask(1)
        self.traverser = CollisionTraverser('traverser')
        self.last_known_cell = None

    def add_cell(self, collider, name):
        """Add a new cell."""
        cell = Cell(self, name, collider)
        self.cells[name] = cell
        self.cells_by_collider[collider.node()] = cell

    def get_cell(self, pos):
        """Given a position, return the nearest cell below that position.
        If no cell is found, returns None."""
        self.ray.setOrigin(pos)
        queue = CollisionHandlerQueue()
        self.traverser.addCollider(self.ray_nodepath, queue)
        self.traverser.traverse(self.cell_picker_world)
        self.traverser.removeCollider(self.ray_nodepath)
        queue.sortEntries()
        if not queue.getNumEntries():
            return None
        entry = queue.getEntry(0)
        cnode = entry.getIntoNode()
        try:
            return self.cells_by_collider[cnode]
        except KeyError:
            raise Warning('collision ray collided with something '
                          'other than a cell: %s' % cnode)

    def get_dist_to_cell(self, pos):
        """Given a position, return the distance to the nearest cell
        below that position. If no cell is found, returns None."""
        self.ray.setOrigin(pos)
        queue = CollisionHandlerQueue()
        self.traverser.addCollider(self.ray_nodepath, queue)
        self.traverser.traverse(self.cell_picker_world)
        self.traverser.removeCollider(self.ray_nodepath)
        queue.sortEntries()
        if not queue.getNumEntries():
            return None
        entry = queue.getEntry(0)
        return (entry.getSurfacePoint(self.cell_picker_world) - pos).length()

    def load_cells_from_model(self, modelpath):
        """Loads cells from an EGG file. Cells must be named in the
        format "cell#" to be loaded by this function."""
        cell_model = self.game.loader.loadModel(modelpath)
        for collider in cell_model.findAllMatches('**/+GeomNode'):
            name = collider.getName()
            if name.startswith('cell'):
                self.add_cell(collider, name[4:])
        cell_model.removeNode()

    def load_portals_from_model(self, modelpath):
        """Loads portals from an EGG file. Portals must be named in the
        format "portal_#to#_*" to be loaded by this function, whereby the
        first # is the from cell, the second # is the into cell, and * can
        be anything."""
        portal_model = loader.loadModel(modelpath)
        portal_nodepaths = portal_model.findAllMatches('**/+PortalNode')
        for portal_nodepath in portal_nodepaths:
            name = portal_nodepath.getName()
            if name.startswith('portal_'):
                from_cell_id, into_cell_id = name.split('_')[1].split('to')
                try:
                    from_cell = self.cells[from_cell_id]
                except KeyError:
                    print ('could not load portal "%s" because cell "%s"'
                           'does not exist' % (name, from_cell_id))
                    continue
                try:
                    into_cell = self.cells[into_cell_id]
                except KeyError:
                    print ('could not load portal "%s" because cell "%s"'
                           'does not exist' % (name, into_cell_id))
                    continue
                from_cell.add_portal(portal_nodepath, into_cell)
        portal_model.removeNode()

    def update(self):
        """Show the cell the camera is currently in and hides the rest.
        If the camera is not in a cell, use the last known cell that the
        camera was in. If the camera has not yet been in a cell, then all
        cells will be hidden."""
        camera_pos = self.game.camera.getPos(self.game.render)
        for cell in self.cells:
            self.cells[cell].nodepath.hide()
        current_cell = self.get_cell(camera_pos)
        if current_cell is None:
            if self.last_known_cell is None:
                return
            self.last_known_cell.nodepath.show()
        else:
            self.last_known_cell = current_cell
            current_cell.nodepath.show()


class Cell(object):
    """The Cell class is a handy way to keep an association between
    all the related nodes and information of a cell."""
    def __init__(self, cellmanager, name, collider):
        self.cellmanager = cellmanager
        self.name = name
        self.collider = collider
        self.collider.reparentTo(self.cellmanager.cell_picker_world)
        self.collider.setCollideMask(1)
        self.collider.hide()
        self.nodepath = NodePath('cell_%s_root' % name)
        self.nodepath.reparentTo(self.cellmanager.game.render)
        self.portals = []

    def add_portal(self, portal, cell_out):
        """Add a portal from this cell going into another one."""
        portal.reparentTo(self.nodepath)
        portal.node().setCellIn(self.nodepath)
        portal.node().setCellOut(cell_out.nodepath)
        self.portals.append(portal)

game = Game()
game.run()
