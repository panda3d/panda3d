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

    def saveFileData(self, filename):
	"""saveFileData(self, filename)"""
	fname = Filename(filename)
	fname.resolveFilename(getParticlePath())
	fname.resolveFilename(getModelPath())
        f = open(fname.toOsSpecific(), 'a')
        # Add a blank line
        f.write('\n')
        # Now output style details to file
	self.printParams(f)
        # Close the file
        f.close()

    def getFileData(self, filename):
	"""getFileData(self, filename)
        Open the specified file and strip out unwanted whitespace and
        empty lines.  Return file as list, one file line per element.
        """
        fname = Filename(filename)
	fname.resolveFilename(getParticlePath())
	fname.resolveFilename(getModelPath())
        f = open(fname.toOsSpecific(), 'r')
        rawData = f.readlines()
        f.close()
        styleData = []
        for line in rawData:
            l = string.strip(line)
            if l:
                styleData.append(l)
        return styleData

    def printParams(self, file = sys.stdout):
	"""printParams(self, file)"""
	file.write('# Particles parameters\n')
	file.write('import Particles\n')
	file.write('p = Particles()\n')
	file.write('p.setFactory(\"' + self.factoryType + '\")\n')
	file.write('p.setRenderer(\"' + self.rendererType + '\")\n')
	file.write('p.setEmitter(\"' + self.emitterType + '\")\n')

	file.write('# Factory parameters\n')
	file.write('p.factory.setLifespanBase(%.4f)\n' % self.factory.getLifespanBase())
	file.write('p.factory.setLifespanSpread(%.4f)\n' % self.factory.getLifespanSpread())
	file.write('p.factory.setMassBase(%.4f)\n' % self.factory.getMassBase())
	file.write('p.factory.setMassSpread(%.4f)\n' % self.factory.getMassSpread())
	file.write('p.factory.setTerminalVelocityBase(%.4f)\n' % self.factory.getTerminalVelocityBase())
	file.write('p.factory.setTerminalVelocitySpread(%.4f)\n' % self.factory.getTerminalVelocitySpread())
	if (self.factoryType == "PointParticleFactory"):
	    file.write('# Point factory parameters\n')
	elif (self.factoryType == "ZSpinParticleFactory"):
	    file.write('# Z Spin factory parameters\n')
	    file.write('p.factory.setInitialAngle(%.4f)\n' % self.factory.getInitialAngle())
	    file.write('p.factory.setFinalAngle(%.4f)\n' % self.factory.getFinalAngle())
	    file.write('p.factory.setInitialAngleSpread(%.4f)\n' % self.factory.getInitialAngleSpread())
	    file.write('p.factory.setFinalAngleSpread(%.4f)\n' % self.factory.getFinalAngleSpread())
	elif (self.factoryType == "OrientedParticleFactory"):
	    file.write('# Oriented factory parameters\n')
	    file.write('p.factory.setInitialOrientation(%.4f)\n' % self.factory.getInitialOrientation()) 
	    file.write('p.factory.setFinalOrientation(%.4f)\n' % self.factory.getFinalOrientation())

	file.write('# Renderer parameters\n')
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
	file.write('p.renderer.setAlphaMode(BaseParticleRenderer.BaseParticleRenderer.' + aMode + ')\n')
	file.write('p.renderer.setUserAlpha(%.2f)\n' % self.renderer.getUserAlpha())
	if (self.rendererType == "Point"):
	    file.write('# Point parameters\n')
	    file.write('p.renderer.setPointSize(%.2f)\n' % self.renderer.getPointSize())
	    sColor = self.renderer.getStartColor()
	    file.write(('p.renderer.setStartColor(Colorf(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3]))) 
	    sColor = self.renderer.getEndColor()
	    file.write(('p.renderer.setEndColor(Colorf(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3]))) 
	    blendType = self.renderer.getBlendType()
	    bType = "unknown"
	    if (blendType == PointParticleRenderer.PointParticleRenderer.PPONECOLOR):
	    	bType = "PP_ONE_COLOR"
	    elif (blendType == PointParticleRenderer.PointParticleRenderer.PPBLENDLIFE):
		bType = "PP_BLEND_LIFE"	
	    elif (blendType == PointParticleRenderer.PointParticleRenderer.PPBLENDVEL):
		bType = "PP_BLEND_VEL"	
	    file.write('p.renderer.setBlendType(PointParticleRenderer.PointParticleRenderer.' + bType + ')\n')
	    blendMethod = self.renderer.getBlendMethod()
	    bMethod = "PP_NO_BLEND"
	    if (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPNOBLEND):
		bMethod = "PP_NO_BLEND"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDLINEAR):
		bMethod = "PP_BLEND_LINEAR"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDCUBIC):
		bMethod = "PP_BLEND_CUBIC"
	    file.write('p.renderer.setBlendMethod(BaseParticleRenderer.BaseParticleRenderer.' + bMethod + ')\n')
	elif (self.rendererType == "LineParticleRenderer"):
	    file.write('# Line parameters\n')
	    sColor = self.renderer.getHeadColor()
	    file.write(('p.renderer.setHeadColor(Colorf(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	    sColor = self.renderer.getTailColor()
	    file.write(('p.renderer.setTailColor(Colorf(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	elif (self.rendererType == "GeomParticleRenderer"):
	    file.write('# Geom parameters\n')
	    node = self.renderer.getGeomNode()
	    file.write('p.renderer.setGeomNode(' + node.getName() + ')\n')
	elif (self.rendererType == "SparkleParticleRenderer"):
	    file.write('# Sparkle parameters\n')
	    sColor = self.renderer.getCenterColor()
	    file.write(('p.renderer.setCenterColor(Colorf(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	    sColor = self.renderer.getEdgeColor()
	    file.write(('p.renderer.setEdgeColor(Colorf(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	    file.write('p.renderer.setBirthRadius(%.4f)\n' % self.renderer.getBirthRadius())
	    file.write('p.renderer.setDeathRadius(%.4f)\n' % self.renderer.getDeathRadius())
	    lifeScale = self.renderer.getLifeScale()
	    lScale = "SP_NO_SCALE"
	    if (lifeScale == SparkleParticleRenderer.SparkleParticleRenderer.SPSCALE):
		lScale = "SP_SCALE"
	    file.write('p.renderer.setLifeScale(SparkleParticleRenderer.SparkleParticleRenderer.' + lScale + ')\n')
	elif (self.rendererType == "SpriteParticleRenderer"):
	    file.write('# Sprite parameters\n')
	    tex = self.renderer.getTexture()
	    file.write('p.renderer.setTexture(loader.loadTexture(\'' + tex.getName() + '\'))\n')
	    sColor = self.renderer.getColor()
	    file.write(('p.renderer.setColor(Colorf(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	    file.write('p.renderer.setXScaleFlag(%d)\n' % self.renderer.getXScaleFlag())
	    file.write('p.renderer.setYScaleFlag(%d)\n' % self.renderer.getYScaleFlag())
	    file.write('p.renderer.setAnimAngleFlag(%d)\n' % self.renderer.getAnimAngleFlag())
	    file.write('p.renderer.setInitialXScale(%.4f)\n' % self.renderer.getInitialXScale())
	    file.write('p.renderer.setFinalXScale(%.4f)\n' % self.renderer.getFinalXScale())
	    file.write('p.renderer.setInitialYScale(%.4f)\n' % self.renderer.getInitialYScale())
	    file.write('p.renderer.setFinalYScale(%.4f)\n' % self.renderer.getFinalYScale())
	    file.write('p.renderer.setNonanimatedTheta(%.4f)\n' % self.renderer.getNonanimatedTheta())
	    blendMethod = self.renderer.getAlphaBlendMethod()
	    bMethod = "PP_NO_BLEND"
	    if (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPNOBLEND):
		bMethod = "PP_NO_BLEND"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDLINEAR):
		bMethod = "PP_BLEND_LINEAR"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDCUBIC):
		bMethod = "PP_BLEND_CUBIC"
	    file.write('p.renderer.setAlphaBlendMethod(BaseParticleRenderer.BaseParticleRenderer.' + bMethod + ')\n')
	    file.write('p.renderer.setAlphaDisable(%d)\n' % self.renderer.getAlphaDisable())

	file.write('# Emitter parameters\n')
	emissionType = self.emitter.getEmissionType()
	eType = "unknown"
	if (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETEXPLICIT):
	    eType = "ET_EXPLICIT"
	elif (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETRADIATE):
	    eType = "ET_RADIATE"
	elif (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETCUSTOM):
	    eType = "ET_CUSTOM"
	file.write('p.emitter.setEmissionType(BaseParticleEmitter.BaseParticleEmitter.' + eType + ')\n')
	file.write('p.emitter.setAmplitude(%.4f)\n' % self.emitter.getAmplitude())
	file.write('p.emitter.setAmplitudeSpread(%.4f)\n' % self.emitter.getAmplitudeSpread())
	oForce = self.emitter.getOffsetForce()
	file.write(('p.emitter.setOffsetForce(Vec3(%.4f, %.4f, %.4f))\n' % (oForce[0], oForce[1], oForce[2])))
	oForce = self.emitter.getExplicitLaunchVector()
	file.write(('p.emitter.setExplicitLaunchVector(Vec3(%.4f, %.4f, %.4f))\n' % (oForce[0], oForce[1], oForce[2])))
	orig = self.emitter.getRadiateOrigin()
	file.write(('p.emitter.setRadiateOrigin(Point3(%.4f, %.4f, %.4f))\n' % (orig[0], orig[1], orig[2])))
	if (self.emitterType == "BoxEmitter"):
	    file.write('# Box parameters\n')
	    bound = self.emitter.getMinBound()
	    file.write(('p.emitter.setMinBound(Point3(%.4f, %.4f, %.4f))\n' % (bound[0], bound[1], bound[2])))
	    bound = self.emitter.getMaxBound()
	    file.write(('p.emitter.setMaxBound(Point3(%.4f, %.4f, %.4f))\n' % (bound[0], bound[1], bound[2])))
	elif (self.emitterType == "DiscEmitter"):
	    file.write('# Disc parameters\n')
	    file.write('p.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
	    if (eType == "ET_CUSTOM"):
	    	file.write('p.emitter.setOuterAngle(%.4f)\n' % self.emitter.getOuterAngle())
	    	file.write('p.emitter.setInnerAngle(%.4f)\n' % self.emitter.getInnerAngle())
	    	file.write('p.emitter.setOuterMagnitude(%.4f)\n' % self.emitter.getOuterMagnitude())
	    	file.write('p.emitter.setInnerMagnitude(%.4f)\n' % self.emitter.getInnerMagnitude())
	    	file.write('p.emitter.setCubicLerping(%d)\n' % self.emitter.getCubicLerping())

	elif (self.emitterType == "LineEmitter"):
	    file.write('# Line parameters\n')
	    point = self.emitter.getEndpoint1()
	    file.write(('p.emitter.setEndpoint1(Point3(%.4f, %.4f, %.4f))\n' % (point[0], point[1], point[2])))
	    point = self.emitter.getEndpoint2()
	    file.write(('p.emitter.setEndpoint2(Point3(%.4f, %.4f, %.4f))\n' % (point[0], point[1], point[2])))
	elif (self.emitterType == "PointEmitter"):
	    file.write('# Point parameters\n')
	    point = self.emitter.getLocation()
	    file.write(('p.emitter.setLocation(Point3(%.4f, %.4f, %.4f))\n' % (point[0], point[1], point[2])))
	elif (self.emitterType == "RectangleEmitter"):
	    file.write('# Rectangle parameters\n')
	    bound = self.emitter.getMinBound()
	    file.write(('p.emitter.setMinBound(Point2(%.4f, %.4f))\n' % (point[0], point[1])))
	    bound = self.emitter.getMaxBound()
	    file.write(('p.emitter.setMaxBound(Point2(%.4f, %.4f))\n' % (point[0], point[1])))
	elif (self.emitterType == "RingEmitter"):
	    file.write('# Ring parameters\n')
	    file.write('p.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
	    if (eType == "ET_CUSTOM"):
	    	file.write('p.emitter.setAngle(%.4f)\n' % self.emitter.getAngle())
	elif (self.emitterType == "SphereSurfaceEmitter"):
	    file.write('# Sphere Surface parameters\n')
	    file.write('p.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
	elif (self.emitterType == "SphereVolumeEmitter"):
	    file.write('# Sphere Volume parameters\n')
	    file.write('p.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
	elif (self.emitterType == "TangentRingEmitter"):
	    file.write('# Tangent Ring parameters\n')
	    file.write('p.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
