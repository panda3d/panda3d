from IntervalGlobal import *
import BasicEntities
import random

class AmbientSound(BasicEntities.NodePathEntity):
    def __init__(self, level, entId):
        BasicEntities.NodePathEntity.__init__(self, level, entId)
        self.initSound()

    def destroy(self):
        self.destroySound()
        BasicEntities.NodePathEntity.destroy(self)

    def initSound(self):
        if not self.enabled:
            return
        if self.soundPath == '':
            return
        self.sound = base.loadSfx(self.soundPath)
        if self.sound is None:
            return
        self.soundIval = SoundInterval(self.sound, node=self,
                                       volume=self.volume)
        self.soundIval.loop()
        self.soundIval.setT(random.random() * self.sound.length())

    def destroySound(self):
        if hasattr(self, 'soundIval'):
            self.soundIval.pause()
            del self.soundIval
        if hasattr(self, 'sound'):
            del self.sound

    if __dev__:
        def attribChanged(self, *args):
            self.destroySound()
            self.initSound()
