from direct.directbase.TestStart import *
from direct.interval.IntervalGlobal import *
from direct.actor import Actor
from direct.particles import ParticleEffect
from direct.particles import Particles
from direct.particles import ForceGroup
from direct.showbase import Pool

class ParticleFloorTest(NodePath):
    def __init__(self, pool=None):
        NodePath.__init__(self, "particleTest")
        self.pool = pool
        
        # Sort Order of Particles
        self.setDepthWrite(0)

        # Load Particle Effects
        self.f = ParticleEffect.ParticleEffect()
        self.f.reparentTo(self)         
        self.p0 = Particles.Particles('particles-1')
        # Particles parameters
        self.p0.setFactory("PointParticleFactory")
        self.p0.setRenderer("PointParticleRenderer")
        self.p0.setEmitter("SphereVolumeEmitter")
        self.p0.setPoolSize(64)
        self.p0.setBirthRate(0.020)
        self.p0.setLitterSize(7)
        self.p0.setLitterSpread(2)
        self.p0.setSystemLifespan(0.0000)
        #self.p0.setLocalVelocityFlag(1) 
        self.p0.setFloorZ(-1.0)
        self.p0.setSystemGrowsOlderFlag(0)
        # Factory parameters
        self.p0.factory.setLifespanBase(10.000)
        self.p0.factory.setLifespanSpread(0.50)
        self.p0.factory.setMassBase(1.80)
        self.p0.factory.setMassSpread(1.00)
        self.p0.factory.setTerminalVelocityBase(400.0000)
        self.p0.factory.setTerminalVelocitySpread(0.0000)
        if 0:
            # Z Spin factory parameters
            self.p0.factory.setInitialAngle(0.0000)
            self.p0.factory.setInitialAngleSpread(360.0000)
            self.p0.factory.enableAngularVelocity(1)
            self.p0.factory.setAngularVelocity(40.00)
            self.p0.factory.setAngularVelocitySpread(20.00)
        if 0:
            # Renderer parameters
            self.p0.renderer.setAlphaMode(BaseParticleRenderer.PRALPHAOUT)
            self.p0.renderer.setUserAlpha(1.0)
            # Sprite parameters
            self.p0.renderer.setTexture(loader.loadTexture('maps/smiley.rgb'))
            self.p0.renderer.setColor(Vec4(1.00, 1.00, 1.00, 1.00))
            self.p0.renderer.setXScaleFlag(1)
            self.p0.renderer.setYScaleFlag(1)
            self.p0.renderer.setZScaleFlag(1)
            self.p0.renderer.setAnimAngleFlag(1)
            self.p0.renderer.setInitialXScale(0.0012)
            self.p0.renderer.setFinalXScale(0.0035)
            self.p0.renderer.setInitialYScale(0.0012)
            self.p0.renderer.setFinalYScale(0.0035)
            self.p0.renderer.setInitialZScale(0.0012)
            self.p0.renderer.setFinalZScale(0.0035)
            self.p0.renderer.setNonanimatedTheta(0.0000)
            self.p0.renderer.setAlphaBlendMethod(BaseParticleRenderer.PPNOBLEND)
            self.p0.renderer.setAlphaDisable(0)
        if 0:
            # Emitter parameters
            self.p0.emitter.setEmissionType(BaseParticleEmitter.ETRADIATE)
            self.p0.emitter.setAmplitude(57.000)
            self.p0.emitter.setAmplitudeSpread(1.000)
            self.p0.emitter.setOffsetForce(Vec3(5.0000, 5.0000, 5.000))
            #self.p0.emitter.setExplicitLaunchVector(Vec3(0.0000, 0.0000, 1.0000))
            self.p0.emitter.setRadiateOrigin(Point3(0.0000, 0.0000, 0.0000))
            # Sphere Volume parameters
            self.p0.emitter.setRadius(0.500)       
        self.f.addParticles(self.p0)  
        if 0:
            f0 = ForceGroup.ForceGroup('frict')
            # Force parameters
            force0 = LinearFrictionForce(1., 1., 1.)
            force0.setActive(1)
            f0.addForce(force0)
            self.f.addForceGroup(f0)
        if 1:
            f0 = ForceGroup.ForceGroup('frict')
            # Force parameters
            force0 = LinearVectorForce(Vec3(0., 0., -1.))
            force0.setActive(1)
            f0.addForce(force0)
            self.f.addForceGroup(f0)

        self.createTrack()
    
    def createTrack(self, rate = 1):
        print "createTrack"
        self.track = Sequence(
            Func(self.p0.setBirthRate, 0.020),
            Func(self.f.enable),
            Func(self.f.reparentTo, self),
            Wait(0.3),
            Func(self.p0.setBirthRate, 100),
            Wait(1.5),
            Func(self.finish)
            )
    
    def start(self):
        print "start"
        self.f.enable()
        self.f.reparentTo(self)

    def play(self, rate = 1):
        print "play"
        self.track.start()
    
    def loop(self, rate = 1):
        print "loop"
        self.track.loop()
    
    def stop(self):
        if self.track:
            self.track.finish()

    def finish(self):
        print "finish"
        self.stop()
        self.f.disable()
        self.reparentTo(hidden)
        if self.pool:
            self.pool.checkin(self)

    def destroy(self):
        print "destroy"
        self.stop()
        self.f.cleanup()
        del self.f
        del self.p0
        self.removeNode()
        if self.pool:
            self.pool.remove(self)
        del self.pool
        del self.track

pt=ParticleFloorTest()
pt.reparentTo(render)
pt.start()

