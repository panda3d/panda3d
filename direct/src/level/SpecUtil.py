"""SpecUtil module: contains utility functions for creating and managing level specs"""

from ShowBaseGlobal import *
import LevelSpec
import LevelConstants
import LevelUtil
from PythonUtil import list2dict
import EntityTypes
import types

def makeNewSpec(filename, modelPath, entTypeModule=EntityTypes):
    """call this to create a new level spec for the level model at 'modelPath'.
    Spec will be saved as 'filename'"""
    spec = LevelSpec.LevelSpec()
    privUpdateSpec(spec, modelPath, entTypeModule)
    spec.saveToDisk(filename, makeBackup=0)

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
SpecUtil.updateSpec(SellbotLegFactorySpec, FactoryEntityTypes, newModelPath)
"""

def updateSpec(specModule, entTypeModule=EntityTypes, modelPath=None):
    """Call this to update an existing levelSpec to work with a new level
    model. If the level model has a new path, pass it in as 'modelPath'.
    specModule must be a Python module"""
    spec = LevelSpec.LevelSpec(specModule)
    privUpdateSpec(spec, modelPath, entTypeModule)
    spec.saveToDisk()

def privUpdateSpec(spec, modelPath, entTypeModule):
    """internal: take a spec and update it to match its level model"""
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
    TexturePool.clearFakeTextureImage()
    # get the model's zone info
    modelZoneNum2node = LevelUtil.getZoneNum2Node(model)
    modelZoneNums = modelZoneNum2node.keys()

    # what zone entities do we have specs for?
    type2ids = spec.getEntType2ids(spec.getAllEntIds())
    type2ids.setdefault('zone', [])
    zoneEntIds = type2ids['zone']

    def removeZoneEntity(entId, spec=spec, zoneEntIds=zoneEntIds):
        spec.removeEntity(entId)
        zoneEntIds.remove(entId)

    def insertZoneEntity(entId, modelZoneNum, spec=spec, zoneEntIds=zoneEntIds):
        spec.insertEntity(entId, 'zone', LevelConstants.UberZoneEntId)
        spec.doSetAttrib(entId, 'name', 'zone%s' % modelZoneNum)
        spec.doSetAttrib(entId, 'modelZoneNum', modelZoneNum)
        zoneEntIds.append(entId)

    # create dict of zoneNum -> entId
    zoneNum2entId = {}
    for entId in list(zoneEntIds):
        zoneNum = spec.getEntitySpec(entId)['modelZoneNum']
        if zoneNum in zoneNum2entId:
            print ('multiple zone entities reference zoneNum %s; removing '
                   'entity %s' % (zoneNum, entId))
            removeZoneEntity(entId)
            continue
        zoneNum2entId[zoneNum] = entId

    # prune zone entities that reference zones that no longer exist
    removedZoneNums = []
    for entId in list(zoneEntIds):
        zoneNum = spec.getEntitySpec(entId)['modelZoneNum']
        if zoneNum not in modelZoneNums:
            print 'zone %s no longer exists; removing entity %s' % (
                zoneNum, entId)
            removeZoneEntity(entId)
            del zoneNum2entId[zoneNum]
            removedZoneNums.append(zoneNum)
            
    # add new zone entities for new zones
    newZoneNums = []
    for zoneNum in modelZoneNums:
        if zoneNum not in zoneNum2entId:
            newZoneNums.append(zoneNum)

            entIdDict = list2dict(spec.getAllEntIds())
            entId = LevelConstants.ZoneEntIdStart
            # we could do better than linear
            while entId in entIdDict:
                entId += 1

            print 'adding entity %s for new zone %s' % (entId, zoneNum)
            insertZoneEntity(entId, zoneNum)
            # by default, new zone can't see any other zones
            spec.doSetAttrib(entId, 'visibility', [])

    # make sure none of the zones reference removed zones
    # TODO: prune from other zoneList attribs
    for entId in zoneEntIds:
        visList = spec.getEntitySpec(entId)['visibility']
        visDict = list2dict(visList)
        for zoneNum in removedZoneNums:
            if zoneNum in visDict:
                del visDict[zoneNum]
        visList = visDict.keys()
        visList.sort()
        spec.doSetAttrib(entId, 'visibility', visList)
