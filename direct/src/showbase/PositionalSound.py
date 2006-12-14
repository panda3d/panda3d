from direct.interval.IntervalGlobal import *
from pandac.PandaModules import NodePath
import random

class PositionalSound:
    def __init__(self, soundPath, node, volume=None):
        if volume is None:
            volume = 1.
        self._sound = base.loadSfx(soundPath)
        self._ival = SoundInterval(self._sound, node=node, volume=volume)
        self.start()
    def destroy(self):
        self._ival.pause()
        del self._ival
        del self._sound
    def start(self):
        self._ival.loop()
        self._ival.setT(random.random() * self._sound.length())
    def stop(self):
        self._ival.stop()
