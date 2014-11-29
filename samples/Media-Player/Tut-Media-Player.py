from panda3d.core import *
# Tell Panda3D to use OpenAL, not FMOD
loadPrcFileData("", "audio-library-name p3openal_audio")
from direct.showbase.DirectObject import DirectObject
from direct.gui.OnscreenText import OnscreenText
import direct.directbase.DirectStart

# The name of the media file.
MEDIAFILE="PandaSneezes.avi"

# Function to put instructions on the screen.
def addInstructions(pos, msg):
    return OnscreenText(text=msg, style=1, fg=(0,0,0,1), mayChange=1,
                        pos=(-1.3, pos), align=TextNode.ALeft, scale = .05, shadow=(1,1,1,1), shadowOffset=(0.1,0.1))

# Function to put title on the screen.
def addTitle(text):
    return OnscreenText(text=text, style=1, fg=(0,0,0,1),
                        pos=(1.3,-0.95), align=TextNode.ARight, scale = .07, shadow=(1,1,1,1), shadowOffset=(0.05,0.05))


class World(DirectObject):
  def __init__(self):
    self.title = addTitle("Panda3D: Tutorial - Media Player")
    self.inst1 = addInstructions(0.95,"P: Play/Pause")
    self.inst2 = addInstructions(0.90,"S: Stop and Rewind")
    self.inst3 = addInstructions(0.85,"M: Slow Motion / Normal Motion toggle")
    
    # Load the texture. We could use loader.loadTexture for this,
    # but we want to make sure we get a MovieTexture, since it
    # implements synchronizeTo.
    self.tex = MovieTexture("name")
    assert self.tex.read(MEDIAFILE), "Failed to load video!"
    
    # Set up a fullscreen card to set the video texture on.
    cm = CardMaker("My Fullscreen Card");
    cm.setFrameFullscreenQuad()
    cm.setUvRange(self.tex)
    card = NodePath(cm.generate())
    card.reparentTo(render2d)
    card.setTexture(self.tex)
    card.setTexScale(TextureStage.getDefault(), self.tex.getTexScale())
    self.sound=loader.loadSfx(MEDIAFILE)
    # Synchronize the video to the sound.
    self.tex.synchronizeTo(self.sound)
    
    self.accept('p', self.playpause)
    self.accept('P', self.playpause)
    self.accept('s', self.stopsound)
    self.accept('S', self.stopsound)
    self.accept('m', self.fastforward)
    self.accept('M', self.fastforward)

  def stopsound(self):
    self.sound.stop()
    self.sound.setPlayRate(1.0)

  def fastforward(self):
    print self.sound.status()
    if (self.sound.status() == AudioSound.PLAYING):
      t = self.sound.getTime()
      self.sound.stop()
      if (self.sound.getPlayRate() == 1.0):
        self.sound.setPlayRate(0.5)
      else:
        self.sound.setPlayRate(1.0)
      self.sound.setTime(t)
      self.sound.play()

  def playpause(self):
    if (self.sound.status() == AudioSound.PLAYING):
      t = self.sound.getTime()
      self.sound.stop()
      self.sound.setTime(t)
    else:
      self.sound.play()

w = World()
run()

