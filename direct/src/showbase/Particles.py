from PandaObject import *
from ParticleManagerGlobal import *
from PhysicsManagerGlobal import *
from TaskManagerGlobal import *

import ParticleSystem
import BaseParticleFactory
import PointParticleFactory
import ZSpinParticleFactory
import OrientedParticleFactory
import BaseParticleRenderer
import PointParticleRenderer
import LineParticleRenderer
import GeomParticleRenderer
import SparkleParticleRenderer
import SpriteParticleRenderer
import BaseParticleEmitter
import BoxEmitter
import DiscEmitter
import LineEmitter
import PointEmitter
import RectangleEmitter
import RingEmitter
import SphereSurfaceEmitter
import SphereVolumeEmitter
import TangentRingEmitter
import PhysicalNode
import ForceNode
import RenderRelation
import LinearEulerIntegrator
import ClockObject

globalClock = ClockObject.ClockObject.getGlobalClock()

class Particles(ParticleSystem.ParticleSystem):

    def __init__(self, poolSize = 1024):
	"""__init__(self)"""

	ParticleSystem.ParticleSystem.__init__(self, poolSize)
	self.setBirthRate(0.02)
	self.setLitterSize(10)
	self.setLitterSpread(0)
	self.setRenderParent(render.node())

	self.factory = None 
	self.setFactory("Point")
	self.renderer = None 
	self.setRenderer("Line")
	self.emitter = None 
	self.setEmitter("Sphere Volume")

	self.node = PhysicalNode.PhysicalNode()
	self.node.addPhysical(self)
	self.nodePath = hidden.attachNewNode(self.node)
	#self.forceNode = ForceNode.ForceNode()

	self.integrator = LinearEulerIntegrator.LinearEulerIntegrator()

	physicsMgr.attachLinearIntegrator(self.integrator)
	physicsMgr.attachPhysical(self)

	particleMgr.setFrameStepping(1)
	particleMgr.attachParticlesystem(self)

    def setFactory(self, type):
	"""setFactory(self, type)"""
	if (self.factory):
	    self.factory = None
	if (type == "Point"):
	    self.factory = PointParticleFactory.PointParticleFactory() 
	elif (type == "Z Spin"):
	    self.factory = ZSpinParticleFactory.ZSpinParticleFactory()
	elif (type == "Oriented"):
	    self.factory = OrientedParticleFactory.OrientedParticleFactory()
	else:
	    print "unknown factory type: %s" % type
	    return None
	#self.factory.setLifespanBase(0.5)
	ParticleSystem.ParticleSystem.setFactory(self, self.factory)

    def setRenderer(self, type):
	"""setRenderer(self, type)"""
	if (self.renderer):
	    self.renderer = None
	if (type == "Point"):
	    self.renderer = PointParticleRenderer.PointParticleRenderer()
	    self.renderer.setPointSize(1.0)
	elif (type == "Line"):
	    self.renderer = LineParticleRenderer.LineParticleRenderer()
	    self.renderer.setHeadColor(Vec4(1.0, 1.0, 1.0, 1.0))
	    self.renderer.setTailColor(Vec4(1.0, 1.0, 1.0, 1.0))
	elif (type == "Geom"):
	    self.renderer = GeomParticleRenderer.GeomParticleRenderer()
	elif (type == "Sparkle"):
	    self.renderer = SparkleParticleRenderer.SparkleParticleRenderer()
	elif (type == "Sprite"):
	    self.renderer = SpriteParticleRenderer.SpriteParticleRenderer()
	else:
	    print "unknown renderer type: %s" % type
	    return None
	#self.renderer.setAlphaMode(
        #BaseParticleRenderer.BaseParticleRenderer.PRALPHAUSER)
	#self.renderer.setUserAlpha(1.0)
	ParticleSystem.ParticleSystem.setRenderer(self, self.renderer)

    def setEmitter(self, type):
	"""setEmitter(self, type)"""
	if (self.emitter):
	    self.emitter = None
	if (type == "Box"):
	    self.emitter = BoxEmitter.BoxEmitter()
	elif (type == "Disc"):
	    self.emitter = DiscEmitter.DiscEmitter()
	elif (type == "Line"):
	    self.emitter = LineEmitter.LineEmitter()
	elif (type == "Point"):
	    self.emitter = PointEmitter.PointEmitter()
	elif (type == "Rectangle"):
	    self.emitter = RectangleEmitter.RectangleEmitter()
	elif (type == "Ring"):
	    self.emitter = RingEmitter.RingEmitter()
	elif (type == "Sphere Surface"):
	    self.emitter = SphereSurfaceEmitter.SphereSurfaceEmitter()
	elif (type == "Sphere Volume"):
	    self.emitter = SphereVolumeEmitter.SphereVolumeEmitter()
	    self.emitter.setRadius(1.0)
	elif (type == "Tangent Ring"):
	    self.emitter = TangentRingEmitter.TangentRingEmitter()
	else:
	    print "unknown emitter type: %s" % type
	    return None
	#self.emitter.setEmissionType(
        #BaseParticleEmitter.BaseParticleEmitter.ETEXPLICIT)
	#self.emitter.setExplicitLaunchVector(Vec3(-1.0, -1.0, 1.0))
	#self.emitter.setAmplitude(1.0)
	ParticleSystem.ParticleSystem.setEmitter(self, self.emitter)

    def __update(self, state):
	"""update(self, state)"""
        dt = globalClock.getDt()
        physicsMgr.doPhysics(dt)
        particleMgr.doParticles(dt)
        return Task.cont

    def getNodePath(self):
	"""getNode(self)"""
	return self.nodePath

    def start(self):
	"""start(self)"""
	taskMgr.spawnTaskNamed(Task.Task(self.__update), 'update-particles')

    def stop(self):
	"""stop(self)"""
	taskMgr.removeTasksNamed('update-particles')
