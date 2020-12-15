"""PANDA3D Particle Panel"""

__all__ = ['ParticlePanel']

from direct.tkpanels.ParticlePanel import ParticlePanel


class SeParticlePanel(ParticlePanel):
    """Scene Editor specific Particle Panel. This only adds a sphere to which
    the particles get reparented to make them selectable."""

    def __init__(self, particleEffect = None, effectsDict={}, **kw):
        ParticlePanel.__init__(self)
        # Record particle effect
        if particleEffect != None:
            self.effectsDict = effectsDict
        else:
            # Scene Editor custom. Create a sphere to mae the particles selectable
            self.emitter=loader.loadModel("misc/sphere")
            self.particleEffect.reparentTo(self.emitter)
            self.emitter.setName("effect1")
            self.emitter.reparentTo(render)

            messenger.send('ParticlePanel_Added_Effect',['effect1',self.particleEffect,self.emitter])
            self.effectsDict[self.particleEffect.getName()]=self.particleEffect

        messenger.send('SGE_Update Explorer',[render])

    def onDestroy(self, event):
        messenger.send('ParticlePanle_close')
        return

    def loadParticleEffectFromFile(self):
        ParticlePanel.loadParticleEffectFromFile(self)
        messenger.send('SGE_Update Explorer',[render])

    ## SYSTEM COMMANDS ##
    def createNewEffect(self):
        ParticlePanel.createNewEffect()
        if name:
            self.emitter=loader.loadModel("sphere")
            self.emitter.setName(name)
            self.effectsDict[name].reparentTo(self.emitter)
            self.emitter.reparentTo(render)

            messenger.send('ParticlePanel_Added_Effect',[name,self.effectsDict[name],self.emitter])
            messenger.send('SGE_Update Explorer',[render])

