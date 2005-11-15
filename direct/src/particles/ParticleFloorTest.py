
from pandac.PandaModules import *
from direct.particles import ParticleEffect
from direct.particles import Particles
from direct.particles import ForceGroup

class ParticleFloorTest(NodePath):
    def __init__(self):
        NodePath.__init__(self, "particleFloorTest")
        
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
        self.f.addParticles(self.p0)  
        if 1:
            f0 = ForceGroup.ForceGroup('frict')
            # Force parameters
            force0 = LinearVectorForce(Vec3(0., 0., -1.))
            force0.setActive(1)
            f0.addForce(force0)
            self.f.addForceGroup(f0)
    
    def start(self):
        self.f.enable()

if __name__ == "__main__":
    from direct.directbase.TestStart import *
    pt=ParticleFloorTest()
    pt.reparentTo(render)
    pt.start()
    camera.setY(-10.0)
    run()
