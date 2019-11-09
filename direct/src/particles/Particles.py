"""The Python specialization of the particle system.

See the :ref:`particle-effects` section in the manual for an explanation
of the particle system.
"""

from panda3d.core import *

from panda3d.physics import PhysicalNode
from panda3d.physics import ParticleSystem
from panda3d.physics import PointParticleFactory
from panda3d.physics import ZSpinParticleFactory
#from panda3d.physics import OrientedParticleFactory
from panda3d.physics import BaseParticleRenderer
from panda3d.physics import PointParticleRenderer
from panda3d.physics import LineParticleRenderer
from panda3d.physics import GeomParticleRenderer
from panda3d.physics import SparkleParticleRenderer
#from panda3d.physics import SpriteParticleRenderer
from panda3d.physics import BaseParticleEmitter
from panda3d.physics import ArcEmitter
from panda3d.physics import BoxEmitter
from panda3d.physics import DiscEmitter
from panda3d.physics import LineEmitter
from panda3d.physics import PointEmitter
from panda3d.physics import RectangleEmitter
from panda3d.physics import RingEmitter
from panda3d.physics import SphereSurfaceEmitter
from panda3d.physics import SphereVolumeEmitter
from panda3d.physics import TangentRingEmitter
from panda3d.physics import SpriteAnim

from . import SpriteParticleRendererExt

from direct.directnotify.DirectNotifyGlobal import directNotify
import sys


