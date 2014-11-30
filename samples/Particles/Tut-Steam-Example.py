# Author: Shao Zhang and Phil Saltzman
# Last Updated: 4/20/2005
#
# This tutorial shows how to take an existing particle effect taken from a
# .ptf file and run it in a general Panda project. 

import direct.directbase.DirectStart
from panda3d.physics import BaseParticleEmitter,BaseParticleRenderer
from panda3d.physics import PointParticleFactory,SpriteParticleRenderer
from panda3d.physics import LinearNoiseForce,DiscEmitter
from panda3d.core import TextNode
from panda3d.core import AmbientLight,DirectionalLight
from panda3d.core import Point3,Vec3,Vec4
from panda3d.core import Filename
from direct.particles.Particles import Particles
from direct.particles.ParticleEffect import ParticleEffect
from direct.particles.ForceGroup import ForceGroup
from direct.gui.OnscreenText import OnscreenText
from direct.showbase.DirectObject import DirectObject
import sys

HELPTEXT = """
1: Load Steam
2: Load Dust
3: Load Fountain
4: Load Smoke
5: Load Smokering
6: Load Fireish
ESC: Quit
"""

class World(DirectObject):
    def __init__(self):
        #Standard title and instruction text
        self.title = OnscreenText(
            text="Panda3D: Tutorial - Particles",
            style=1, fg=(1,1,1,1), pos=(0.8,-0.95), scale = .07)
        self.escapeEvent = OnscreenText(
            text=HELPTEXT,
            style=1, fg=(1,1,1,1), pos=(-1.3, 0.95),
            align=TextNode.ALeft, scale = .05)

        #More standard initialization
        self.accept('escape', sys.exit)
        self.accept('1', self.loadParticleConfig , ['steam.ptf'])
        self.accept('2', self.loadParticleConfig , ['dust.ptf'])
        self.accept('3', self.loadParticleConfig , ['fountain.ptf'])
        self.accept('4', self.loadParticleConfig , ['smoke.ptf'])
        self.accept('5', self.loadParticleConfig , ['smokering.ptf'])
        self.accept('6', self.loadParticleConfig , ['fireish.ptf'])
        
        self.accept('escape', sys.exit)
        base.disableMouse()
        camera.setPos(0,-20,2)
        base.setBackgroundColor( 0, 0, 0 )

        #This command is required for Panda to render particles
        base.enableParticles()
        self.t = loader.loadModel("models/teapot")
        self.t.setPos(0,10,0)
        self.t.reparentTo(render)
        self.setupLights()
        self.p = ParticleEffect()
        self.loadParticleConfig('steam.ptf')

    def loadParticleConfig(self, file):
        #Start of the code from steam.ptf
        self.p.cleanup()
        self.p = ParticleEffect()
        self.p.loadConfig(Filename(file))        
        #Sets particles to birth relative to the teapot, but to render at toplevel
        self.p.start(self.t)
        self.p.setPos(3.000, 0.000, 2.250)
    
    #Setup lighting
    def setupLights(self):
        ambientLight = AmbientLight("ambientLight")
        ambientLight.setColor(Vec4(.4, .4, .35, 1))
        directionalLight = DirectionalLight("directionalLight")
        directionalLight.setDirection(Vec3( 0, 8, -2.5 ) )
        directionalLight.setColor(Vec4( 0.9, 0.8, 0.9, 1 ) )
        #Set lighting on teapot so steam doesn't get affected
        self.t.setLight(self.t.attachNewNode(directionalLight))
        self.t.setLight(self.t.attachNewNode(ambientLight))

w = World()
run()

        
        
    

