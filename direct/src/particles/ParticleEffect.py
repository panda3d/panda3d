from PandaObject import *
from DirectObject import *

import Particles
import ForceGroup

class ParticleEffect(NodePath):
    def __init__(self, name = 'ParticleEffect'):
	"""__init__()"""
	NodePath.__init__(self)
        self.assign(hidden.attachNewNode(name))
        # Record particle effect name
	self.name = name
        # Enabled flag
        self.fEnabled = 0
        # Dictionary of particles and forceGroups
	self.particlesDict = {}
	self.forceGroupDict = {}
        # The effect's particle system
	self.addParticles(Particles.Particles())

    def enable(self):
	"""enable()"""
	for f in self.forceGroupDict.values():
	    f.enable()
	for p in self.particlesDict.values():
	    p.enable()
        self.fEnabled = 1

    def disable(self):
	"""disable()"""
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
	self.forceGroupDict[forceGroup.getName()] = forceGroup

    def addParticles(self, particles):
	"""addParticles(particles)"""
	particles.nodePath.reparentTo(self)
	self.particlesDict[particles.getName()] = particles

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
        #fname = Filename(filename)
        #fname.resolveFilename(getParticlePath())
        #fname.resolveFilename(getModelPath())
        f = open(filename.toOsSpecific(), 'wb')
        # Add a blank line
        f.write('\n')

	# Save all the forces to file
	num = 0
	for fg in self.forceGroupDict.values():
	    target = 'f%d' % num
	    num = num + 1
	    f.write(target + ' = ForceGroup.ForceGroup(\'%s\')\n' % fg.getName())
	    fg.printParams(f, target)	
	    f.write('self.addForceGroup(%s)\n' % target)

        # Save all the particles to file
	f.write('self.particlesDict = {}\n')
	num = 0
	for p in self.particlesDict.values():
	    target = 'p%d' % num 
	    num = num + 1
	    f.write(target + ' = Particles.Particles(\'%s\')\n' % p.getName())
	    p.printParams(f, target)
	    f.write('self.addParticles(%s)\n' % target)

        # Close the file
        f.close()

    def loadConfig(self, filename):
	"""loadConfig(filename)"""
	execfile(filename.toOsSpecific())