class Particles(ParticleSystem):
    notify = directNotify.newCategory('Particles')
    id = 1

    def __init__(self, name=None, poolSize=1024):
        if (name == None):
            self.name = 'particles-%d' % Particles.id
            Particles.id += 1
        else:
            self.name = name
        ParticleSystem.__init__(self, poolSize)
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
        self.geomReference = ""

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
        if (self.fEnabled == 0):
            base.physicsMgr.attachPhysical(self)
            base.particleMgr.attachParticlesystem(self)
            self.fEnabled = 1

    def disable(self):
        if (self.fEnabled == 1):
            base.physicsMgr.removePhysical(self)
            base.particleMgr.removeParticlesystem(self)
            self.fEnabled = 0

    def isEnabled(self):
        return self.fEnabled

    def getNode(self):
        return self.node

    def setFactory(self, type):
        if (self.factoryType == type):
            return None
        if (self.factory):
            self.factory = None
        self.factoryType = type
        if (type == "PointParticleFactory"):
            self.factory = PointParticleFactory()
        elif (type == "ZSpinParticleFactory"):
            self.factory = ZSpinParticleFactory()
        elif (type == "OrientedParticleFactory"):
            self.factory = OrientedParticleFactory()
        else:
            print("unknown factory type: %s" % type)
            return None
        self.factory.setLifespanBase(0.5)
        ParticleSystem.setFactory(self, self.factory)

    def setRenderer(self, type):
        if (self.rendererType == type):
            return None
        if (self.renderer):
            self.renderer = None
        self.rendererType = type
        if (type == "PointParticleRenderer"):
            self.renderer = PointParticleRenderer()
            self.renderer.setPointSize(1.0)
        elif (type == "LineParticleRenderer"):
            self.renderer = LineParticleRenderer()
        elif (type == "GeomParticleRenderer"):
            self.renderer = GeomParticleRenderer()
            # This was moved here because we do not want to download
            # the direct tools with toontown.
            if __dev__:
                from direct.directtools import DirectSelection
                npath = NodePath('default-geom')
                bbox = DirectSelection.DirectBoundingBox(npath)
                self.renderer.setGeomNode(bbox.lines.node())
        elif (type == "SparkleParticleRenderer"):
            self.renderer = SparkleParticleRenderer()
        elif (type == "SpriteParticleRenderer"):
            self.renderer = SpriteParticleRendererExt.SpriteParticleRendererExt()
            # self.renderer.setTextureFromFile()
        else:
            print("unknown renderer type: %s" % type)
            return None
        ParticleSystem.setRenderer(self, self.renderer)

    def setEmitter(self, type):
        if (self.emitterType == type):
            return None
        if (self.emitter):
            self.emitter = None
        self.emitterType = type
        if (type == "ArcEmitter"):
            self.emitter = ArcEmitter()
        elif (type == "BoxEmitter"):
            self.emitter = BoxEmitter()
        elif (type == "DiscEmitter"):
            self.emitter = DiscEmitter()
        elif (type == "LineEmitter"):
            self.emitter = LineEmitter()
        elif (type == "PointEmitter"):
            self.emitter = PointEmitter()
        elif (type == "RectangleEmitter"):
            self.emitter = RectangleEmitter()
        elif (type == "RingEmitter"):
            self.emitter = RingEmitter()
        elif (type == "SphereSurfaceEmitter"):
            self.emitter = SphereSurfaceEmitter()
        elif (type == "SphereVolumeEmitter"):
            self.emitter = SphereVolumeEmitter()
            self.emitter.setRadius(1.0)
        elif (type == "TangentRingEmitter"):
            self.emitter = TangentRingEmitter()
        else:
            print("unknown emitter type: %s" % type)
            return None
        ParticleSystem.setEmitter(self, self.emitter)

    def addForce(self, force):
        if (force.isLinear()):
            self.addLinearForce(force)
        else:
            self.addAngularForce(force)

    def removeForce(self, force):
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
        return self.name

    def getFactory(self):
        return self.factory

    def getEmitter(self):
        return self.emitter

    def getRenderer(self):
        return self.renderer

    def printParams(self, file = sys.stdout, targ = 'self'):
        file.write('# Particles parameters\n')
        file.write(targ + '.setFactory(\"' + self.factoryType + '\")\n')
        file.write(targ + '.setRenderer(\"' + self.rendererType + '\")\n')
        file.write(targ + '.setEmitter(\"' + self.emitterType + '\")\n')

        # System parameters
        file.write(targ + ('.setPoolSize(%d)\n' %
                           int(self.getPoolSize())))
        file.write(targ + ('.setBirthRate(%.4f)\n' %
                           self.getBirthRate()))
        file.write(targ + ('.setLitterSize(%d)\n' %
                           int(self.getLitterSize())))
        file.write(targ + ('.setLitterSpread(%d)\n' %
                           self.getLitterSpread()))
        file.write(targ + ('.setSystemLifespan(%.4f)\n' %
                           self.getSystemLifespan()))
        file.write(targ + ('.setLocalVelocityFlag(%d)\n' %
                           self.getLocalVelocityFlag()))
        file.write(targ + ('.setSystemGrowsOlderFlag(%d)\n' %
                           self.getSystemGrowsOlderFlag()))
        file.write('# Factory parameters\n')
        file.write(targ + ('.factory.setLifespanBase(%.4f)\n' %
                           self.factory.getLifespanBase()))
        file.write(targ + '.factory.setLifespanSpread(%.4f)\n' % \
                                self.factory.getLifespanSpread())
        file.write(targ + '.factory.setMassBase(%.4f)\n' % \
                                self.factory.getMassBase())
        file.write(targ + '.factory.setMassSpread(%.4f)\n' % \
                                self.factory.getMassSpread())
        file.write(targ + '.factory.setTerminalVelocityBase(%.4f)\n' % \
                                self.factory.getTerminalVelocityBase())
        file.write(targ + '.factory.setTerminalVelocitySpread(%.4f)\n' % \
                                self.factory.getTerminalVelocitySpread())
        if (self.factoryType == "PointParticleFactory"):
            file.write('# Point factory parameters\n')
        elif (self.factoryType == "ZSpinParticleFactory"):
            file.write('# Z Spin factory parameters\n')
            file.write(targ + '.factory.setInitialAngle(%.4f)\n' % \
                                        self.factory.getInitialAngle())
            file.write(targ + '.factory.setInitialAngleSpread(%.4f)\n' % \
                                        self.factory.getInitialAngleSpread())
            file.write(targ + '.factory.enableAngularVelocity(%d)\n' % \
                                        self.factory.getAngularVelocityEnabled())
            if(self.factory.getAngularVelocityEnabled()):
                file.write(targ + '.factory.setAngularVelocity(%.4f)\n' % \
                                            self.factory.getAngularVelocity())
                file.write(targ + '.factory.setAngularVelocitySpread(%.4f)\n' % \
                                            self.factory.getAngularVelocitySpread())
            else:
                file.write(targ + '.factory.setFinalAngle(%.4f)\n' % \
                                            self.factory.getFinalAngle())
                file.write(targ + '.factory.setFinalAngleSpread(%.4f)\n' % \
                                        self.factory.getFinalAngleSpread())

        elif (self.factoryType == "OrientedParticleFactory"):
            file.write('# Oriented factory parameters\n')
            file.write(targ + '.factory.setInitialOrientation(%.4f)\n' % \
                                        self.factory.getInitialOrientation())
            file.write(targ + '.factory.setFinalOrientation(%.4f)\n' % \
                                        self.factory.getFinalOrientation())

        file.write('# Renderer parameters\n')
        alphaMode = self.renderer.getAlphaMode()
        aMode = "PRALPHANONE"
        if (alphaMode == BaseParticleRenderer.PRALPHANONE):
            aMode = "PRALPHANONE"
        elif (alphaMode == BaseParticleRenderer.PRALPHAOUT):
            aMode = "PRALPHAOUT"
        elif (alphaMode == BaseParticleRenderer.PRALPHAIN):
            aMode = "PRALPHAIN"
        elif (alphaMode == BaseParticleRenderer.PRALPHAINOUT):
            aMode = "PRALPHAINOUT"
        elif (alphaMode == BaseParticleRenderer.PRALPHAUSER):
            aMode = "PRALPHAUSER"
        file.write(targ + '.renderer.setAlphaMode(BaseParticleRenderer.' + aMode + ')\n')
        file.write(targ + '.renderer.setUserAlpha(%.2f)\n' % \
                                        self.renderer.getUserAlpha())
        if (self.rendererType == "PointParticleRenderer"):
            file.write('# Point parameters\n')
            file.write(targ + '.renderer.setPointSize(%.2f)\n' % \
                                        self.renderer.getPointSize())
            sColor = self.renderer.getStartColor()
            file.write((targ + '.renderer.setStartColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
            sColor = self.renderer.getEndColor()
            file.write((targ + '.renderer.setEndColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
            blendType = self.renderer.getBlendType()
            bType = "PPONECOLOR"
            if (blendType == PointParticleRenderer.PPONECOLOR):
                bType = "PPONECOLOR"
            elif (blendType == PointParticleRenderer.PPBLENDLIFE):
                bType = "PPBLENDLIFE"
            elif (blendType == PointParticleRenderer.PPBLENDVEL):
                bType = "PPBLENDVEL"
            file.write(targ + '.renderer.setBlendType(PointParticleRenderer.' + bType + ')\n')
            blendMethod = self.renderer.getBlendMethod()
            bMethod = "PPNOBLEND"
            if (blendMethod == BaseParticleRenderer.PPNOBLEND):
                bMethod = "PPNOBLEND"
            elif (blendMethod == BaseParticleRenderer.PPBLENDLINEAR):
                bMethod = "PPBLENDLINEAR"
            elif (blendMethod == BaseParticleRenderer.PPBLENDCUBIC):
                bMethod = "PPBLENDCUBIC"
            file.write(targ + '.renderer.setBlendMethod(BaseParticleRenderer.' + bMethod + ')\n')
        elif (self.rendererType == "LineParticleRenderer"):
            file.write('# Line parameters\n')
            sColor = self.renderer.getHeadColor()
            file.write((targ + '.renderer.setHeadColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
            sColor = self.renderer.getTailColor()
            file.write((targ + '.renderer.setTailColor(Vec4(%.2f, %.2f, %.2f, %.2f))\n' % (sColor[0], sColor[1], sColor[2], sColor[3])))
            sf = self.renderer.getLineScaleFactor()
            file.write((targ + '.renderer.setLineScaleFactor(%.2f)\n' % (sf)))
        elif (self.rendererType == "GeomParticleRenderer"):
            file.write('# Geom parameters\n')
            node = self.renderer.getGeomNode()
            file.write('geomRef = loader.loadModel("' + self.geomReference + '")\n')
            file.write(targ + '.renderer.setGeomNode(geomRef.node())\n')
            file.write(targ + '.geomReference = "' + self.geomReference + '"\n');
            cbmLut = ('MNone','MAdd','MSubtract','MInvSubtract','MMin','MMax')
            cboLut = ('OZero','OOne','OIncomingColor','OOneMinusIncomingColor','OFbufferColor',
                      'OOneMinusFbufferColor','OIncomingAlpha','OOneMinusIncomingAlpha',
                      'OFbufferAlpha','OOneMinusFbufferAlpha','OConstantColor',
                      'OOneMinusConstantColor','OConstantAlpha','OOneMinusConstantAlpha',
                      'OIncomingColorSaturate')
            file.write(targ + '.renderer.setXScaleFlag(%d)\n' % self.renderer.getXScaleFlag())
            file.write(targ + '.renderer.setYScaleFlag(%d)\n' % self.renderer.getYScaleFlag())
            file.write(targ + '.renderer.setZScaleFlag(%d)\n' % self.renderer.getZScaleFlag())
            file.write(targ + '.renderer.setInitialXScale(%.4f)\n' % self.renderer.getInitialXScale())
            file.write(targ + '.renderer.setFinalXScale(%.4f)\n' % self.renderer.getFinalXScale())
            file.write(targ + '.renderer.setInitialYScale(%.4f)\n' % self.renderer.getInitialYScale())
            file.write(targ + '.renderer.setFinalYScale(%.4f)\n' % self.renderer.getFinalYScale())
            file.write(targ + '.renderer.setInitialZScale(%.4f)\n' % self.renderer.getInitialZScale())
            file.write(targ + '.renderer.setFinalZScale(%.4f)\n' % self.renderer.getFinalZScale())

            cbAttrib = self.renderer.getRenderNode().getAttrib(ColorBlendAttrib.getClassType())
            if(cbAttrib):
                cbMode = cbAttrib.getMode()
                if(cbMode > 0):
                    if(cbMode in (ColorBlendAttrib.MAdd, ColorBlendAttrib.MSubtract, ColorBlendAttrib.MInvSubtract)):
                        cboa = cbAttrib.getOperandA()
                        cbob = cbAttrib.getOperandB()
                        file.write(targ+'.renderer.setColorBlendMode(ColorBlendAttrib.%s, ColorBlendAttrib.%s, ColorBlendAttrib.%s)\n' %
                                (cbmLut[cbMode], cboLut[cboa], cboLut[cbob]))
                    else:
                        file.write(targ+'.renderer.setColorBlendMode(ColorBlendAttrib.%s)\n' % cbmLut[cbMode])
            cim = self.renderer.getColorInterpolationManager()
            segIdList = [int(seg) for seg in cim.getSegmentIdList().split()]
            for sid in segIdList:
                seg = cim.getSegment(sid)
                if seg.isEnabled():
                    t_b = seg.getTimeBegin()
                    t_e = seg.getTimeEnd()
                    mod = seg.isModulated()
                    fun = seg.getFunction()
                    typ = type(fun).__name__
                    if typ == 'ColorInterpolationFunctionConstant':
                        c_a = fun.getColorA()
                        file.write(targ+'.renderer.getColorInterpolationManager().addConstant('+repr(t_b)+','+repr(t_e)+','+ \
                                   'Vec4('+repr(c_a[0])+','+repr(c_a[1])+','+repr(c_a[2])+','+repr(c_a[3])+'),'+repr(mod)+')\n')
                    elif typ == 'ColorInterpolationFunctionLinear':
                        c_a = fun.getColorA()
                        c_b = fun.getColorB()
                        file.write(targ+'.renderer.getColorInterpolationManager().addLinear('+repr(t_b)+','+repr(t_e)+','+ \
                                   'Vec4('+repr(c_a[0])+','+repr(c_a[1])+','+repr(c_a[2])+','+repr(c_a[3])+'),' + \
                                   'Vec4('+repr(c_b[0])+','+repr(c_b[1])+','+repr(c_b[2])+','+repr(c_b[3])+'),'+repr(mod)+')\n')
                    elif typ == 'ColorInterpolationFunctionStepwave':
                        c_a = fun.getColorA()
                        c_b = fun.getColorB()
                        w_a = fun.getWidthA()
                        w_b = fun.getWidthB()
                        file.write(targ+'.renderer.getColorInterpolationManager().addStepwave('+repr(t_b)+','+repr(t_e)+','+ \
                                   'Vec4('+repr(c_a[0])+','+repr(c_a[1])+','+repr(c_a[2])+','+repr(c_a[3])+'),' + \
                                   'Vec4('+repr(c_b[0])+','+repr(c_b[1])+','+repr(c_b[2])+','+repr(c_b[3])+'),' + \
                                   repr(w_a)+','+repr(w_b)+','+repr(mod)+')\n')
                    elif typ == 'ColorInterpolationFunctionSinusoid':
                        c_a = fun.getColorA()
                        c_b = fun.getColorB()
                        per = fun.getPeriod()
                        file.write(targ+'.renderer.getColorInterpolationManager().addSinusoid('+repr(t_b)+','+repr(t_e)+','+ \
                                   'Vec4('+repr(c_a[0])+','+repr(c_a[1])+','+repr(c_a[2])+','+repr(c_a[3])+'),' + \
                                   'Vec4('+repr(c_b[0])+','+repr(c_b[1])+','+repr(c_b[2])+','+repr(c_b[3])+'),' + \
                                   repr(per)+','+repr(mod)+')\n')

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
            if (lifeScale == SparkleParticleRenderer.SPSCALE):
                lScale = "SPSCALE"
            file.write(targ + '.renderer.setLifeScale(SparkleParticleRenderer.' + lScale + ')\n')
        elif (self.rendererType == "SpriteParticleRenderer"):
            file.write('# Sprite parameters\n')
            if (self.renderer.getAnimateFramesEnable()):
                file.write(targ + '.renderer.setAnimateFramesEnable(True)\n')
                rate = self.renderer.getAnimateFramesRate()
                if(rate):
                    file.write(targ + '.renderer.setAnimateFramesRate(%.3f)\n'%rate)
            animCount = self.renderer.getNumAnims()
            for x in range(animCount):
                anim = self.renderer.getAnim(x)
                if(anim.getSourceType() == SpriteAnim.STTexture):
                    file.write(targ + '.renderer.addTextureFromFile(\'%s\')\n' % (anim.getTexSource(),))
                else:
                    file.write(targ + '.renderer.addTextureFromNode(\'%s\',\'%s\')\n' % (anim.getModelSource(), anim.getNodeSource()))
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
            if (blendMethod == BaseParticleRenderer.PPNOBLEND):
                bMethod = "PPNOBLEND"
            elif (blendMethod == BaseParticleRenderer.PPBLENDLINEAR):
                bMethod = "PPBLENDLINEAR"
            elif (blendMethod == BaseParticleRenderer.PPBLENDCUBIC):
                bMethod = "PPBLENDCUBIC"
            file.write(targ + '.renderer.setAlphaBlendMethod(BaseParticleRenderer.' + bMethod + ')\n')
            file.write(targ + '.renderer.setAlphaDisable(%d)\n' % self.renderer.getAlphaDisable())
            # Save the color blending to file
            cbmLut = ('MNone','MAdd','MSubtract','MInvSubtract','MMin','MMax')
            cboLut = ('OZero','OOne','OIncomingColor','OOneMinusIncomingColor','OFbufferColor',
                      'OOneMinusFbufferColor','OIncomingAlpha','OOneMinusIncomingAlpha',
                      'OFbufferAlpha','OOneMinusFbufferAlpha','OConstantColor',
                      'OOneMinusConstantColor','OConstantAlpha','OOneMinusConstantAlpha',
                      'OIncomingColorSaturate')
            cbAttrib = self.renderer.getRenderNode().getAttrib(ColorBlendAttrib.getClassType())
            if(cbAttrib):
                cbMode = cbAttrib.getMode()
                if(cbMode > 0):
                    if(cbMode in (ColorBlendAttrib.MAdd, ColorBlendAttrib.MSubtract, ColorBlendAttrib.MInvSubtract)):
                        cboa = cbAttrib.getOperandA()
                        cbob = cbAttrib.getOperandB()
                        file.write(targ+'.renderer.setColorBlendMode(ColorBlendAttrib.%s, ColorBlendAttrib.%s, ColorBlendAttrib.%s)\n' %
                                (cbmLut[cbMode], cboLut[cboa], cboLut[cbob]))
                    else:
                        file.write(targ+'.renderer.setColorBlendMode(ColorBlendAttrib.%s)\n' % cbmLut[cbMode])
            cim = self.renderer.getColorInterpolationManager()
            segIdList = [int(seg) for seg in cim.getSegmentIdList().split()]
            for sid in segIdList:
                seg = cim.getSegment(sid)
                if seg.isEnabled():
                    t_b = seg.getTimeBegin()
                    t_e = seg.getTimeEnd()
                    mod = seg.isModulated()
                    fun = seg.getFunction()
                    typ = type(fun).__name__
                    if typ == 'ColorInterpolationFunctionConstant':
                        c_a = fun.getColorA()
                        file.write(targ+'.renderer.getColorInterpolationManager().addConstant('+repr(t_b)+','+repr(t_e)+','+ \
                                   'Vec4('+repr(c_a[0])+','+repr(c_a[1])+','+repr(c_a[2])+','+repr(c_a[3])+'),'+repr(mod)+')\n')
                    elif typ == 'ColorInterpolationFunctionLinear':
                        c_a = fun.getColorA()
                        c_b = fun.getColorB()
                        file.write(targ+'.renderer.getColorInterpolationManager().addLinear('+repr(t_b)+','+repr(t_e)+','+ \
                                   'Vec4('+repr(c_a[0])+','+repr(c_a[1])+','+repr(c_a[2])+','+repr(c_a[3])+'),' + \
                                   'Vec4('+repr(c_b[0])+','+repr(c_b[1])+','+repr(c_b[2])+','+repr(c_b[3])+'),'+repr(mod)+')\n')
                    elif typ == 'ColorInterpolationFunctionStepwave':
                        c_a = fun.getColorA()
                        c_b = fun.getColorB()
                        w_a = fun.getWidthA()
                        w_b = fun.getWidthB()
                        file.write(targ+'.renderer.getColorInterpolationManager().addStepwave('+repr(t_b)+','+repr(t_e)+','+ \
                                   'Vec4('+repr(c_a[0])+','+repr(c_a[1])+','+repr(c_a[2])+','+repr(c_a[3])+'),' + \
                                   'Vec4('+repr(c_b[0])+','+repr(c_b[1])+','+repr(c_b[2])+','+repr(c_b[3])+'),' + \
                                   repr(w_a)+','+repr(w_b)+','+repr(mod)+')\n')
                    elif typ == 'ColorInterpolationFunctionSinusoid':
                        c_a = fun.getColorA()
                        c_b = fun.getColorB()
                        per = fun.getPeriod()
                        file.write(targ+'.renderer.getColorInterpolationManager().addSinusoid('+repr(t_b)+','+repr(t_e)+','+ \
                                   'Vec4('+repr(c_a[0])+','+repr(c_a[1])+','+repr(c_a[2])+','+repr(c_a[3])+'),' + \
                                   'Vec4('+repr(c_b[0])+','+repr(c_b[1])+','+repr(c_b[2])+','+repr(c_b[3])+'),' + \
                                   repr(per)+','+repr(mod)+')\n')

        file.write('# Emitter parameters\n')
        emissionType = self.emitter.getEmissionType()
        eType = "ETEXPLICIT"
        if (emissionType == BaseParticleEmitter.ETEXPLICIT):
            eType = "ETEXPLICIT"
        elif (emissionType == BaseParticleEmitter.ETRADIATE):
            eType = "ETRADIATE"
        elif (emissionType == BaseParticleEmitter.ETCUSTOM):
            eType = "ETCUSTOM"
        file.write(targ + '.emitter.setEmissionType(BaseParticleEmitter.' + eType + ')\n')
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
            point = self.emitter.getMinBound()
            file.write((targ + '.emitter.setMinBound(Point2(%.4f, %.4f))\n' % (point[0], point[1])))
            point = self.emitter.getMaxBound()
            file.write((targ + '.emitter.setMaxBound(Point2(%.4f, %.4f))\n' % (point[0], point[1])))
        elif (self.emitterType == "RingEmitter"):
            file.write('# Ring parameters\n')
            file.write(targ + '.emitter.setRadius(%.4f)\n' % self.emitter.getRadius())
            file.write(targ + '.emitter.setRadiusSpread(%.4f)\n' % self.emitter.getRadiusSpread())
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
            file.write(targ + '.emitter.setRadiusSpread(%.4f)\n' % self.emitter.getRadiusSpread())

    def getPoolSizeRanges(self):
        litterRange = [max(1,self.getLitterSize()-self.getLitterSpread()),
                       self.getLitterSize(),
                       self.getLitterSize()+self.getLitterSpread()]
        lifespanRange = [self.factory.getLifespanBase()-self.factory.getLifespanSpread(),
                         self.factory.getLifespanBase(),
                         self.factory.getLifespanBase()+self.factory.getLifespanSpread()]
        birthRateRange = [self.getBirthRate()] * 3

        print('Litter Ranges:    %s' % litterRange)
        print('LifeSpan Ranges:  %s' % lifespanRange)
        print('BirthRate Ranges: %s' % birthRateRange)

        return dict(zip(('min','median','max'),[l*s/b for l,s,b in zip(litterRange,lifespanRange,birthRateRange)]))

    def accelerate(self,time,stepCount = 1,stepTime=0.0):
        if time > 0.0:
            if stepTime == 0.0:
                stepTime = float(time)/stepCount
                remainder = 0.0
            else:
                stepCount = int(float(time)/stepTime)
                remainder = time-stepCount*stepTime

            for step in range(stepCount):
                base.particleMgr.doParticles(stepTime,self,False)
                base.physicsMgr.doPhysics(stepTime,self)

            if(remainder):
                base.particleMgr.doParticles(remainder,self,False)
                base.physicsMgr.doPhysics(remainder,self)

            self.render()
