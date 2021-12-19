# all imports needed by the engine itself
from direct.showbase.ShowBase import ShowBase

# import our own repositories
from ClientRepository import GameClientRepository

# initialize the engine
base = ShowBase()

# initialize the client
client = GameClientRepository()

base.accept("escape", exit)




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
inst1 = addInstructions(0.06, "esc: Close the client")
inst2 = addInstructions(0.12, "See console output")

def setConnectedMessage():
    title["text"] = "Panda3D: Tutorial - Distributed Network (CONNECTED)"

base.accept("client-joined", setConnectedMessage)

# start the client
base.run()
