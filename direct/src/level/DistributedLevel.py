"""DistributedLevel.py: contains the DistributedLevel class"""

from ClockDelta import *
from PythonUtil import Functor, sameElements, list2dict, uniqueElements
import ToontownGlobals
import DistributedObject
import LevelBase
import DirectNotifyGlobal
import EntityCreator

class DistributedLevel(DistributedObject.DistributedObject,
                       LevelBase.LevelBase):
    """DistributedLevel"""
    notify = DirectNotifyGlobal.directNotify.newCategory('DistributedLevel')

    WantVisibility = config.GetBool('level-visibility', 1)
    HideZones = config.GetBool('level-hidezones', 1)

    def __init__(self, cr):
        DistributedObject.DistributedObject.__init__(self, cr)
        LevelBase.LevelBase.__init__(self)

    def generate(self):
        self.notify.debug('generate')
        DistributedObject.DistributedObject.generate(self)

        # this dict stores entity reparents if the parent hasn't been
        # created yet
        self.parent2ChildIds = {}

        # Most (if not all) of the timed entities of levels
        # run on looping intervals that are started once based on
        # the level's start time.
        # This sync request is *NOT* guaranteed to finish by the time
        # the entities get created.
        # We should listen for any and all time-sync events and re-sync
        # all our entities at that time.
        toonbase.tcr.timeManager.synchronize('DistributedLevel.generate')

    # required fields (these ought to be required fields, but
    # the AI level obj doesn't know the data values until it has been
    # generated.)
    def setZoneIds(self, zoneIds):
        self.notify.debug('setZoneIds: %s' % zoneIds)
        self.zoneIds = zoneIds

    def setStartTimestamp(self, timestamp):
        self.notify.debug('setStartTimestamp: %s' % timestamp)
        self.startTime = globalClockDelta.networkToLocalTime(timestamp,bits=32)

    def setScenarioIndex(self, scenarioIndex):
        self.scenarioIndex = scenarioIndex
        # ugly hack: we treat these DC fields as if they were required,
        # and use 'levelAnnounceGenerate()' in place of regular old
        # announceGenerate(). Note that we have to call
        # levelAnnounceGenerate() in the last 'faux-required' DC update
        # handler. If you add another field, move this to the last one.
        self.levelAnnounceGenerate()

    def levelAnnounceGenerate(self):
        pass

    def initializeLevel(self, spec):
        """subclass should call this as soon as it's located its spec data.
        Must be called after obj has been generated."""
        LevelBase.LevelBase.initializeLevel(self, self.doId,
                                            spec, self.scenarioIndex)

        # there should not be any pending reparents left at this point
        assert len(self.parent2ChildIds) == 0
        # make sure the zoneNums from the model match the zoneNums from
        # the zone entities
        assert sameElements(self.zoneNums, self.zoneNum2entId.keys())

        # load stuff

        # fix up the floor collisions for walkable zones
        for zoneNum in self.zoneNums:
            zoneNode = self.zoneNum2node[zoneNum]

            # if this is a walkable zone, fix up the model
            floorColls = zoneNode.findAllMatches('**/+CollisionNode').asList()
            if len(floorColls) > 0:
                # rename the floor collision nodes, and make sure no other
                # nodes under the ZoneNode have that name
                floorCollName = '%s' % zoneNum
                others = zoneNode.findAllMatches(
                    '**/%s' % floorCollName).asList()
                for other in others:
                    other.setName('%s_renamed' % floorCollName)
                for floorColl in floorColls:
                    floorColl.setName(floorCollName)

                # listen for zone enter events from floor collisions
                def handleZoneEnter(collisionEntry,
                                    self=self, zoneNum=zoneNum):
                    # eat the collisionEntry
                    self.toonEnterZone(zoneNum)
                self.accept('enter%s' % floorCollName, handleZoneEnter)

        self.initVisibility()

    def createEntityCreator(self):
        """Create the object that will be used to create Entities.
        Inheritors, override if desired."""
        return EntityCreator.EntityCreator(level=self)

    def setupEntityCreationHandlers(self):
        LevelBase.LevelBase.setupEntityCreationHandlers(self)
        # load up the model ASAP so that we can get zone info out of it
        self.acceptOnce(self.getEntityTypeCreateEvent('levelMgr'),
                        self.handleLevelMgrCreated)

    def removeEntityCreationHandlers(self):
        LevelBase.LevelBase.removeEntityCreationHandlers(self)

    def handleLevelMgrCreated(self):
        # as soon as the levelMgr has been created, load up the model
        # and extract zone info
        self.geom = loader.loadModel(self.modelFilename)

        def findNumberedNodes(baseString, model=self.geom, self=self):
            # finds nodes whose name follows the pattern 'baseString#'
            # where there are no characters after #
            # returns dictionary that maps # to node
            potentialNodes = model.findAllMatches(
                '**/%s*' % baseString).asList()
            num2node = {}
            for potentialNode in potentialNodes:
                name = potentialNode.getName()
                self.notify.debug('potential match for %s: %s' %
                                  (baseString, name))
                try:
                    num = int(name[len(baseString):])
                except:
                    continue
                
                num2node[num] = potentialNode

            return num2node

        # find the zones in the model and fix them up
        self.zoneNum2node = findNumberedNodes('Zone')
        # add the UberZone to the table
        self.zoneNum2node[0] = self.geom

        self.zoneNums = self.zoneNum2node.keys()
        self.zoneNums.sort()
        self.notify.debug('zones: %s' % self.zoneNums)

        # hack in another doorway
        dw = self.geom.attachNewNode('Doorway27')
        dw.setPos(-49.4,86.7,19.26)
        dw.setH(0)

        # find the doorway nodes
        self.doorwayNum2Node = findNumberedNodes('Doorway')

    def announceGenerate(self):
        self.notify.debug('announceGenerate')
        DistributedObject.DistributedObject.announceGenerate(self)

    def disable(self):
        self.notify.debug('disable')
        self.destroyLevel()
        DistributedObject.DistributedObject.disable(self)
        self.ignoreAll()

    def delete(self):
        self.notify.debug('delete')
        DistributedObject.DistributedObject.delete(self)

    def getDoorwayNode(self, doorwayNum):
        # returns node that doors should parent themselves to
        return self.doorwayNum2Node[doorwayNum]

    def getZoneNode(self, zoneNum):
        return self.zoneNum2node[zoneNum]

    def requestReparent(self, entity, parentId):
        assert(entity.entId != parentId)
        if self.entities.has_key(parentId):
            # parent has already been created
            entity.reparentTo(self.entities[parentId].getNodePath())
        else:
            # parent hasn't been created yet; schedule the reparent
            self.notify.debug(
                'entity %s requesting reparent to %s, not yet created' %
                (entity, parentId))

            entId = entity.entId
            entity.reparentTo(hidden)

            # if this parent doesn't already have another child pending,
            # do some setup
            if not self.parent2ChildIds.has_key(parentId):
                self.parent2ChildIds[parentId] = []

                # do the reparent once the parent is initialized
                def doReparent(parentId=parentId, self=self):
                    assert self.parent2ChildIds.has_key(parentId)
                    parent=self.getEntity(parentId)
                    for entId in self.parent2ChildIds[parentId]:
                        entity=self.getEntity(entId)
                        self.notify.debug(
                            'performing pending reparent of %s to %s' %
                            (entity, parent))
                        entity.reparentTo(parent.getNodePath())
                    del self.parent2ChildIds[parentId]
                    
                self.accept(self.getEntityCreateEvent(parentId), doReparent)

            self.parent2ChildIds[parentId].append(entId)

    def showZone(self, zoneNum):
        self.zoneNum2node[zoneNum].show()

    def hideZone(self, zoneNum):
        self.zoneNum2node[zoneNum].hide()

    def setTransparency(self, alpha, zone=None):
        self.geom.setTransparency(1)
        if zone is None:
            node = self.geom
        else:
            node = self.zoneNum2node[zone]
        node.setAlphaScale(alpha)

    def initVisibility(self):
        # start out with every zone visible, since none of the zones have
        # been hidden
        self.curVisibleZoneNums = list2dict(self.zoneNums)
        # the UberZone is always visible, so it's not included in the
        # viz lists
        del self.curVisibleZoneNums[0]
        # we have not entered any zone yet
        self.curZoneNum = None

        # listen for camera-ray/floor collision events
        def handleCameraRayFloorCollision(collEntry, self=self):
            name = collEntry.getIntoNode().getName()
            try:
                zoneNum = int(name)
            except:
                self.notify.warning('Invalid floor collision node: %s' % name)
            else:
                self.camEnterZone(zoneNum)
        self.accept('on-floor', handleCameraRayFloorCollision)

        # if no viz, listen to all the zones
        if not DistributedLevel.WantVisibility:
            self.setVisibility(self.zoneNums)

    def toonEnterZone(self, zoneNum):
        self.notify.debug('toonEnterZone%s' % zoneNum)

    def camEnterZone(self, zoneNum):
        self.notify.debug('camEnterZone%s' % zoneNum)
        self.enterZone(zoneNum)

    def enterZone(self, zoneNum):
        self.notify.debug("entering zone %s" % zoneNum)

        if not DistributedLevel.WantVisibility:
            return
        
        if zoneNum == self.curZoneNum:
            return

        zoneEntId = self.zoneNum2entId[zoneNum]
        zoneSpec = self.entId2spec[zoneEntId]
        # use dicts to efficiently ensure that there are no duplicates
        visibleZoneNums = list2dict([zoneNum])
        visibleZoneNums.update(list2dict(zoneSpec['visibility']))
        
        if DistributedLevel.HideZones:
            # figure out which zones are new and which are going invisible
            # use dicts because it's faster to use dict.has_key(x)
            # than 'x in list'
            addedZoneNums = []
            removedZoneNums = []
            allVZ = dict(visibleZoneNums)
            allVZ.update(self.curVisibleZoneNums)
            for vz,None in allVZ.items():
                new = vz in visibleZoneNums
                old = vz in self.curVisibleZoneNums
                if new and old:
                    continue
                if new:
                    addedZoneNums.append(vz)
                else:
                    removedZoneNums.append(vz)
            # show the new, hide the old
            self.notify.debug('showing zones %s' % addedZoneNums)
            for az in addedZoneNums:
                self.showZone(az)
            self.notify.debug('hiding zones %s' % removedZoneNums)
            for rz in removedZoneNums:
                self.hideZone(rz)

        self.setVisibility(visibleZoneNums.keys())

        self.curZoneNum = zoneNum
        self.curVisibleZoneNums = visibleZoneNums

    def setVisibility(self, vizList):
        # accepts list of visible zone numbers
        # convert the zone numbers into their actual zoneIds
        # always include Toontown and factory uberZones
        factoryUberZone = self.getZoneId(zoneNum=0)
        visibleZoneIds = [ToontownGlobals.UberZone, factoryUberZone]
        for vz in vizList:
            visibleZoneIds.append(self.getZoneId(zoneNum=vz))
        assert(uniqueElements(visibleZoneIds))
        self.notify.debug('new viz list: %s' % visibleZoneIds)

        toonbase.tcr.sendSetZoneMsg(factoryUberZone, visibleZoneIds)

    if __debug__:
        # level editing stuff
        def setAttribChange(self, entId, attribName, valueStr):
            try:
                value = eval(valueStr)
            except Exception, e:
                print ('Exception in %s(%s, %s, %s):\n\t%s' %
                       (lineInfo()[2], entId, attribName, valueStr, e))
                raise e
                
            entity = self.getEntity(entId)
            entity.handleAttribChange(attribName, value)
