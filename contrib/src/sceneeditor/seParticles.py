from panda3d.core import *
from panda3d.physics import *
from direct.particles.ParticleManagerGlobal import *
from direct.showbase.PhysicsManagerGlobal import *
#import OrientedParticleFactory
import string
import os
from direct.directnotify import DirectNotifyGlobal
import sys

class Particles(ParticleSystem):

    notify = DirectNotifyGlobal.directNotify.newCategory('Particles')
    id = 1

    def __init__(self, name=None, poolSize=1024):
        """__init__(name, poolSize)"""

        if (name == None):
            self.name = 'particles-%d' % Particles.id
            Particles.id += 1
        else:
            self.name = name
        ParticleSystem.ParticleSystem.__init__(self, poolSize)
        # self.setBirthRate(0.02)
        # self.setLitterSize(10)
        # self.setLitterSpread(0)

        # Set up a physical node
        self.node = PhysicalNode(self.name)
        self.nodePath = NodePath(self.node)
        self.setRenderParent(self.node)
        self.node.addPhysical(self)

        self.factory = None
        self.factoryType = "undefined"
        # self.setFactory("PointParticleFactory")
        self.renderer = None
        self.rendererType = "undefined"
        # self.setRenderer("PointParticleRenderer")
        self.emitter = None
        self.emitterType = "undefined"
        # self.setEmitter("SphereVolumeEmitter")

        # Enable particles by default
        self.fEnabled = 0
        #self.enable()

    def cleanup(self):
        self.disable()
        self.clearLinearForces()
        self.clearAngularForces()
        self.setRenderParent(self.node)
        self.node.removePhysical(self)
        self.nodePath.removeNode()
        del self.node
        del self.nodePath
        del self.factory
        del self.renderer
        del self.emitter

    def enable(self):
        """enable()"""
        if (self.fEnabled == 0):
            physicsMgr.attachPhysical(self)
            particleMgr.attachParticlesystem(self)
            self.fEnabled = 1

    def disable(self):
        """disable()"""
        if (self.fEnabled == 1):
            physicsMgr.removePhysical(self)
            particleMgr.removeParticlesystem(self)
            self.fEnabled = 0

    def isEnabled(self):
        return self.fEnabled

    def getNode(self):
        return self.node

    def setFactory(self, type):
        """setFactory(type)"""
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
            print("unknown factory type: %s" % type)
            return None
        self.factory.setLifespanBase(0.5)
        ParticleSystem.ParticleSystem.setFactory(self, self.factory)

    def setRenderer(self, type):
        """setRenderer(type)"""
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
            npath = NodePath('default-geom')
            # This was moved here because we do not want to download
            # the direct tools with toontown.
            from direct.directtools import DirectSelection
            bbox = DirectSelection.DirectBoundingBox(npath)
            self.renderer.setGeomNode(bbox.lines.node())
        elif (type == "SparkleParticleRenderer"):
            self.renderer = SparkleParticleRenderer.SparkleParticleRenderer()
        elif (type == "SpriteParticleRenderer"):
            self.renderer = SpriteParticleRenderer.SpriteParticleRenderer()
            if (self.renderer.getSourceType() ==
                SpriteParticleRenderer.SpriteParticleRenderer.STTexture):
                # Use current default texture
                # See sourceTextureName SpriteParticleRenderer-extensions.py
                self.renderer.setTextureFromFile()
            else:
                # Use current default model file and node
                # See sourceFileName and sourceNodeName in SpriteParticleRenderer-extensions.py
                self.renderer.setTextureFromNode()
        else:
            print("unknown renderer type: %s" % type)
            return None
        ParticleSystem.ParticleSystem.setRenderer(self, self.renderer)

    def setEmitter(self, type):
        """setEmitter(type)"""
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
            print("unknown emitter type: %s" % type)
            return None
        ParticleSystem.ParticleSystem.setEmitter(self, self.emitter)

    def addForce(self, force):
        """addForce(force)"""
        if (force.isLinear()):
            self.addLinearForce(force)
        else:
            self.addAngularForce(force)

    def removeForce(self, force):
        """removeForce(force)"""
        if (force == None):
            self.notify.warning('removeForce() - force == None!')
            return
        if (force.isLinear()):
            self.removeLinearForce(force)
        else:
            self.removeAngularForce(force)

    def setRenderNodePath(self, nodePath):
        self.setRenderParent(nodePath.node())

    ## Getters ##
    def getName(self):
        """getName()"""
        return self.name
    def getFactory(self):
        """getFactory()"""
        return self.factory
    def getEmitter(self):
        """getEmitter()"""
        return self.emitter
    def getRenderer(self):
        """getRenderer()"""
        return self.renderer

    def printParams(self, file = sys.stdout, targ = 'self'):
        """printParams(file, targ)"""
        i1="    "
        i2=i1+i1
        file.write(i2+'# Particles parameters\n')
        file.write(i2+targ + '.setFactory(\"' + self.factoryType + '\")\n')
        file.write(i2+targ + '.setRenderer(\"' + self.rendererType + '\")\n')
        file.write(i2+targ + '.setEmitter(\"' + self.emitterType + '\")\n')

        # System parameters
        file.write(i2+targ + ('.setPoolSize(%d)\n' %
                           int(self.getPoolSize())))
        file.write(i2+targ + ('.setBirthRate(%.4f)\n' %
                           self.getBirthRate()))
        file.write(i2+targ + ('.setLitterSize(%d)\n' %
                           int(self.getLitterSize())))
        file.write(i2+targ + ('.setLitterSpread(%d)\n' %
                           self.getLitterSpread()))
        file.write(i2+targ + ('.setSystemLifespan(%.4f)\n' %
                           self.getSystemLifespan()))
        file.write(i2+targ + ('.setLocalVelocityFlag(%d)\n' %
                           self.getLocalVelocityFlag()))
        file.write(i2+targ + ('.setSystemGrowsOlderFlag(%d)\n' %
                           self.getSystemGrowsOlderFlag()))
        file.write(i2+'# Factory parameters\n')
        file.write(i2+targ + ('.factory.setLifespanBase(%.4f)\n' %
                           self.factory.getLifespanBase()))
        file.write(i2+targ + '.factory.setLifespanSpread(%.4f)\n' % \
                                self.factory.getLifespanSpread())
        file.write(i2+targ + '.factory.setMassBase(%.4f)\n' % \
                                self.factory.getMassBase())
        file.write(i2+targ + '.factory.setMassSpread(%.4f)\n' % \
                                self.factory.getMassSpread())
        file.write(i2+targ + '.factory.setTerminalVelocityBase(%.4f)\n' % \
                                self.factory.getTerminalVelocityBase())
        file.write(i2+targ + '.factory.setTerminalVelocitySpread(%.4f)\n' % \
                                self.factory.getTerminalVelocitySpread())
        if (self.factoryType == "PointParticleFactory"):
            file.write(i2+'# Point factory parameters\n')
        elif (self.factoryType == "ZSpinParticleFactory"):
            file.write(i2+'# Z Spin factory parameters\n')
            file.write(i2+targ + '.factory.setInitialAngle(%.4f)\n' % \
                                        self.factory.getInitialAngle())
            file.write(i2+targ + '.factory.setInitialAngleSpread(%.4f)\n' % \
                                        self.factory.getInitialAngleSpread())
            file.write(i2+targ + '.factory.enableAngularVelocity(%d)\n' % \
                                        self.factory.getAngularVelocityEnabled())
            if(self.factory.getAngularVelocityEnabled()):
                file.write(i2+targ + '.factory.setAngularVelocity(%.4f)\n' % \
                                            self.factory.getAngularVelocity())
                file.write(i2+targ + '.factory.setAngularVelocitySpread(%.4f)\n' % \
                                            self.factory.getAngularVelocitySpread())
            else:
                file.write(i2+targ + '.factory.setFinalAngle(%.4f)\n' % \
                                            self.factory.getFinalAngle())
                file.write(i2+targ + '.factory.setFinalAngleSpread(%.4f)\n' % \
                                        self.factory.getFinalAngleSpread())

        elif (self.factoryType == "OrientedParticleFactory"):
            file.write(i2+'# Oriented factory parameters\n')
            file.write(i2+targ + '.factory.setInitialOrientation(%.4f)\n' % \
                                        self.factory.getInitialOrientation())
            file.write(i2+targ + '.factory.setFinalOrientation(%.4f)\n' % \
                                        self.factory.getFinalOrientation())

        file.write(i2+'# Renderer parameters\n')
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
        file.write(i2+targ + '.renderer.setAlphaMode(BaseParticleRenderer.' + aMode + ')\n')
        file.write(i2+targ + '.renderer.setUserAlpha(%.2f)\n' % \
                                        self.renderer.getUserAlpha())
        if (self.rendererType == "PointParticleRenderer"):
            file.write(i2+'# Point parameters\n')
            file.write(i2+targ + '.renderer.setPointSize(%.2f)\n' % \
                                        self.renderer.getPointSize())
            sColor = self.renderer.getStartColor()
            file.write(i2+(targ + '.renderer.setStartColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
            sColor = self.renderer.getEndColor()
            file.write(i2+(targ + '.renderer.setEndColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
            blendType = self.renderer.getBlendType()
            bType = "PPONECOLOR"
            if (blendType == PointParticleRenderer.PointParticleRenderer.PPONECOLOR):
                bType = "PPONECOLOR"
            elif (blendType == PointParticleRenderer.PointParticleRenderer.PPBLENDLIFE):
                bType = "PPBLENDLIFE"
            elif (blendType == PointParticleRenderer.PointParticleRenderer.PPBLENDVEL):
                bType = "PPBLENDVEL"
            file.write(i2+targ + '.renderer.setBlendType(PointParticleRenderer.' + bType + ')\n')
            blendMethod = self.renderer.getBlendMethod()
            bMethod = "PPNOBLEND"
            if (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPNOBLEND):
                bMethod = "PPNOBLEND"
            elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDLINEAR):
                bMethod = "PPBLENDLINEAR"
            elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDCUBIC):
                bMethod = "PPBLENDCUBIC"
            file.write(i2+targ + '.renderer.setBlendMethod(BaseParticleRenderer.' + bMethod + ')\n')
        elif (self.rendererType == "LineParticleRenderer"):
            file.write(i2+'# Line parameters\n')
            sColor = self.renderer.getHeadColor()
            file.write(i2+(targ + '.renderer.setHeadColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
            sColor = self.renderer.getTailColor()
            file.write(i2+(targ + '.renderer.setTailColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
        elif (self.rendererType == "GeomParticleRenderer"):
            file.write(i2+'# Geom parameters\n')
            node = self.renderer.getGeomNode()
            file.write(i2+targ + '.renderer.setGeomNode(' + node.getName() + ')\n')
        elif (self.rendererType == "SparkleParticleRenderer"):
            file.write(i2+'# Sparkle parameters\n')
            sColor = self.renderer.getCenterColor()
            file.write(i2+(targ + '.renderer.setCenterColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
            sColor = self.renderer.getEdgeColor()
            file.write(i2+(targ + '.renderer.setEdgeColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
            file.write(i2+targ + '.renderer.setBirthRadius(%.4f)\n' % self.renderer.getBirthRadius())
            file.write(i2+targ + '.renderer.setDeathRadius(%.4f)\n' % self.renderer.getDeathRadius())
            lifeScale = self.renderer.getLifeScale()
            lScale = "SPNOSCALE"
            if (lifeScale == SparkleParticleRenderer.SparkleParticleRenderer.SPSCALE):
                lScale = "SPSCALE"
            file.write(i2+targ + '.renderer.setLifeScale(SparkleParticleRenderer.' + lScale + ')\n')
        elif (self.rendererType == "SpriteParticleRenderer"):
            file.write(i2+'# Sprite parameters\n')
            if (self.renderer.getSourceType() ==
                SpriteParticleRenderer.SpriteParticleRenderer.STTexture):
                tex = self.renderer.getTexture()
                file.write(i2+targ + '.renderer.setTexture(loader.loadTexture(\'' + tex.getFilename().getFullpath() + '\'))\n')
            else:
                modelName = self.renderer.getSourceFileName()
                nodeName = self.renderer.getSourceNodeName()
                file.write(i2+targ + '.renderer.setTextureFromNode("%s", "%s")\n' % (modelName, nodeName))
            sColor = self.renderer.getColor()
            file.write(i2+(targ + '.renderer.setColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
            file.write(i2+targ + '.renderer.setXScaleFlag(%d)\n' % self.renderer.getXScaleFlag())
            file.write(i2+targ + '.renderer.setYScaleFlag(%d)\n' % self.renderer.getYScaleFlag())
            file.write(i2+targ + '.renderer.setAnimAngleFlag(%d)\n' % self.renderer.getAnimAngleFlag())
            file.write(i2+targ + '.renderer.setInitialXScale(%.4f)\n' % self.renderer.getInitialXScale())
            file.write(i2+targ + '.renderer.setFinalXScale(%.4f)\n' % self.renderer.getFinalXScale())
            file.write(i2+targ + '.renderer.setInitialYScale(%.4f)\n' % self.renderer.getInitialYScale())
            file.write(i2+targ + '.renderer.setFinalYScale(%.4f)\n' % self.renderer.getFinalYScale())
            file.write(i2+targ + '.renderer.setNonanimatedTheta(%.4f)\n' % self.renderer.getNonanimatedTheta())
            blendMethod = self.renderer.getAlphaBlendMethod()
            bMethod = "PPNOBLEND"
            if (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPNOBLEND):
                bMethod = "PPNOBLEND"
            elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDLINEAR):
                bMethod = "PPBLENDLINEAR"
            elif (blendMethod == BaseParticleRenderer.BaseParticleRenderer.PPBLENDCUBIC):
                bMethod = "PPBLENDCUBIC"
            file.write(i2+targ + '.renderer.setAlphaBlendMethod(BaseParticleRenderer.' + bMethod + ')\n')
            file.write(i2+targ + '.renderer.setAlphaDisable(%d)\n' % self.renderer.getAlphaDisable())

        file.write(i2+'# Emitter parameters\n')
        emissionType = self.emitter.getEmissionType()
        eType = "ETEXPLICIT"
        if (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETEXPLICIT):
            eType = "ETEXPLICIT"
        elif (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETRADIATE):
            eType = "ETRADIATE"
        elif (emissionType == BaseParticleEmitter.BaseParticleEmitter.ETCUSTOM):
            eType = "ETCUSTOM"
        file.write(i2+targ + '.emitter.setEmissionType(BaseParticleEmitter.' + eType + ')\n')
        file.write(i2+targ + '.emitter.setAmplitude(%.4f)\n' % self.emitter.getAmplitude())
        file.write(i2+targ + '.emitter.setAmplitudeSpread(%.4f)\n' % self.emitter.getAmplitudeSpread())
        oForce = self.emitter.getOffsetForce()
        file.write(i2+(targ + '.emitter.setOffsetForce(Vec3(%.4f, %.4f, %.4f))\n' % (oForce[0], oForce[1], oForce[2])))
        oForce = self.emitter.getExplicitLaunchVector()
        file.write(i2+(targ + '.emitter.setExplicitLaunchVector(Vec3(%.4f, %.4f, %.4f))\n' % (oForce[0], oForce[1], oForce[2])))
        orig = self.emitter.getRadiateOrigin()
        file.write(i2+(targ + '.emitter.setRadiateOrigin(Point3(%.4f, %.4f, %.4f))\n' % (orig[0], orig[1], orig[2])))
        if (self.emitterType == "BoxEmitter"):
            file.write(i2+'# Box parameters\n')
            bound = self.emitter.getMinBound()
            file.write(i2+(targ + '.emitter.setMinBound(Point3(%.4f, %.4f, %.4f))\n' % (bound[0], bound[1], bound[2])))
            bound = self.emitter.getMaxBound()
            file.write(i2+(targ + '.emitter.setMaxBound(Point3(%.4f, %.4f, %.4f))\n' % (bound[0], bound[1], bound[2])))
        elif (self.emitterType == "DiscEmitter"):
            file.write(i2+'# Disc parameters\n')
            file.write(i2+targ + '.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
            if (eType == "ETCUSTOM"):
                file.write(i2+targ + '.emitter.setOuterAngle(%.4f)\n' % self.emitter.getOuterAngle())
                file.write(i2+targ + '.emitter.setInnerAngle(%.4f)\n' % self.emitter.getInnerAngle())
                file.write(i2+targ + '.emitter.setOuterMagnitude(%.4f)\n' % self.emitter.getOuterMagnitude())
                file.write(i2+targ + '.emitter.setInnerMagnitude(%.4f)\n' % self.emitter.getInnerMagnitude())
                file.write(i2+targ + '.emitter.setCubicLerping(%d)\n' % self.emitter.getCubicLerping())

        elif (self.emitterType == "LineEmitter"):
            file.write(i2+'# Line parameters\n')
            point = self.emitter.getEndpoint1()
            file.write(i2+(targ + '.emitter.setEndpoint1(Point3(%.4f, %.4f, %.4f))\n' % (point[0], point[1], point[2])))
            point = self.emitter.getEndpoint2()
            file.write(i2+(targ + '.emitter.setEndpoint2(Point3(%.4f, %.4f, %.4f))\n' % (point[0], point[1], point[2])))
        elif (self.emitterType == "PointEmitter"):
            file.write(i2+'# Point parameters\n')
            point = self.emitter.getLocation()
            file.write(i2+(targ + '.emitter.setLocation(Point3(%.4f, %.4f, %.4f))\n' % (point[0], point[1], point[2])))
        elif (self.emitterType == "RectangleEmitter"):
            file.write(i2+'# Rectangle parameters\n')
            point = self.emitter.getMinBound()
            file.write(i2+(targ + '.emitter.setMinBound(Point2(%.4f, %.4f))\n' % (point[0], point[1])))
            point = self.emitter.getMaxBound()
            file.write(i2+(targ + '.emitter.setMaxBound(Point2(%.4f, %.4f))\n' % (point[0], point[1])))
        elif (self.emitterType == "RingEmitter"):
            file.write(i2+'# Ring parameters\n')
            file.write(i2+targ + '.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
            if (eType == "ETCUSTOM"):
                file.write(i2+targ + '.emitter.setAngle(%.4f)\n' % self.emitter.getAngle())
        elif (self.emitterType == "SphereSurfaceEmitter"):
            file.write(i2+'# Sphere Surface parameters\n')
            file.write(i2+targ + '.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
        elif (self.emitterType == "SphereVolumeEmitter"):
            file.write(i2+'# Sphere Volume parameters\n')
            file.write(i2+targ + '.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
        elif (self.emitterType == "TangentRingEmitter"):
            file.write(i2+'# Tangent Ring parameters\n')
            file.write(i2+targ + '.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
