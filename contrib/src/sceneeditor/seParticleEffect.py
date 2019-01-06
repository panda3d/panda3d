from panda3d.core import *
import seParticles
import seForceGroup
from direct.directnotify import DirectNotifyGlobal

class ParticleEffect(NodePath):

    notify = DirectNotifyGlobal.directNotify.newCategory('ParticleEffect')
    pid = 1

    def __init__(self, name=None, particles=None):
        """__init__()"""
        if (name == None):
            name = 'particle-effect-%d' % ParticleEffect.pid
            ParticleEffect.pid += 1
        NodePath.__init__(self, name)
        # Record particle effect name
        self.name = name
        # Enabled flag
        self.fEnabled = 0
        # Dictionary of particles and forceGroups
        self.particlesDict = {}
        self.forceGroupDict = {}
        # The effect's particle system
        if (particles != None):
            self.addParticles(particles)
        self.renderParent = None

    def start(self, parent=None, renderParent=None):
        assert(self.notify.debug('start() - name: %s' % self.name))
        self.renderParent = renderParent
        self.enable()
        if (parent != None):
            self.reparentTo(parent)

    def cleanup(self):
        self.removeNode()
        self.disable()
        for f in self.forceGroupDict.values():
            f.cleanup()
        for p in self.particlesDict.values():
            p.cleanup()
        del self.renderParent
        del self.particlesDict
        del self.forceGroupDict

    def reset(self):
        self.removeAllForces()
        self.removeAllParticles()
        self.forceGroupDict = {}
        self.particlesDict = {}

    def enable(self):
        """enable()"""
        if (self.renderParent != None):
            for p in self.particlesDict.values():
                p.setRenderParent(self.renderParent.node())
        for f in self.forceGroupDict.values():
            f.enable()
        for p in self.particlesDict.values():
            p.enable()
        self.fEnabled = 1

    def disable(self):
        """disable()"""
        self.detachNode()
        for p in self.particlesDict.values():
            p.setRenderParent(p.node)
        for f in self.forceGroupDict.values():
            f.disable()
        for p in self.particlesDict.values():
            p.disable()
        self.fEnabled = 0

    def isEnabled(self):
        """
        isEnabled()
        Note: this may be misleading if enable(),disable() not used
        """
        return self.fEnabled

    def addForceGroup(self, forceGroup):
        """addForceGroup(forceGroup)"""
        forceGroup.nodePath.reparentTo(self)
        forceGroup.particleEffect = self
        self.forceGroupDict[forceGroup.getName()] = forceGroup

        # Associate the force group with all particles
        for i in range(len(forceGroup)):
            self.addForce(forceGroup[i])

    def addForce(self, force):
        """addForce(force)"""
        for p in self.particlesDict.values():
            p.addForce(force)

    def removeForceGroup(self, forceGroup):
        """removeForceGroup(forceGroup)"""
        # Remove forces from all particles
        for i in range(len(forceGroup)):
            self.removeForce(forceGroup[i])

        forceGroup.nodePath.removeNode()
        forceGroup.particleEffect = None
        del self.forceGroupDict[forceGroup.getName()]

    def removeForce(self, force):
        """removeForce(force)"""
        for p in self.particlesDict.values():
            p.removeForce(force)

    def removeAllForces(self):
        for fg in self.forceGroupDict.values():
            self.removeForceGroup(fg)

    def addParticles(self, particles):
        """addParticles(particles)"""
        particles.nodePath.reparentTo(self)
        self.particlesDict[particles.getName()] = particles

        # Associate all forces in all force groups with the particles
        for fg in self.forceGroupDict.values():
            for i in range(len(fg)):
                particles.addForce(fg[i])

    def removeParticles(self, particles):
        """removeParticles(particles)"""
        if (particles == None):
            self.notify.warning('removeParticles() - particles == None!')
            return
        particles.nodePath.detachNode()
        del self.particlesDict[particles.getName()]

        # Remove all forces from the particles
        for fg in self.forceGroupDict.values():
            for f in fg.asList():
                particles.removeForce(f)

    def removeAllParticles(self):
        for p in self.particlesDict.values():
            self.removeParticles(p)

    def getParticlesList(self):
        """getParticles()"""
        return self.particlesDict.values()

    def getParticlesNamed(self, name):
        """getParticlesNamed(name)"""
        return self.particlesDict.get(name, None)

    def getParticlesDict(self):
        """getParticlesDict()"""
        return self.particlesDict

    def getForceGroupList(self):
        """getForceGroup()"""
        return self.forceGroupDict.values()

    def getForceGroupNamed(self, name):
        """getForceGroupNamed(name)"""
        return self.forceGroupDict.get(name, None)

    def getForceGroupDict(self):
        """getForceGroup()"""
        return self.forceGroupDict

    def saveConfig(self, filename):
        """saveFileData(filename)"""
        f = open(filename.toOsSpecific(), 'wb')
        # Add a blank line
        f.write('\n')

        # Make sure we start with a clean slate
        f.write('self.reset()\n')

        pos = self.getPos()
        hpr = self.getHpr()
        scale = self.getScale()
        f.write('self.setPos(%0.3f, %0.3f, %0.3f)\n' %
                (pos[0], pos[1], pos[2]))
        f.write('self.setHpr(%0.3f, %0.3f, %0.3f)\n' %
                (hpr[0], hpr[1], hpr[2]))
        f.write('self.setScale(%0.3f, %0.3f, %0.3f)\n' %
                (scale[0], scale[1], scale[2]))

        # Save all the particles to file
        num = 0
        for p in self.particlesDict.values():
            target = 'p%d' % num
            num = num + 1
            f.write(target + ' = Particles.Particles(\'%s\')\n' % p.getName())
            p.printParams(f, target)
            f.write('self.addParticles(%s)\n' % target)

        # Save all the forces to file
        num = 0
        for fg in self.forceGroupDict.values():
            target = 'f%d' % num
            num = num + 1
            f.write(target + ' = ForceGroup.ForceGroup(\'%s\')\n' % \
                                                fg.getName())
            fg.printParams(f, target)
            f.write('self.addForceGroup(%s)\n' % target)

        # Close the file
        f.close()

    def loadConfig(self, filename):
        """loadConfig(filename)"""
        #try:
        #    if vfs:
        print(vfs.readFile(filename))
        exec(vfs.readFile(filename))
        print("Particle Effect Reading using VFS")
        #    else:
        #       execfile(filename.toOsSpecific())
        #       print "Shouldnt be wrong"
        #except:
        #    self.notify.error('loadConfig: failed to load particle file: '+ repr(filename))



    def AppendConfig(self, f):
        f.write('\n')
        i1="    "
        i2=i1+i1
        # Make sure we start with a clean slate
        f.write(i2+'self.effect.reset()\n')

        pos = self.getPos()
        hpr = self.getHpr()
        scale = self.getScale()
        f.write(i2+'self.effect.setPos(%0.3f, %0.3f, %0.3f)\n' %
                (pos[0], pos[1], pos[2]))
        f.write(i2+'self.effect.setHpr(%0.3f, %0.3f, %0.3f)\n' %
                (hpr[0], hpr[1], hpr[2]))
        f.write(i2+'self.effect.setScale(%0.3f, %0.3f, %0.3f)\n' %
                (scale[0], scale[1], scale[2]))

        # Save all the particles to file
        num = 0
        for p in self.particlesDict.values():
            target = 'p%d' % num
            num = num + 1
            f.write(i2+"if(mode==0):\n")
            f.write(i2+i1+target + ' = seParticles.Particles(\'%s\')\n' % p.getName())
            f.write(i2+"else:\n")
            f.write(i2+i1+target + ' = Particles.Particles(\'%s\')\n' % p.getName())
            p.printParams(f, target)
            f.write(i2+'self.effect.addParticles(%s)\n' % target)

        # Save all the forces to file
        num = 0
        for fg in self.forceGroupDict.values():
            target = 'f%d' % num
            num = num + 1
            f.write(i2+target + ' = ForceGroup.ForceGroup(\'%s\')\n' % \
                                                fg.getName())
            fg.printParams(f, target)
            f.write(i2+'self.effect.addForceGroup(%s)\n' % target)







