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
import string
import os

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
	elif (type == "GeomParticleRenderer"):
	    self.renderer = GeomParticleRenderer.GeomParticleRenderer()
	elif (type == "SparkleParticleRenderer"):
	    self.renderer = SparkleParticleRenderer.SparkleParticleRenderer()
	elif (type == "SpriteParticleRenderer"):
	    self.renderer = SpriteParticleRenderer.SpriteParticleRenderer()
	    t = loader.loadTexture('phase_3/maps/eyes.jpg')
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

    def saveConfig(self, filename):
	"""saveFileData(self, filename)"""
	#fname = Filename(filename)
	#fname.resolveFilename(getParticlePath())
	#fname.resolveFilename(getModelPath())
        f = open(filename.toOsSpecific(), 'ab')
        # Add a blank line
        f.write('\n')
        # Now output style details to file
	self.printParams(f)
        # Close the file
        f.close()

    def loadConfig(self, filename):
	"""getFileData(self, filename)
        Open the specified file and strip out unwanted whitespace and
        empty lines.  Return file as list, one file line per element.
        """
        #fname = Filename(filename)
	#fname.resolveFilename(getParticlePath())
	#fname.resolveFilename(getModelPath())
	execfile(filename.toOsSpecific())

    def printParams(self, file = sys.stdout):
	"""printParams(self, file)"""
	file.write('# Particles parameters\n')
	file.write('self.setFactory(\"' + self.factoryType + '\")\n')
	file.write('self.setRenderer(\"' + self.rendererType + '\")\n')
	file.write('self.setEmitter(\"' + self.emitterType + '\")\n')

	file.write('# Factory parameters\n')
	file.write('self.factory.setLifespanBase(%.4f)\n' % self.factory.getLifespanBase())
	file.write('self.factory.setLifespanSpread(%.4f)\n' % self.factory.getLifespanSpread())
	file.write('self.factory.setMassBase(%.4f)\n' % self.factory.getMassBase())
	file.write('self.factory.setMassSpread(%.4f)\n' % self.factory.getMassSpread())
	file.write('self.factory.setTerminalVelocityBase(%.4f)\n' % self.factory.getTerminalVelocityBase())
	file.write('self.factory.setTerminalVelocitySpread(%.4f)\n' % self.factory.getTerminalVelocitySpread())
	if (self.factoryType == "PointParticleFactory"):
	    file.write('# Point factory parameters\n')
	elif (self.factoryType == "ZSpinParticleFactory"):
	    file.write('# Z Spin factory parameters\n')
	    file.write('self.factory.setInitialAngle(%.4f)\n' % self.factory.getInitialAngle())
	    file.write('self.factory.setFinalAngle(%.4f)\n' % self.factory.getFinalAngle())
	    file.write('self.factory.setInitialAngleSpread(%.4f)\n' % self.factory.getInitialAngleSpread())
	    file.write('self.factory.setFinalAngleSpread(%.4f)\n' % self.factory.getFinalAngleSpread())
	elif (self.factoryType == "OrientedParticleFactory"):
	    file.write('# Oriented factory parameters\n')
	    file.write('self.factory.setInitialOrientation(%.4f)\n' % self.factory.getInitialOrientation()) 
	    file.write('self.factory.setFinalOrientation(%.4f)\n' % self.factory.getFinalOrientation())

	file.write('# Renderer parameters\n')
	alphaMode = self.renderer.getAlphaMode()
	aMode = "PRALPHANONE" 
	if (alphaMode == BaseParticleRenderer.BaseParticleRenderer.PRALPHANONE):
	    aMode = "PRALPHANONE"
	elif (alphaMode == 
		BaseParticleRenderer.BaseParticleRenderer.PRALPHAOUT):
	    aMode = "PRALPHAOUT"
	elif (alphaMode == 
		BaseParticleRenderer.BaseParticleRenderer.PRALPHAIN):
	    aMode = "PRALPHAIN"
	elif (alphaMode == 
	   	BaseParticleRenderer.BaseParticleRenderer.PRALPHAUSER):
	    aMode = "PRALPHAUSER"
	file.write('self.renderer.setAlphaMode(BaseParticleRenderer.BaseParticleRenderer.' + aMode + ')\n')
	file.write('self.renderer.setUserAlpha(%.2f)\n' % self.renderer.getUserAlpha())
	if (self.rendererType == "Point"):
	    file.write('# Point parameters\n')
	    file.write('self.renderer.setPointSize(%.2f)\n' % self.renderer.getPointSize())
	    sColor = self.renderer.getStartColor()
	    file.write(('self.renderer.setStartColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3]))) 
	    sColor = self.renderer.getEndColor()
	    file.write(('self.renderer.setEndColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3]))) 
	    blendType = self.renderer.getBlendType()
	    bType = "PPONECOLOR"
	    if (blendType == PointParticleRenderer.PointParticleRenderer.PPONECOLOR):
	    	bType = "PPONECOLOR"
	    elif (blendType == PointParticleRenderer.PointParticleRenderer.PPBLENDLIFE):
		bType = "PPBLENDLIFE"	
	    elif (blendType == PointParticleRenderer.PointParticleRenderer.PPBLENDVEL):
		bType = "PPBLENDVEL"	
	    file.write('self.renderer.setBlendType(PointParticleRenderer.PointParticleRenderer.' + bType + ')\n')
	    blendMethod = self.renderer.getBlendMethod()
	    bMethod = "PPNOBLEND"
	    if (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPNOBLEND):
		bMethod = "PPNOBLEND"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDLINEAR):
		bMethod = "PPBLENDLINEAR"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDCUBIC):
		bMethod = "PPBLENDCUBIC"
	    file.write('self.renderer.setBlendMethod(BaseParticleRenderer.BaseParticleRenderer.' + bMethod + ')\n')
	elif (self.rendererType == "LineParticleRenderer"):
	    file.write('# Line parameters\n')
	    sColor = self.renderer.getHeadColor()
	    file.write(('self.renderer.setHeadColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	    sColor = self.renderer.getTailColor()
	    file.write(('self.renderer.setTailColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	elif (self.rendererType == "GeomParticleRenderer"):
	    file.write('# Geom parameters\n')
	    node = self.renderer.getGeomNode()
	    file.write('self.renderer.setGeomNode(' + node.getName() + ')\n')
	elif (self.rendererType == "SparkleParticleRenderer"):
	    file.write('# Sparkle parameters\n')
	    sColor = self.renderer.getCenterColor()
	    file.write(('self.renderer.setCenterColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	    sColor = self.renderer.getEdgeColor()
	    file.write(('self.renderer.setEdgeColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	    file.write('self.renderer.setBirthRadius(%.4f)\n' % self.renderer.getBirthRadius())
	    file.write('self.renderer.setDeathRadius(%.4f)\n' % self.renderer.getDeathRadius())
	    lifeScale = self.renderer.getLifeScale()
	    lScale = "SPNOSCALE"
	    if (lifeScale == SparkleParticleRenderer.SparkleParticleRenderer.SPSCALE):
		lScale = "SPSCALE"
	    file.write('self.renderer.setLifeScale(SparkleParticleRenderer.SparkleParticleRenderer.' + lScale + ')\n')
	elif (self.rendererType == "SpriteParticleRenderer"):
	    file.write('# Sprite parameters\n')
	    tex = self.renderer.getTexture()
	    file.write('self.renderer.setTexture(loader.loadTexture(\'' + tex.getName() + '\'))\n')
	    sColor = self.renderer.getColor()
	    file.write(('self.renderer.setColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	    file.write('self.renderer.setXScaleFlag(%d)\n' % self.renderer.getXScaleFlag())
	    file.write('self.renderer.setYScaleFlag(%d)\n' % self.renderer.getYScaleFlag())
	    file.write('self.renderer.setAnimAngleFlag(%d)\n' % self.renderer.getAnimAngleFlag())
	    file.write('self.renderer.setInitialXScale(%.4f)\n' % self.renderer.getInitialXScale())
	    file.write('self.renderer.setFinalXScale(%.4f)\n' % self.renderer.getFinalXScale())
	    file.write('self.renderer.setInitialYScale(%.4f)\n' % self.renderer.getInitialYScale())
	    file.write('self.renderer.setFinalYScale(%.4f)\n' % self.renderer.getFinalYScale())
	    file.write('self.renderer.setNonanimatedTheta(%.4f)\n' % self.renderer.getNonanimatedTheta())
	    blendMethod = self.renderer.getAlphaBlendMethod()
	    bMethod = "PPNOBLEND"
	    if (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPNOBLEND):
		bMethod = "PPNOBLEND"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDLINEAR):
		bMethod = "PPBLENDLINEAR"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDCUBIC):
		bMethod = "PPBLENDCUBIC"
	    file.write('self.renderer.setAlphaBlendMethod(BaseParticleRenderer.BaseParticleRenderer.' + bMethod + ')\n')
	    file.write('self.renderer.setAlphaDisable(%d)\n' % self.renderer.getAlphaDisable())

	file.write('# Emitter parameters\n')
	emissionType = self.emitter.getEmissionType()
	eType = "ETEXPLICIT"
	if (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETEXPLICIT):
	    eType = "ETEXPLICIT"
	elif (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETRADIATE):
	    eType = "ETRADIATE"
	elif (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETCUSTOM):
	    eType = "ETCUSTOM"
	file.write('self.emitter.setEmissionType(BaseParticleEmitter.BaseParticleEmitter.' + eType + ')\n')
	file.write('self.emitter.setAmplitude(%.4f)\n' % self.emitter.getAmplitude())
	file.write('self.emitter.setAmplitudeSpread(%.4f)\n' % self.emitter.getAmplitudeSpread())
	oForce = self.emitter.getOffsetForce()
	file.write(('self.emitter.setOffsetForce(Vec3(%.4f, %.4f, %.4f))\n' % (oForce[0], oForce[1], oForce[2])))
	oForce = self.emitter.getExplicitLaunchVector()
	file.write(('self.emitter.setExplicitLaunchVector(Vec3(%.4f, %.4f, %.4f))\n' % (oForce[0], oForce[1], oForce[2])))
	orig = self.emitter.getRadiateOrigin()
	file.write(('self.emitter.setRadiateOrigin(Point3(%.4f, %.4f, %.4f))\n' % (orig[0], orig[1], orig[2])))
	if (self.emitterType == "BoxEmitter"):
	    file.write('# Box parameters\n')
	    bound = self.emitter.getMinBound()
	    file.write(('self.emitter.setMinBound(Point3(%.4f, %.4f, %.4f))\n' % (bound[0], bound[1], bound[2])))
	    bound = self.emitter.getMaxBound()
	    file.write(('self.emitter.setMaxBound(Point3(%.4f, %.4f, %.4f))\n' % (bound[0], bound[1], bound[2])))
	elif (self.emitterType == "DiscEmitter"):
	    file.write('# Disc parameters\n')
	    file.write('self.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
	    if (eType == "ETCUSTOM"):
	    	file.write('self.emitter.setOuterAngle(%.4f)\n' % self.emitter.getOuterAngle())
	    	file.write('self.emitter.setInnerAngle(%.4f)\n' % self.emitter.getInnerAngle())
	    	file.write('self.emitter.setOuterMagnitude(%.4f)\n' % self.emitter.getOuterMagnitude())
	    	file.write('self.emitter.setInnerMagnitude(%.4f)\n' % self.emitter.getInnerMagnitude())
	    	file.write('self.emitter.setCubicLerping(%d)\n' % self.emitter.getCubicLerping())

	elif (self.emitterType == "LineEmitter"):
	    file.write('# Line parameters\n')
	    point = self.emitter.getEndpoint1()
	    file.write(('self.emitter.setEndpoint1(Point3(%.4f, %.4f, %.4f))\n' % (point[0], point[1], point[2])))
	    point = self.emitter.getEndpoint2()
	    file.write(('self.emitter.setEndpoint2(Point3(%.4f, %.4f, %.4f))\n' % (point[0], point[1], point[2])))
	elif (self.emitterType == "PointEmitter"):
	    file.write('# Point parameters\n')
	    point = self.emitter.getLocation()
	    file.write(('self.emitter.setLocation(Point3(%.4f, %.4f, %.4f))\n' % (point[0], point[1], point[2])))
	elif (self.emitterType == "RectangleEmitter"):
	    file.write('# Rectangle parameters\n')
	    bound = self.emitter.getMinBound()
	    file.write(('self.emitter.setMinBound(Point2(%.4f, %.4f))\n' % (point[0], point[1])))
	    bound = self.emitter.getMaxBound()
	    file.write(('self.emitter.setMaxBound(Point2(%.4f, %.4f))\n' % (point[0], point[1])))
	elif (self.emitterType == "RingEmitter"):
	    file.write('# Ring parameters\n')
	    file.write('self.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
	    if (eType == "ETCUSTOM"):
	    	file.write('self.emitter.setAngle(%.4f)\n' % self.emitter.getAngle())
	elif (self.emitterType == "SphereSurfaceEmitter"):
	    file.write('# Sphere Surface parameters\n')
	    file.write('self.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
	elif (self.emitterType == "SphereVolumeEmitter"):
	    file.write('# Sphere Volume parameters\n')
	    file.write('self.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
	elif (self.emitterType == "TangentRingEmitter"):
	    file.write('# Tangent Ring parameters\n')
	    file.write('self.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
