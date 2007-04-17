# objects that report different types of leaks to the ContainerLeakDetector

class SceneGraphLeakDetector:
    # is a scene graph leaking nodes?
    def __init__(self, render):
        self._render = render
        if config.GetBool('leak-scene-graph', 0):
            self._leakTaskName = 'leakNodes-%s' % serialNum()
            self._leakNode()
    def destroy(self):
        if hasattr(self, '_leakTaskName'):
            taskMgr.remove(self._leakTaskName)
        del self._render
    def __len__(self):
        return self._render.getNumDescendants()
    def __repr__(self):
        return 'SceneGraphLeakDetector(%s)' % self._render
    def _leakNode(self, task=None):
        self._render.attachNewNode('leakNode-%s' % serialNum())
        taskMgr.doMethodLater(10, self._leakNode, self._leakTaskName)
