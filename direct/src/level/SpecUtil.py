"""SpecUtil module: contains utility functions for creating and managing level specs"""

import LevelSpec
import LevelConstants
import LevelUtil
from PythonUtil import list2dict

def makeNewSpec(filename, modelPath):
    """call this to create a new level spec for the level model at 'modelPath'.
    Spec will be saved as 'filename'"""
    spec = LevelSpec.LevelSpec()
    privUpdateSpec(spec, modelPath)
    spec.saveToDisk(filename, makeBackup=0)

def updateSpec(specModule, modelPath=None):
    """Call this to update an existing levelSpec to work with a new level
    model. If the level model has a new path, pass it in as 'modelPath'.
    specModule must be a Python module"""
    spec = LevelSpec.LevelSpec(specModule)
    privUpdateSpec(spec, modelPath)
    spec.saveToDisk()

def privUpdateSpec(spec, modelPath=None):
    """internal: take a spec and update it to match its level model"""
    if modelPath is None:
        modelPath = spec.getEntitySpec(
            LevelConstants.LevelMgrEntId)['modelFilename']
    else:
        spec.doSetAttrib(LevelConstants.LevelMgrEntId,
                         'modelFilename', modelPath)

    # load the model
    model = loader.loadModel(modelPath)
    # get the model's zone info
    modelZoneNum2node = LevelUtil.getZoneNum2Node(model)
    modelZoneNums = modelZoneNum2node.keys()

    # what zone entities do we have specs for?
    type2ids = spec.getEntType2ids(spec.getAllEntIds())
    type2ids.setdefault('zone', [])
    zoneEntIds = type2ids['zone']
    zoneEntId2spec = list2dict(zoneEntIds)
    for entId in zoneEntIds:
        zoneEntId2spec[entId] = spec.getEntitySpec(entId)

    def removeZoneEntity(entId, spec=spec, zoneEntIds=zoneEntIds,
                         zoneEntId2spec=zoneEntId2spec):
        spec.removeEntity(entId)
        zoneEntIds.remove(entId)
        del zoneEntId2spec[entId]

    # create dict of zoneNum -> entId
    zoneNum2entId = {}
    for entId in list(zoneEntIds):
        zoneNum = zoneEntId2spec[entId]['modelZoneNum']
        if zoneNum in zoneNum2entId:
            print ('multiple zone entities reference zoneNum %s; removing '
                   'entity %s' % (zoneNum, entId))
            removeZoneEntity(entId)
            continue
        zoneNum2entId[zoneNum] = entId

    # prune zone entities that reference zones that no longer exist
    for entId in list(zoneEntIds):
        zoneNum = zoneEntId2spec[entId]['modelZoneNum']
        if zoneNum not in modelZoneNums:
            print 'zone %s no longer exists; removing entity %s' % (
                zoneNum, entId)
            removeZoneEntity(entId)
            del zoneNum2entId[zoneNum]
            
    # add new zone entities for new zones
    for zoneNum in modelZoneNums:
        if zoneNum not in zoneNum2entId:
            entIdDict = list2dict(spec.getAllEntIds())
            entId = LevelConstants.ZoneEntIdStart
            # we could do better than linear
            while entId in entIdDict:
                entId += 1

            print 'adding entity %s for new zone %s' % (entId, zoneNum)
            spec.insertEntity(entId, 'zone', LevelConstants.UberZoneEntId)
            spec.doSetAttrib(entId, 'name', 'zone%s' % zoneNum)
            spec.doSetAttrib(entId, 'modelZoneNum', zoneNum)
