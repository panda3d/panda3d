from PandaObject import *
from ParticleManagerGlobal import *
from PhysicsManagerGlobal import *

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
import string
import os

SparkleParticleRenderer.SparkleParticleRenderer.SPNOSCALE = 0
SparkleParticleRenderer.SparkleParticleRenderer.SPSCALE = 1

class Particles(ParticleSystem.ParticleSystem):

    particleNum = 1

    def __init__(self, name = None, poolSize = 1024):
	"""__init__(self, name, poolSize)"""

	if (name == None):
	    self.name = 'particles-%d' % particleNum
	    particleNum = particleNum + 1
	else:
	    self.name = name
	ParticleSystem.ParticleSystem.__init__(self, poolSize)
	self.setBirthRate(0.02)
	self.setLitterSize(10)
	self.setLitterSpread(0)

	# Set up a physical node
	self.node = PhysicalNode.PhysicalNode(self.name)
	self.nodePath = hidden.attachNewNode(self.node)
	self.setRenderParent(self.node)
	self.node.addPhysical(self)

	self.factory = None 
	self.factoryType = "undefined"
	self.setFactory("PointParticleFactory")
	self.renderer = None 
	self.rendererType = "undefined"
	self.setRenderer("PointParticleRenderer")
	self.emitter = None 
	self.emitterType = "undefined"
	self.setEmitter("SphereVolumeEmitter")

    def enable(self):
	"""enable(self)"""
	physicsMgr.attachPhysical(self.node)
	particleMgr.attachParticlesystem(self)

    def disable(self):
	"""disable(self)"""
	physicsMgr.removePhysical(self.node)
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

    def bakeConfig(self, filename):
	"""saveFileData(self, filename)"""
	#fname = Filename(filename)
	#fname.resolveFilename(getParticlePath())
	#fname.resolveFilename(getModelPath())
        f = open(filename.toOsSpecific(), 'wb')
        # Add a blank line
        f.write('\n')
        # Now output style details to file
	self.printParams(f)
        # Close the file
        f.close()

    def saveConfig(self, filename):
	"""saveFileData(self, filename)"""
	#fname = Filename(filename)
	#fname.resolveFilename(getParticlePath())
	#fname.resolveFilename(getModelPath())
        f = open(filename.toOsSpecific(), 'wb')
        # Add a blank line
        f.write('\n')
        # Now output style details to file
	self.printParams(f, 1)
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

    def printParams(self, file = sys.stdout, bakeflag = 0):
	"""printParams(self, file)"""
	targ = 'self'
	if (bakeflag == 0):
	    targ = 'p'
	    file.write('from Particles import *\n')
	    file.write('p = Particles(\'' + self.name + '\')\n')
	    file.write('self.enable()\n')
	file.write('# Particles parameters\n')
	file.write(targ + '.setFactory(\"' + self.factoryType + '\")\n')
	file.write(targ + '.setRenderer(\"' + self.rendererType + '\")\n')
	file.write(targ + '.setEmitter(\"' + self.emitterType + '\")\n')

	file.write('# Factory parameters\n')
	file.write(targ + '.factory.setLifespanBase(%.4f)\n' % self.factory.getLifespanBase())
	file.write(targ + '.factory.setLifespanSpread(%.4f)\n' % self.factory.getLifespanSpread())
	file.write(targ + '.factory.setMassBase(%.4f)\n' % self.factory.getMassBase())
	file.write(targ + '.factory.setMassSpread(%.4f)\n' % self.factory.getMassSpread())
	file.write(targ + '.factory.setTerminalVelocityBase(%.4f)\n' % self.factory.getTerminalVelocityBase())
	file.write(targ + '.factory.setTerminalVelocitySpread(%.4f)\n' % self.factory.getTerminalVelocitySpread())
	if (self.factoryType == "PointParticleFactory"):
	    file.write('# Point factory parameters\n')
	elif (self.factoryType == "ZSpinParticleFactory"):
	    file.write('# Z Spin factory parameters\n')
	    file.write(targ + '.factory.setInitialAngle(%.4f)\n' % self.factory.getInitialAngle())
	    file.write(targ + '.factory.setFinalAngle(%.4f)\n' % self.factory.getFinalAngle())
	    file.write(targ + '.factory.setInitialAngleSpread(%.4f)\n' % self.factory.getInitialAngleSpread())
	    file.write(targ + '.factory.setFinalAngleSpread(%.4f)\n' % self.factory.getFinalAngleSpread())
	elif (self.factoryType == "OrientedParticleFactory"):
	    file.write('# Oriented factory parameters\n')
	    file.write(targ + '.factory.setInitialOrientation(%.4f)\n' % self.factory.getInitialOrientation()) 
	    file.write(targ + '.factory.setFinalOrientation(%.4f)\n' % self.factory.getFinalOrientation())

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
	file.write(targ + '.renderer.setAlphaMode(BaseParticleRenderer.BaseParticleRenderer.' + aMode + ')\n')
	file.write(targ + '.renderer.setUserAlpha(%.2f)\n' % self.renderer.getUserAlpha())
	if (self.rendererType == "Point"):
	    file.write('# Point parameters\n')
	    file.write(targ + '.renderer.setPointSize(%.2f)\n' % self.renderer.getPointSize())
	    sColor = self.renderer.getStartColor()
	    file.write((targ + '.renderer.setStartColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3]))) 
	    sColor = self.renderer.getEndColor()
	    file.write((targ + '.renderer.setEndColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3]))) 
	    blendType = self.renderer.getBlendType()
	    bType = "PPONECOLOR"
	    if (blendType == PointParticleRenderer.PointParticleRenderer.PPONECOLOR):
	    	bType = "PPONECOLOR"
	    elif (blendType == PointParticleRenderer.PointParticleRenderer.PPBLENDLIFE):
		bType = "PPBLENDLIFE"	
	    elif (blendType == PointParticleRenderer.PointParticleRenderer.PPBLENDVEL):
		bType = "PPBLENDVEL"	
	    file.write(targ + '.renderer.setBlendType(PointParticleRenderer.PointParticleRenderer.' + bType + ')\n')
	    blendMethod = self.renderer.getBlendMethod()
	    bMethod = "PPNOBLEND"
	    if (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPNOBLEND):
		bMethod = "PPNOBLEND"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDLINEAR):
		bMethod = "PPBLENDLINEAR"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDCUBIC):
		bMethod = "PPBLENDCUBIC"
	    file.write(targ + '.renderer.setBlendMethod(BaseParticleRenderer.BaseParticleRenderer.' + bMethod + ')\n')
	elif (self.rendererType == "LineParticleRenderer"):
	    file.write('# Line parameters\n')
	    sColor = self.renderer.getHeadColor()
	    file.write((targ + '.renderer.setHeadColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	    sColor = self.renderer.getTailColor()
	    file.write((targ + '.renderer.setTailColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	elif (self.rendererType == "GeomParticleRenderer"):
	    file.write('# Geom parameters\n')
	    node = self.renderer.getGeomNode()
	    file.write(targ + '.renderer.setGeomNode(' + node.getName() + ')\n')
	elif (self.rendererType == "SparkleParticleRenderer"):
	    file.write('# Sparkle parameters\n')
	    sColor = self.renderer.getCenterColor()
	    file.write((targ + '.renderer.setCenterColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	    sColor = self.renderer.getEdgeColor()
	    file.write((targ + '.renderer.setEdgeColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	    file.write(targ + '.renderer.setBirthRadius(%.4f)\n' % self.renderer.getBirthRadius())
	    file.write(targ + '.renderer.setDeathRadius(%.4f)\n' % self.renderer.getDeathRadius())
	    lifeScale = self.renderer.getLifeScale()
	    lScale = "SPNOSCALE"
	    if (lifeScale == SparkleParticleRenderer.SparkleParticleRenderer.SPSCALE):
		lScale = "SPSCALE"
	    file.write(targ + '.renderer.setLifeScale(SparkleParticleRenderer.SparkleParticleRenderer.' + lScale + ')\n')
	elif (self.rendererType == "SpriteParticleRenderer"):
	    file.write('# Sprite parameters\n')
	    tex = self.renderer.getTexture()
	    file.write(targ + '.renderer.setTexture(loader.loadTexture(\'' + tex.getName() + '\'))\n')
	    sColor = self.renderer.getColor()
	    file.write((targ + '.renderer.setColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
	    file.write(targ + '.renderer.setXScaleFlag(%d)\n' % self.renderer.getXScaleFlag())
	    file.write(targ + '.renderer.setYScaleFlag(%d)\n' % self.renderer.getYScaleFlag())
	    file.write(targ + '.renderer.setAnimAngleFlag(%d)\n' % self.renderer.getAnimAngleFlag())
	    file.write(targ + '.renderer.setInitialXScale(%.4f)\n' % self.renderer.getInitialXScale())
	    file.write(targ + '.renderer.setFinalXScale(%.4f)\n' % self.renderer.getFinalXScale())
	    file.write(targ + '.renderer.setInitialYScale(%.4f)\n' % self.renderer.getInitialYScale())
	    file.write(targ + '.renderer.setFinalYScale(%.4f)\n' % self.renderer.getFinalYScale())
	    file.write(targ + '.renderer.setNonanimatedTheta(%.4f)\n' % self.renderer.getNonanimatedTheta())
	    blendMethod = self.renderer.getAlphaBlendMethod()
	    bMethod = "PPNOBLEND"
	    if (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPNOBLEND):
		bMethod = "PPNOBLEND"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDLINEAR):
		bMethod = "PPBLENDLINEAR"
	    elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDCUBIC):
		bMethod = "PPBLENDCUBIC"
	    file.write(targ + '.renderer.setAlphaBlendMethod(BaseParticleRenderer.BaseParticleRenderer.' + bMethod + ')\n')
	    file.write(targ + '.renderer.setAlphaDisable(%d)\n' % self.renderer.getAlphaDisable())

	file.write('# Emitter parameters\n')
	emissionType = self.emitter.getEmissionType()
	eType = "ETEXPLICIT"
	if (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETEXPLICIT):
	    eType = "ETEXPLICIT"
	elif (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETRADIATE):
	    eType = "ETRADIATE"
	elif (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETCUSTOM):
	    eType = "ETCUSTOM"
	file.write(targ + '.emitter.setEmissionType(BaseParticleEmitter.BaseParticleEmitter.' + eType + ')\n')
	file.write(targ + '.emitter.setAmplitude(%.4f)\n' % self.emitter.getAmplitude())
	file.write(targ + '.emitter.setAmplitudeSpread(%.4f)\n' % self.emitter.getAmplitudeSpread())
	oForce = self.emitter.getOffsetForce()
	file.write((targ + '.emitter.setOffsetForce(Vec3(%.4f, %.4f, %.4f))\n' % (oForce[0], oForce[1], oForce[2])))
	oForce = self.emitter.getExplicitLaunchVector()
	file.write((targ + '.emitter.setExplicitLaunchVector(Vec3(%.4f, %.4f, %.4f))\n' % (oForce[0], oForce[1], oForce[2])))
	orig = self.emitter.getRadiateOrigin()
	file.write((targ + '.emitter.setRadiateOrigin(Point3(%.4f, %.4f, %.4f))\n' % (orig[0], orig[1], orig[2])))
	if (self.emitterType == "BoxEmitter"):
	    file.write('# Box parameters\n')
	    bound = self.emitter.getMinBound()
	    file.write((targ + '.emitter.setMinBound(Point3(%.4f, %.4f, %.4f))\n' % (bound[0], bound[1], bound[2])))
	    bound = self.emitter.getMaxBound()
	    file.write((targ + '.emitter.setMaxBound(Point3(%.4f, %.4f, %.4f))\n' % (bound[0], bound[1], bound[2])))
	elif (self.emitterType == "DiscEmitter"):
	    file.write('# Disc parameters\n')
	    file.write(targ + '.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
	    if (eType == "ETCUSTOM"):
	    	file.write(targ + '.emitter.setOuterAngle(%.4f)\n' % self.emitter.getOuterAngle())
	    	file.write(targ + '.emitter.setInnerAngle(%.4f)\n' % self.emitter.getInnerAngle())
	    	file.write(targ + '.emitter.setOuterMagnitude(%.4f)\n' % self.emitter.getOuterMagnitude())
	    	file.write(targ + '.emitter.setInnerMagnitude(%.4f)\n' % self.emitter.getInnerMagnitude())
	    	file.write(targ + '.emitter.setCubicLerping(%d)\n' % self.emitter.getCubicLerping())

	elif (self.emitterType == "LineEmitter"):
	    file.write('# Line parameters\n')
	    point = self.emitter.getEndpoint1()
	    file.write((targ + '.emitter.setEndpoint1(Point3(%.4f, %.4f, %.4f))\n' % (point[0], point[1], point[2])))
	    point = self.emitter.getEndpoint2()
	    file.write((targ + '.emitter.setEndpoint2(Point3(%.4f, %.4f, %.4f))\n' % (point[0], point[1], point[2])))
	elif (self.emitterType == "PointEmitter"):
	    file.write('# Point parameters\n')
	    point = self.emitter.getLocation()
	    file.write((targ + '.emitter.setLocation(Point3(%.4f, %.4f, %.4f))\n' % (point[0], point[1], point[2])))
	elif (self.emitterType == "RectangleEmitter"):
	    file.write('# Rectangle parameters\n')
	    bound = self.emitter.getMinBound()
	    file.write((targ + '.emitter.setMinBound(Point2(%.4f, %.4f))\n' % (point[0], point[1])))
	    bound = self.emitter.getMaxBound()
	    file.write((targ + '.emitter.setMaxBound(Point2(%.4f, %.4f))\n' % (point[0], point[1])))
	elif (self.emitterType == "RingEmitter"):
	    file.write('# Ring parameters\n')
	    file.write(targ + '.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
	    if (eType == "ETCUSTOM"):
	    	file.write(targ + '.emitter.setAngle(%.4f)\n' % self.emitter.getAngle())
	elif (self.emitterType == "SphereSurfaceEmitter"):
	    file.write('# Sphere Surface parameters\n')
	    file.write(targ + '.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
	elif (self.emitterType == "SphereVolumeEmitter"):
	    file.write('# Sphere Volume parameters\n')
	    file.write(targ + '.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
	elif (self.emitterType == "TangentRingEmitter"):
	    file.write('# Tangent Ring parameters\n')
	    file.write(targ + '.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
