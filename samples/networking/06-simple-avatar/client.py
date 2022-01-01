# all imports needed by the engine itself
from direct.showbase.ShowBase import ShowBase
from panda3d.core import KeyboardButton, NodePath, PandaNode

# import our own repositories
from ClientRepository import GameClientRepository
from DistributedSmoothActor import DistributedSmoothActor

# initialize the engine
base = ShowBase()

base.disableMouse()

class Avatar:
    def __init__(self, cr):
        self.cr = cr
        self.ralph = DistributedSmoothActor(self.cr)

        self.cr.createDistributedObject(
            distObj = self.ralph,
            zoneId = 2)

        # Create a floater object, which floats 2 units above ralph.  We
        # use this as a target for the camera to look at.

        self.floater = NodePath(PandaNode("floater"))
        self.floater.reparentTo(self.ralph)
        self.floater.setZ(2.0)

        # We will use this for checking if keyboard keys are pressed
        self.isDown = base.mouseWatcherNode.isButtonDown

        taskMgr.add(self.move, "moveTask")

        # Set up the camera
        base.camera.setPos(self.ralph.getX(), self.ralph.getY() + 10, 2)

        # start the avatar
        self.ralph.start()

    # Accepts arrow keys to move either the player or the menu cursor,
    # Also deals with grid checking and collision detection
    def move(self, task):

        # Get the time that elapsed since last frame.  We multiply this with
        # the desired speed in order to find out with which distance to move
        # in order to achieve that desired speed.
        dt = base.clock.dt

        # If the camera-left key is pressed, move camera left.
        # If the camera-right key is pressed, move camera right.

        if self.isDown(KeyboardButton.asciiKey(b"j")):
            base.camera.setX(base.camera, -20 * dt)
        if self.isDown(KeyboardButton.asciiKey(b"k")):
            base.camera.setX(base.camera, +20 * dt)

        # If a move-key is pressed, move ralph in the specified direction.

        if self.isDown(KeyboardButton.asciiKey(b"a")):
            self.ralph.setH(self.ralph.getH() + 300 * dt)
        if self.isDown(KeyboardButton.asciiKey(b"d")):
            self.ralph.setH(self.ralph.getH() - 300 * dt)
        if self.isDown(KeyboardButton.asciiKey(b"w")):
            self.ralph.setY(self.ralph, -20 * dt)
        if self.isDown(KeyboardButton.asciiKey(b"s")):
            self.ralph.setY(self.ralph, +10 * dt)

        # update distributed position and rotation
        #self.ralph.setDistPos(self.ralph.getX(), self.ralph.getY(), self.ralph.getZ())
        #self.ralph.setDistHpr(self.ralph.getH(), self.ralph.getP(), self.ralph.getR())

        # If ralph is moving, loop the run animation.
        # If he is standing still, stop the animation.
        currentAnim = self.ralph.getCurrentAnim()

        if self.isDown(KeyboardButton.asciiKey(b"w")):
            if currentAnim != "run":
                self.ralph.loop("run")
        elif self.isDown(KeyboardButton.asciiKey(b"s")):
            # Play the walk animation backwards.
            if currentAnim != "walk":
                self.ralph.loop("walk")
            self.ralph.setPlayRate(-1.0, "walk")
        elif self.isDown(KeyboardButton.asciiKey(b"a")) or self.isDown(KeyboardButton.asciiKey(b"d")):
            if currentAnim != "walk":
                self.ralph.loop("walk")
            self.ralph.setPlayRate(1.0, "walk")
        else:
            if currentAnim is not None:
                self.ralph.stop()
                self.ralph.pose("walk", 5)
                self.isMoving = False

        # If the camera is too far from ralph, move it closer.
        # If the camera is too close to ralph, move it farther.

        camvec = self.ralph.getPos() - base.camera.getPos()
        camvec.setZ(0)
        camdist = camvec.length()
        camvec.normalize()
        if camdist > 10.0:
            base.camera.setPos(base.camera.getPos() + camvec * (camdist - 10))
            camdist = 10.0
        if camdist < 5.0:
            base.camera.setPos(base.camera.getPos() - camvec * (5 - camdist))
            camdist = 5.0

        # The camera should look in ralph's direction,
        # but it should also try to stay horizontal, so look at
        # a floater which hovers above ralph's head.
        base.camera.lookAt(self.floater)

        return task.cont

base.accept("escape", exit)

# initialize the client
client = GameClientRepository()


from direct.gui.OnscreenText import OnscreenText
from panda3d.core import TextNode

# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(0, 0, 0, 1), shadow=(1, 1, 1, 1),
                        parent=base.a2dTopLeft, align=TextNode.ALeft,
                        pos=(0.08, -pos - 0.04), scale=.06)

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, pos=(-0.1, 0.09), scale=.08,
                        parent=base.a2dBottomRight, align=TextNode.ARight,
                        fg=(1, 1, 1, 1), shadow=(0, 0, 0, 1))

title = addTitle("Panda3D: Tutorial - Distributed Network (NOT CONNECTED)")
inst1 = addInstructions(0.06, "W|A|S|D: Move avatar)")
inst2 = addInstructions(0.12, "esc: Close the client")
inst3 = addInstructions(0.24, "See console output")

def clientJoined():
    title["text"] = "Panda3D: Tutorial - Distributed Network (CONNECTED)"

    # Setup our avatar
    Avatar(client)

base.accept("client-joined", clientJoined)

# start the client
base.run()
