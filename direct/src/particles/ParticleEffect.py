# Leave these imports in, they may be used by ptf files.
from panda3d.core import *
from panda3d.physics import *
from . import Particles
from . import ForceGroup

from direct.directnotify import DirectNotifyGlobal


class ParticleEffect(NodePath):
    notify = DirectNotifyGlobal.directNotify.newCategory('ParticleEffect')
    pid = 1

    def __init__(self, name=None, particles=None):
        if name is None:
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
        if particles is not None:
            self.addParticles(particles)
        self.renderParent = None

    def cleanup(self):
        self.removeNode()
        self.disable()
        if self.__isValid():
            for f in self.forceGroupDict.values():
                f.cleanup()
            for p in self.particlesDict.values():
                p.cleanup()
            del self.forceGroupDict
            del self.particlesDict
        del self.renderParent

    def getName(self):
        # override NodePath.getName()
        return self.name

    def reset(self):
        self.removeAllForces()
        self.removeAllParticles()
        self.forceGroupDict = {}
        self.particlesDict = {}

    def start(self, parent=None, renderParent=None):
        assert self.notify.debug('start() - name: %s' % self.name)
        self.renderParent = renderParent
        self.enable()
        if parent is not None:
            self.reparentTo(parent)

    def enable(self):
        # band-aid added for client crash - grw
        if self.__isValid():
            if self.renderParent:
                for p in self.particlesDict.values():
                    p.setRenderParent(self.renderParent.node())
            for f in self.forceGroupDict.values():
                f.enable()
            for p in self.particlesDict.values():
                p.enable()
            self.fEnabled = 1

    def disable(self):
        self.detachNode()
        # band-aid added for client crash - grw
        if self.__isValid():
            for p in self.particlesDict.values():
                p.setRenderParent(p.node)
            for f in self.forceGroupDict.values():
                f.disable()
            for p in self.particlesDict.values():
                p.disable()
            self.fEnabled = 0

    def isEnabled(self):
        """
        Note: this may be misleading if enable(), disable() not used
        """
        return self.fEnabled

    def addForceGroup(self, forceGroup):
        forceGroup.nodePath.reparentTo(self)
        forceGroup.particleEffect = self
        self.forceGroupDict[forceGroup.getName()] = forceGroup

        # Associate the force group with all particles
        for i in range(len(forceGroup)):
            self.addForce(forceGroup[i])

    def addForce(self, force):
        for p in list(self.particlesDict.values()):
            p.addForce(force)

    def removeForceGroup(self, forceGroup):
        # Remove forces from all particles
        for i in range(len(forceGroup)):
            self.removeForce(forceGroup[i])

        forceGroup.nodePath.removeNode()
        forceGroup.particleEffect = None
        self.forceGroupDict.pop(forceGroup.getName(), None)

    def removeForce(self, force):
        for p in list(self.particlesDict.values()):
            p.removeForce(force)

    def removeAllForces(self):
        for fg in list(self.forceGroupDict.values()):
            self.removeForceGroup(fg)

    def addParticles(self, particles):
        particles.nodePath.reparentTo(self)
        self.particlesDict[particles.getName()] = particles

        # Associate all forces in all force groups with the particles
        for fg in list(self.forceGroupDict.values()):
            for i in range(len(fg)):
                particles.addForce(fg[i])

    def removeParticles(self, particles):
        if particles is None:
            self.notify.warning('removeParticles() - particles == None!')
            return
        particles.nodePath.detachNode()
        self.particlesDict.pop(particles.getName(), None)

        # Remove all forces from the particles
        for fg in list(self.forceGroupDict.values()):
            for f in fg:
                particles.removeForce(f)

    def removeAllParticles(self):
        for p in list(self.particlesDict.values()):
            self.removeParticles(p)

    def getParticlesList(self):
        return list(self.particlesDict.values())

    def getParticlesNamed(self, name):
        return self.particlesDict.get(name, None)

    def getParticlesDict(self):
        return self.particlesDict

    def getForceGroupList(self):
        return list(self.forceGroupDict.values())

    def getForceGroupNamed(self, name):
        return self.forceGroupDict.get(name, None)

    def getForceGroupDict(self):
        return self.forceGroupDict

    def saveConfig(self, filename):
        filename = Filename(filename)
        with open(filename.toOsSpecific(), 'w') as f:
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
          for p in list(self.particlesDict.values()):
              target = 'p%d' % num
              num = num + 1
              f.write(target + ' = Particles.Particles(\'%s\')\n' % p.getName())
              p.printParams(f, target)
              f.write('self.addParticles(%s)\n' % target)

          # Save all the forces to file
          num = 0
          for fg in list(self.forceGroupDict.values()):
              target = 'f%d' % num
              num = num + 1
              f.write(target + ' = ForceGroup.ForceGroup(\'%s\')\n' % \
                                                  fg.getName())
              fg.printParams(f, target)
              f.write('self.addForceGroup(%s)\n' % target)

    def loadConfig(self, filename):
        vfs = VirtualFileSystem.getGlobalPtr()
        data = vfs.readFile(filename, 1)
        data = data.replace(b'\r', b'')
        try:
            exec(data)
        except:
            self.notify.warning('loadConfig: failed to load particle file: '+ repr(filename))
            raise

    def accelerate(self,time,stepCount = 1,stepTime=0.0):
        for particles in self.getParticlesList():
            particles.accelerate(time,stepCount,stepTime)

    def clearToInitial(self):
        for particles in self.getParticlesList():
            particles.clearToInitial()

    def softStop(self):
        for particles in self.getParticlesList():
            particles.softStop()

    def softStart(self, firstBirthDelay=None):
        if self.__isValid():
            for particles in self.getParticlesList():
                if firstBirthDelay is not None:
                    particles.softStart(br=-1, first_birth_delay=firstBirthDelay)
                else:
                    particles.softStart()
        else:
            # Not asserting here since we want to crash live clients for more expedient bugfix
            # (Sorry, live clients)
            self.notify.error('Trying to start effect(%s) after cleanup.' % (self.getName(),))

    def __isValid(self):
        return hasattr(self, 'forceGroupDict') and \
               hasattr(self, 'particlesDict')

    # Snake-case aliases.
    is_enabled = isEnabled
    add_force_group = addForceGroup
    add_force = addForce
    remove_force_group = removeForceGroup
    remove_force = removeForce
    remove_all_forces = removeAllForces
    add_particles = addParticles
    remove_particles = removeParticles
    remove_all_particles = removeAllParticles
    get_particles_list = getParticlesList
    get_particles_named = getParticlesNamed
    get_particles_dict = getParticlesDict
    get_force_group_list = getForceGroupList
    get_force_group_named = getForceGroupNamed
    get_force_group_dict = getForceGroupDict
    save_config = saveConfig
    load_config = loadConfig
    clear_to_initial = clearToInitial
    soft_stop = softStop
    soft_start = softStart
