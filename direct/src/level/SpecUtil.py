"""SpecUtil module: contains utility functions for creating and managing level specs"""

import LevelSpec
import LevelConstants

def makeNewSpec(filename, modelPath):
    """call this to create a new level spec for the level model at 'modelPath'.
    Spec will be saved as 'filename'"""
    spec = LevelSpec.LevelSpec()
    spec.doSetAttrib(LevelConstants.LevelMgrEntId,
                     'modelFilename', modelPath)
    spec.saveToDisk(filename, makeBackup=0)

def updateSpec(specModule, modelPath=None):
    """Call this to update an existing levelSpec to work with a new level
    model. If the level model has a new path, pass it in as 'modelPath'.
    specModule must be a Python module"""
    spec = LevelSpec.LevelSpec(specModule)
    if modelPath is None:
        modelPath = spec.getEntitySpec(
            LevelConstants.LevelMgrEntId)['modelFilename']

    # ...

    spec.saveToDisk()
