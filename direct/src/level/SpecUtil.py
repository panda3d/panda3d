"""SpecUtil module: contains utility functions for creating and managing level specs"""

from ShowBaseGlobal import *
import LevelSpec
import LevelConstants
import LevelUtil
from PythonUtil import list2dict
import EntityTypes
import types

"""
TO CREATE A NEW SPEC:
import SpecUtil
import FactoryEntityTypes
SpecUtil.makeNewSpec('$TOONTOWN/src/coghq/FactoryMockupSpec.py', 'phase_9/models/cogHQ/SelbotLegFactory', FactoryEntityTypes)
"""
def makeNewSpec(filename, modelPath, entTypeModule=EntityTypes):
    """call this to create a new level spec for the level model at 'modelPath'.
    Spec will be saved as 'filename'"""
    spec = LevelSpec.LevelSpec()
    # make every zone visible from every other zone
    privUpdateSpec(spec, modelPath, entTypeModule, newZonesGloballyVisible=1)
    # expand any env vars, then convert to an OS-correct path
    fname = Filename.expandFrom(filename).toOsSpecific()
    spec.saveToDisk(fname, makeBackup=0)
    print 'Done.'

"""
FOR SAME LEVEL MODEL PATH:
import SpecUtil
import SellbotLegFactorySpec
import FactoryEntityTypes
SpecUtil.updateSpec(SellbotLegFactorySpec, FactoryEntityTypes)

FOR DIFFERENT LEVEL MODEL PATH:
import SpecUtil
import SellbotLegFactorySpec
import FactoryEntityTypes
SpecUtil.updateSpec(SellbotLegFactorySpec, FactoryEntityTypes, '/i/beta/toons/maya/work/CogHeadquarters/CogFactoriesInteriors/AllFactories/LegFactory/SelbotLegFactory_v##s#.mb')
"""

def updateSpec(specModule, entTypeModule=EntityTypes, modelPath=None):
    """Call this to update an existing levelSpec to work with a new level
    model. If the level model has a new path, pass it in as 'modelPath'.
    specModule must be a Python module"""
    spec = LevelSpec.LevelSpec(specModule)
    privUpdateSpec(spec, modelPath, entTypeModule)
    spec.saveToDisk()
    print 'Done.'

def privUpdateSpec(spec, modelPath, entTypeModule, newZonesGloballyVisible=0):
    """internal: take a spec and update it to match its level model
    If newZonesGloballyVisible is true, any new zones will be added to the
    visibility lists for every zone.
    """
    assert __dev__
    assert type(entTypeModule) is types.ModuleType
    import EntityTypeRegistry
    etr = EntityTypeRegistry.EntityTypeRegistry(entTypeModule)
    spec.setEntityTypeReg(etr)

    if modelPath is None:
        modelPath = spec.getEntitySpec(
            LevelConstants.LevelMgrEntId)['modelFilename']
    else:
        spec.doSetAttrib(LevelConstants.LevelMgrEntId,
                         'modelFilename', modelPath)

    # load the model
    # disable texture loading for speed
    TexturePool.setFakeTextureImage(
        '/i/alpha/player/install/ttmodels/src/fonts/ImpressBT.rgb')
    model = loader.loadModel(modelPath)
    assert model is not None
    TexturePool.clearFakeTextureImage()
    # get the model's zone info
    zoneNum2node = LevelUtil.getZoneNum2Node(model)
    zoneNums = zoneNum2node.keys()

    # what zone entities do we have specs for?
    type2ids = spec.getEntType2ids(spec.getAllEntIds())
    type2ids.setdefault('zone', [])
    zoneEntIds = type2ids['zone']

    def removeZoneEntity(entId, spec=spec, zoneEntIds=zoneEntIds):
        spec.removeEntity(entId)
        zoneEntIds.remove(entId)

    def insertZoneEntity(zoneNum, spec=spec, zoneEntIds=zoneEntIds):
        spec.insertEntity(zoneNum, 'zone', LevelConstants.UberZoneEntId)
        spec.doSetAttrib(zoneNum, 'name', 'zone%s' % zoneNum)
        zoneEntIds.append(zoneNum)

    # prune zone entities that reference zones that no longer exist
    removedZoneNums = []
    for entId in list(zoneEntIds):
        if entId not in zoneNums:
            print 'zone %s no longer exists; removing' % entId
            removeZoneEntity(entId)
            removedZoneNums.append(entId)
            
    # add new zone entities for new zones
    newZoneNums = []
    for zoneNum in zoneNums:
        if zoneNum not in zoneEntIds:
            newZoneNums.append(zoneNum)

            print 'adding new zone entity %s' % zoneNum
            insertZoneEntity(zoneNum)
            # by default, new zone can't see any other zones
            spec.doSetAttrib(zoneNum, 'visibility', [])

    if newZonesGloballyVisible:
        for entId in zoneEntIds:
            visList = list(spec.getEntitySpec(entId)['visibility'])
            visDict = list2dict(visList)
            for zoneNum in newZoneNums:
                visDict[zoneNum] = None
            visList = visDict.keys()
            visList.sort()
            spec.doSetAttrib(entId, 'visibility', visList)

    # make sure none of the zones reference removed zones
    spec.removeZoneReferences(removedZoneNums)
