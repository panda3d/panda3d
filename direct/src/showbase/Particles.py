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
SparkleParticleRenderer.SparkleParticleRenderer.SPNOSCALE = 0
SparkleParticleRenderer.SparkleParticleRenderer.SPSCALE = 1

class Particles(ParticleSystem.ParticleSystem):

    def __init__(self, poolSize = 1024):
	"""__init__(self)"""

	ParticleSystem.ParticleSystem.__init__(self, poolSize)
	self.setBirthRate(0.02)
	self.setLitterSize(10)
	self.setLitterSpread(0)
	self.setRenderParent(render.node())

	self.factory = None 
	self.factoryType = "undefined"
	self.setFactory("PointParticleFactory")
	self.renderer = None 
	self.rendererType = "undefined"
	self.setRenderer("PointParticleRenderer")
	self.emitter = None 
	self.emitterType = "undefined"
	self.setEmitter("SphereVolumeEmitter")

	self.node = PhysicalNode.PhysicalNode()
	self.node.addPhysical(self)
	self.nodePath = hidden.attachNewNode(self.node)
	self.forceNode = ForceNode.ForceNode()

	self.integrator = LinearEulerIntegrator.LinearEulerIntegrator()

	physicsMgr.attachLinearIntegrator(self.integrator)
	physicsMgr.attachPhysical(self)

	particleMgr.setFrameStepping(1)
	particleMgr.attachParticlesystem(self)

    def cleanup(self):
	"""cleanup(self)"""
	physicsMgr.removePhysical(self)
	particleMgr.removeParticlesystem(self)

    def setFactory(self, type):
	"""setFactory(self, type)"""
	if (self.factoryType == type):
	    return None
	if (self.factory):
	    self.factory = None
	self.factoryType = type
	if (type == "PointParticleFactory"):
	    self.factory = PointParticleFactory.PointParticleFactory() 
	elif (type == "ZSpinParticleFactory"):
	    self.factory = ZSpinParticleFactory.ZSpinParticleFactory()
	elif (type == "OrientedParticleFactory"):
	    self.factory = OrientedParticleFactory.OrientedParticleFactory()
	else:
	    print "unknown factory type: %s" % type
	    return None
	self.factory.setLifespanBase(0.5)
	ParticleSystem.ParticleSystem.setFactory(self, self.factory)

    def setRenderer(self, type):
	"""setRenderer(self, type)"""
	if (self.rendererType == type):
	    return None
	if (self.renderer):
	    self.renderer = None
	self.rendererType = type
	if (type == "PointParticleRenderer"):
	    self.renderer = PointParticleRenderer.PointParticleRenderer()
	    self.renderer.setPointSize(1.0)
	elif (type == "LineParticleRenderer"):
	    self.renderer = LineParticleRenderer.LineParticleRenderer()
	    self.renderer.setPointSize(1.0)
	elif (type == "GeomParticleRenderer"):
	    self.renderer = GeomParticleRenderer.GeomParticleRenderer()
	elif (type == "SparkleParticleRenderer"):
	    self.renderer = SparkleParticleRenderer.SparkleParticleRenderer()
	elif (type == "SpriteParticleRenderer"):
	    self.renderer = SpriteParticleRenderer.SpriteParticleRenderer()
	    t = loader.loadTexture("I:/beta/toons/install/maps/evil_eye.rgb")
	    if (t == None):
		print "Couldn't find default texture: evil_eye.rgb!"
		return None
	    self.renderer.setTexture(t)
	else:
	    print "unknown renderer type: %s" % type
	    return None
	ParticleSystem.ParticleSystem.setRenderer(self, self.renderer)

    def setEmitter(self, type):
	"""setEmitter(self, type)"""
	if (self.emitterType == type):
	    return None
	if (self.emitter):
	    self.emitter = None
	self.emitterType = type
	if (type == "BoxEmitter"):
	    self.emitter = BoxEmitter.BoxEmitter()
	elif (type == "DiscEmitter"):
	    self.emitter = DiscEmitter.DiscEmitter()
	elif (type == "LineEmitter"):
	    self.emitter = LineEmitter.LineEmitter()
	elif (type == "PointEmitter"):
	    self.emitter = PointEmitter.PointEmitter()
	elif (type == "RectangleEmitter"):
	    self.emitter = RectangleEmitter.RectangleEmitter()
	elif (type == "RingEmitter"):
	    self.emitter = RingEmitter.RingEmitter()
	elif (type == "SphereSurfaceEmitter"):
	    self.emitter = SphereSurfaceEmitter.SphereSurfaceEmitter()
	elif (type == "SphereVolumeEmitter"):
	    self.emitter = SphereVolumeEmitter.SphereVolumeEmitter()
	    self.emitter.setRadius(1.0)
	elif (type == "TangentRingEmitter"):
	    self.emitter = TangentRingEmitter.TangentRingEmitter()
	else:
	    print "unknown emitter type: %s" % type
	    return None
	ParticleSystem.ParticleSystem.setEmitter(self, self.emitter)

    def __update(self, state):
	"""update(self, state)"""
        dt = min(globalClock.getDt(), 0.1)
        physicsMgr.doPhysics(dt)
        particleMgr.doParticles(dt)
        return Task.cont

    def getNodePath(self):
	"""getNode(self)"""
	return self.nodePath

    def start(self):
	"""start(self)"""
	self.stop()
	taskMgr.spawnTaskNamed(Task.Task(self.__update), 'update-particles')

    def stop(self):
	"""stop(self)"""
	taskMgr.removeTasksNamed('update-particles')

    def printParams(self):
	"""printParams(self)"""
	print "# Factory parameters"
	print "factory.setLifespanBase(%f)" % self.factory.getLifespanBase()
	print "factory.setLifespanSpread(%f)" % self.factory.getLifespanSpread()
	print "factory.setMassBase(%f)" % self.factory.getMassBase()
	print "factory.setMassSpread(%f)" % self.factory.getMassSpread()
	print "factory.setTerminalVelocityBase(%f)" % self.factory.getTerminalVelocityBase()
	print "factory.setTerminalVelocitySpread(%f)" % self.factory.getTerminalVelocitySpread()
	if (self.factoryType == "PointParticleFactory"):
	    print "# Point factory parameters"
	elif (self.factoryType == "ZSpinParticleFactory"):
	    print "# Z Spin factory parameters"
	    print "factory.setInitialAngle(%f)" % self.factory.getInitialAngle()
	    print "factory.setFinalAngle(%f)" % self.factory.getFinalAngle()
	    print "factory.setInitialAngleSpread(%f)" % self.factory.getInitialAngleSpread()
	    print "factory.setFinalAngleSpread(%f)" % self.factory.getFinalAngleSpread()
	elif (self.factoryType == "OrientedParticleFactory"):
	    print "# Oriented factory parameters"
	    print 'factory.setInitialOrientation(' + self.factory.getInitialOrientation + ')'
	    print 'factory.setFinalOrientation(' + self.factory.getFinalOrientation + ')'

	print "# Renderer parameters"
	alphaMode = self.renderer.getAlphaMode()
	aMode = "PR_NOT_INITIALIZED_YET" 
	if (alphaMode == BaseParticleRenderer.BaseParticleRenderer.PRALPHANONE):
	    aMode = "PR_ALPHA_NONE"
	elif (alphaMode == 
		BaseParticleRenderer.BaseParticleRenderer.PRALPHAOUT):
	    aMode = "PR_ALPHA_OUT"
	elif (alphaMode == 
		BaseParticleRenderer.BaseParticleRenderer.PRALPHAIN):
	    aMode = "PR_ALPHA_IN"
	elif (alphaMode == 
	   	BaseParticleRenderer.BaseParticleRenderer.PRALPHAUSER):
	    aMode = "PR_ALPHA_USER"
	print 'renderer.setAlphaMode(BaseParticleRenderer.BaseParticleRenderer.' + aMode + ')'
	print 'renderer.setUserAlpha(%f)' % self.renderer.getUserAlpha()
	if (self.rendererType == "Point"):
	    print "# Point parameters"
	    print 'renderer.setPointSize(%f)' % self.renderer.getPointSize()
	    sColor = self.renderer.getStartColor()
	    print ('renderer.setStartColor(Colorf(%f, %f, %f, %f))' % (sColor[0], sColor[1], sColor[2], sColor[3])) 
	    sColor = self.renderer.getEndColor()
	    print ('renderer.setEndColor(Colorf(%f, %f, %f, %f))' % (sColor[0], sColor[1], sColor[2], sColor[3])) 
	    blendType = self.renderer.getBlendType()
	    bType = "unknown"
	    if (blendType == PointParticleRenderer.PointParticleRenderer.PPONECOLOR):
	    	bType = "PP_ONE_COLOR"
	    elif (blendType == PointParticleRenderer.PointParticleRenderer.PPBLENDLIFE):
		bType = "PP_BLEND_LIFE"	
	    elif (blendType == PointParticleRenderer.PointParticleRenderer.PPBLENDVEL):
		bType = "PP_BLEND_VEL"	
	    print 'renderer.setBlendType(PointParticleRenderer.PointParticleRenderer.' + bType + ')'
	    blendMethod = self.renderer.getBlendMethod()
	    bMethod = "PP_NO_BLEND"
	    if (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPNOBLEND):
		bMethod = "PP_NO_BLEND"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDLINEAR):
		bMethod = "PP_BLEND_LINEAR"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDCUBIC):
		bMethod = "PP_BLEND_CUBIC"
	    print 'renderer.setBlendMethod(BaseParticleRenderer.BaseParticleRenderer.' + bMethod + ')'
	elif (self.rendererType == "LineParticleRenderer"):
	    print "# Line parameters"
	    sColor = self.renderer.getHeadColor()
	    print ('renderer.setHeadColor(Colorf(%f, %f, %f, %f))' % (sColor[0], sColor[1], sColor[2], sColor[3]))
	    sColor = self.renderer.getTailColor()
	    print ('renderer.setTailColor(Colorf(%f, %f, %f, %f))' % (sColor[0], sColor[1], sColor[2], sColor[3]))
	elif (self.rendererType == "GeomParticleRenderer"):
	    print "# Geom parameters"
	    node = self.renderer.getGeomNode()
	    print 'renderer.setGeomNode(' + node.getName() + ')'
	elif (self.rendererType == "SparkleParticleRenderer"):
	    print "# Sparkle parameters"
	    sColor = self.renderer.getCenterColor()
	    print ('renderer.setCenterColor(Colorf(%f, %f, %f, %f))' % (sColor[0], sColor[1], sColor[2], sColor[3]))
	    sColor = self.renderer.getEdgeColor()
	    print ('renderer.setEdgeColor(Colorf(%f, %f, %f, %f))' % (sColor[0], sColor[1], sColor[2], sColor[3]))
	    print 'renderer.setBirthRadius(%f)' % self.renderer.getBirthRadius()
	    print 'renderer.setDeathRadius(%f)' % self.renderer.setDeathRadius()
	    lifeScale = self.renderer.getLifeScale()
	    lScale = "SP_NO_SCALE"
	    if (lifeScale == SparkleParticleRenderer.SparkleParticleRenderer.SPSCALE):
		lScale = "SP_SCALE"
	    print 'renderer.setLifeScale(SparkleParticleRenderer.SparkleParticleRenderer.' + lScale + ')'
	elif (self.rendererType == "SpriteParticleRenderer"):
	    print "# Sprite parameters"
	    tex = self.renderer.getTexture()
	    print 'renderer.setTexture(' + tex.getName() + ')'
	    sColor = self.renderer.getColor()
	    print ('renderer.setColor(Colorf(%f, %f, %f, %f))' % (sColor[0], sColor[1], sColor[2], sColor[3]))
	    print 'renderer.setXScaleFlag(%d)' % self.renderer.getXScaleFlag()
	    print 'renderer.setYScaleFlag(%d)' % self.renderer.getYScaleFlag()
	    print 'renderer.setAnimAngleFlag(%d)' % self.renderer.getAnimAngleFlag()
	    print 'renderer.setInitialXScale(%f)' % self.renderer.getInitialXScale()
	    print 'renderer.setFinalXScale(%f)' % self.renderer.getFinalXScale()
	    print 'renderer.setInitialYScale(%f)' % self.renderer.getInitialYScale()
	    print 'renderer.setFinalYScale(%f)' % self.renderer.getFinalYScale()
	    print 'renderer.setNonanimatedTheta(%f)' % self.renderer.getNonanimatedTheta()
	    blendMethod = self.renderer.getAlphaBlendMethod()
	    bMethod = "PP_NO_BLEND"
	    if (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPNOBLEND):
		bMethod = "PP_NO_BLEND"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDLINEAR):
		bMethod = "PP_BLEND_LINEAR"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDCUBIC):
		bMethod = "PP_BLEND_CUBIC"
	    print 'renderer.setAlphaBlendMethod(BaseParticleRenderer.BaseParticleRenderer.' + bMethod + ')'
	    print 'setAlphaDisable(%d)' % self.renderer.getAlphaDisable()

	print "# Emitter parameters"
	emissionType = self.emitter.getEmissionType()
	eType = "unknown"
	if (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETEXPLICIT):
	    eType = "ET_EXPLICIT"
	elif (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETRADIATE):
	    eType = "ET_RADIATE"
	elif (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETCUSTOM):
	    eType = "ET_CUSTOM"
	print 'emitter.setEmissionType(BaseParticleEmitter.BaseParticleEmitter.' + eType + ')'
	print 'emitter.setAmplitude(%f)' % self.emitter.getAmplitude()
	print 'emitter.setAmplitudeSpread(%f)' % self.emitter.getAmplitudeSpread()
	oForce = self.emitter.getOffsetForce()
	print ('emitter.setOffsetForce(Vec3(%f, %f, %f))' % (oForce[0], oForce[1], oForce[2]))
	oForce = self.emitter.getExplicitLaunchVector()
	print ('emitter.setExplicitLaunchVector(Vec3(%f, %f, %f))' % (oForce[0], oForce[1], oForce[2]))
	orig = self.emitter.getRadiateOrigin()
	print ('emitter.setRadiateOrigin(Point3(%f, %f, %f))' % (orig[0], orig[1], orig[2]))
	if (self.emitterType == "BoxEmitter"):
	    print "# Box parameters"
	    bound = self.emitter.getMinBound()
	    print ('emitter.setMinBound(Point3(%f, %f, %f))' % (bound[0], bound[1], bound[2]))
	    bound = self.emitter.getMaxBound()
	    print ('emitter.setMaxBound(Point3(%f, %f, %f))' % (bound[0], bound[1], bound[2]))
	elif (self.emitterType == "DiscEmitter"):
	    print "# Disc parameters"
	    print 'emitter.setRadius(%f)' % self.emitter.getRadius()
	    print 'emitter.setOuterAngle(%f)' % self.emitter.getOuterAngle()
	    print 'emitter.setInnerAngle(%f)' % self.emitter.getInnerAngle()
	    print 'emitter.setOuterMagnitude(%f)' % self.emitter.getOuterMagnitude()
	    print 'emitter.setInnerMagnitude(%f)' % self.emitter.getInnerMagnitude()
	    print 'emitter.setCubicLerping(%d)' % self.emitter.getCubicLerping()

	elif (self.emitterType == "LineEmitter"):
	    print "# Line parameters"
	    point = self.emitter.getEndpoint1()
	    print ('emitter.setEndpoint1(Point3(%f, %f, %f))' % (point[0], point[1], point[2]))
	    point = self.emitter.getEndpoint2()
	    print ('emitter.setEndpoint2(Point3(%f, %f, %f))' % (point[0], point[1], point[2]))
	elif (self.emitterType == "PointEmitter"):
	    print "# Point parameters"
	    point = self.emitter.getLocation()
	    print ('emitter.setLocation(Point3(%f, %f, %f))' % (point[0], point[1], point[2]))
	elif (self.emitterType == "RectangleEmitter"):
	    print "# Rectangle parameters"
	    bound = self.emitter.getMinBound()
	    print ('emitter.setMinBound(Point2(%f, %f))' % (point[0], point[1]))
	    bound = self.emitter.getMaxBound()
	    print ('emitter.setMaxBound(Point2(%f, %f))' % (point[0], point[1]))
	elif (self.emitterType == "RingEmitter"):
	    print "# Ring parameters"
	    print 'emitter.setRadius(%f)' % self.emitter.getRadius()
	    print 'emitter.setAngle(%f)' % self.emitter.getAngle()
	elif (self.emitterType == "SphereSurfaceEmitter"):
	    print "# Sphere Surface parameters"
	    print 'emitter.setRadius(%f)' % self.emitter.getRadius()
	elif (self.emitterType == "SphereVolumeEmitter"):
	    print "# Sphere Volume parameters"
	    print 'emitter.setRadius(%f)' % self.emitter.getRadius()
	elif (self.emitterType == "TangentRingEmitter"):
	    print "# Tangent Ring parameters"
	    print 'emitter.setRadius(%f)' % self.emitter.getRadius()
