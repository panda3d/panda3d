from IntervalGlobal import *
import BasicEntities

class AmbientSound(BasicEntities.NodePathEntity):
    def __init__(self, level, entId):
        BasicEntities.NodePathEntity.__init__(self, level, entId)

    def destroy(self):
        BasicEntities.NodePathEntity.destroy(self)

    def initSound(self):
        if self.soundPath == '':
            return
        self.sound = base.loadSfx(self.soundPath)
        if self.sound is None:
            return
        self.soundIval = SoundInterval(self.sound, node=self)
        self.soundIval.loop()

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
