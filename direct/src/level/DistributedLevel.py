"""DistributedLevel.py: contains the DistributedLevel class"""

from ClockDelta import *
from PythonUtil import Functor, sameElements, list2dict, uniqueElements
import ToontownGlobals
import DistributedObject
import Level
import LevelConstants
import DirectNotifyGlobal
import EntityCreator
import OnscreenText
import Task
import LevelUtil

class DistributedLevel(DistributedObject.DistributedObject,
                       Level.Level):
    """DistributedLevel"""
    notify = DirectNotifyGlobal.directNotify.newCategory('DistributedLevel')

    WantVisibility = config.GetBool('level-visibility', 1)
    HideZones = config.GetBool('level-hidezones', 1)

    # TODO: move level-model stuff to LevelMgr or FactoryLevelMgr?
    FloorCollPrefix = 'zoneFloor'

    def __init__(self, cr):
        DistributedObject.DistributedObject.__init__(self, cr)
        Level.Level.__init__(self)
        self.lastToonZone = 0
        self.titleColor = (1,1,1,1)
        self.titleText = OnscreenText.OnscreenText(
            "",
            fg = self.titleColor,
            shadow = (0,0,0,1),
            font = ToontownGlobals.getSuitFont(),
            pos = (0,-0.5),
            scale = 0.16,
            drawOrder = 0,
            mayChange = 1,
            )

        self.smallTitleText = OnscreenText.OnscreenText(
            "",
            fg = self.titleColor,
            font = ToontownGlobals.getSuitFont(),
            pos = (0.65,0.9),
            scale = 0.08,
            drawOrder = 0,
            mayChange = 1,
            bg = (.5,.5,.5,.5),
            align = TextNode.ARight,
            )
        self.zonesEnteredList = []

    def generate(self):
        DistributedLevel.notify.debug('generate')
        DistributedObject.DistributedObject.generate(self)

        # this dict stores entity reparents if the parent hasn't been
        # created yet
        self.parent2ChildIds = {}

        # if the AI sends us a full spec, it will be put here
        self.curSpec = None

        # Most (if not all) of the timed entities of levels
        # run on looping intervals that are started once based on
        # the level's start time.
        # This sync request is *NOT* guaranteed to finish by the time
        # the entities get created.
        # We should listen for any and all time-sync events and re-sync
        # all our entities at that time.
        toonbase.tcr.timeManager.synchronize('DistributedLevel.generate')

        # add factory menu to SpeedChat
        toonbase.localToon.chatMgr.chatInputSpeedChat.addFactoryMenu()

    # the real required fields
    def setLevelZoneId(self, zoneId):
        # this is the zone that the level is in; we should listen to this
        # zone the entire time we're in here
        self.levelZone = zoneId

    # "required" fields (these ought to be required fields, but
    # the AI level obj doesn't know the data values until it has been
    # generated.)
    def setZoneIds(self, zoneIds):
        DistributedLevel.notify.debug('setZoneIds: %s' % zoneIds)
        self.zoneIds = zoneIds

    def setStartTimestamp(self, timestamp):
        DistributedLevel.notify.debug('setStartTimestamp: %s' % timestamp)
        self.startTime = globalClockDelta.networkToLocalTime(timestamp,bits=32)

    def setScenarioIndex(self, scenarioIndex):
        self.scenarioIndex = scenarioIndex

        # ugly hack: we treat a few DC fields as if they were required,
        # and use 'levelAnnounceGenerate()' in place of regular old
        # announceGenerate(). Note that we have to call
        # gotAllRequired() in the last 'faux-required' DC update
        # handler. If you add another field, move this to the last one.
        self.privGotAllRequired()

    def privGotAllRequired(self):
        self.levelAnnounceGenerate()
    def levelAnnounceGenerate(self):
        pass

    def initializeLevel(self, levelSpec):
        """subclass should call this as soon as it's located its level spec.
        Must be called after obj has been generated."""
        if __debug__:
            # if we're in debug, give the server the opportunity to send us
            # a full spec
            self.candidateSpec = levelSpec
            self.sendUpdate('requestCurrentLevelSpec',
                            [hash(levelSpec),
                             hash(levelSpec.entTypeReg)])
        else:
            self.privGotSpec(levelSpec)

    if __debug__:
        def setSpecDeny(self, reason):
            DistributedLevel.notify.error(reason)
            
        def setSpecSenderDoId(self, doId):
            DistributedLevel.notify.debug('setSpecSenderDoId: %s' % doId)
            blobSender = toonbase.tcr.doId2do[doId]

            def setSpecBlob(specBlob, blobSender=blobSender, self=self):
                blobSender.sendAck()
                from LevelSpec import LevelSpec
                spec = eval(specBlob)
                if spec is None:
                    spec = self.candidateSpec
                del self.candidateSpec
                self.privGotSpec(spec)

            if blobSender.isComplete():
                setSpecBlob(blobSender.getBlob())
            else:
                evtName = self.uniqueName('specDone')
                blobSender.setDoneEvent(evtName)
                self.acceptOnce(evtName, setSpecBlob)

    def privGotSpec(self, levelSpec):
        Level.Level.initializeLevel(self, self.doId, levelSpec,
                                    self.scenarioIndex)

        # all of the local entities have been created now.
        # TODO: have any of the distributed entities been created at this point?

        # there should not be any pending reparents left at this point
        # TODO: is it possible for a local entity to be parented to a
        # distributed entity? I think so!
        assert len(self.parent2ChildIds) == 0
        # make sure the zoneNums from the model match the zoneNums from
        # the zone entities
        modelZoneNums = self.zoneNums
        entityZoneNums = self.zoneNum2entId.keys()
        if not sameElements(modelZoneNums, entityZoneNums):
            DistributedLevel.notify.error(
                'model zone nums (%s) do not match entity zone nums (%s)' %
                (modelZoneNums, entityZoneNums))

        # load stuff
        self.initVisibility()

    def createEntityCreator(self):
        """Create the object that will be used to create Entities.
        Inheritors, override if desired."""
        return EntityCreator.EntityCreator(level=self)

    def onEntityTypePostCreate(self, entType):
        """listen for certain entity types to be created"""
        Level.Level.onEntityTypePostCreate(self, entType)
        # NOTE: these handlers are private in order to avoid overriding
        # similar handlers in base classes
        if entType == 'levelMgr':
            self.__handleLevelMgrCreated()

    def __handleLevelMgrCreated(self):
        # as soon as the levelMgr has been created, load up the model
        # and extract zone info. We need to do this before any entities
        # get parented to the level!
        levelMgr = self.getEntity(LevelConstants.LevelMgrEntId)
        self.geom = levelMgr.geom

        # find the zones in the model and fix them up
        self.zoneNum2node = LevelUtil.getZoneNum2Node(self.geom)

        self.zoneNums = self.zoneNum2node.keys()
        self.zoneNums.sort()
        DistributedLevel.notify.debug('zones: %s' % self.zoneNums)

        # fix up the floor collisions for walkable zones *before*
        # any entities get put under the model
        for zoneNum,zoneNode in self.zoneNum2node.items():
            # if this is a walkable zone, fix up the model
            allColls = zoneNode.findAllMatches('**/+CollisionNode').asList()
            # which of them, if any, are floors?
            floorColls = []
            for coll in allColls:
                bitmask = coll.node().getIntoCollideMask()
                if not (bitmask & ToontownGlobals.FloorBitmask).isZero():
                    floorColls.append(coll)
            if len(floorColls) > 0:
                # rename the floor collision nodes, and make sure no other
                # nodes under the ZoneNode have that name
                floorCollName = '%s%s' % (DistributedLevel.FloorCollPrefix,
                                          zoneNum)
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

        # hack in another doorway
        dw = self.geom.attachNewNode('Doorway27')
        dw.setPos(-49.4,86.7,19.26)
        dw.setH(0)

        # find the doorway nodes
        # this is going to go away soon.
        def findNumberedNodes(baseString, model=self.geom, self=self):
            # finds nodes whose name follows the pattern 'baseString#'
            # where there are no characters after #
            # returns dictionary that maps # to node
            potentialNodes = model.findAllMatches(
                '**/%s*' % baseString).asList()
            num2node = {}
            for potentialNode in potentialNodes:
                name = potentialNode.getName()
                DistributedLevel.notify.debug('potential match for %s: %s' %
                                  (baseString, name))
                try:
                    num = int(name[len(baseString):])
                except:
                    continue
                
                num2node[num] = potentialNode

            return num2node

        self.doorwayNum2Node = findNumberedNodes('Doorway')

    def announceGenerate(self):
        DistributedLevel.notify.debug('announceGenerate')
        DistributedObject.DistributedObject.announceGenerate(self)

    def disable(self):
        DistributedLevel.notify.debug('disable')

        # geom is owned by the levelMgr
        if hasattr(self, 'geom'):
            del self.geom

        self.destroyLevel()
        DistributedObject.DistributedObject.disable(self)
        self.ignoreAll()

        # NOTE:  this should be moved to FactoryInterior
        if self.smallTitleText:
            self.smallTitleText.cleanup()
            self.smallTitleText = None
        if self.titleText:
            self.titleText.cleanup()
            self.titleText = None
        self.zonesEnteredList = []
        
        # NOTE: this should be moved to ZoneEntity.disable
        toonbase.localToon.chatMgr.chatInputSpeedChat.removeFactoryMenu()

    def delete(self):
        DistributedLevel.notify.debug('delete')
        DistributedObject.DistributedObject.delete(self)
        # remove factory menu to SpeedChat
        toonbase.localToon.chatMgr.chatInputSpeedChat.removeFactoryMenu()

    def getDoorwayNode(self, doorwayNum):
        # returns node that doors should parent themselves to
        return self.doorwayNum2Node[doorwayNum]

    def getZoneNode(self, zoneNum):
        return self.zoneNum2node[zoneNum]

    def requestReparent(self, entity, parentId):
        assert(entity.entId != parentId)
        parent = self.getEntity(parentId)
        if parent is not None:
            # parent has already been created
            entity.reparentTo(parent.getNodePath())
        else:
            # parent hasn't been created yet; schedule the reparent
            DistributedLevel.notify.debug(
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
                        DistributedLevel.notify.debug(
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
        # zones' viz lists
        del self.curVisibleZoneNums[0]
        # we have not entered any zone yet
        self.curZoneNum = None

        # listen for camera-ray/floor collision events
        def handleCameraRayFloorCollision(collEntry, self=self):
            name = collEntry.getIntoNode().getName()
            prefixLen = len(DistributedLevel.FloorCollPrefix)
            if (name[:prefixLen] == DistributedLevel.FloorCollPrefix):
                try:
                    zoneNum = int(name[prefixLen:])
                except:
                    DistributedLevel.notify.debug(
                        'Invalid zone floor collision node: %s'
                        % name)
                else:
                    self.camEnterZone(zoneNum)
        self.accept('on-floor', handleCameraRayFloorCollision)

        # if no viz, listen to all the zones
        if not DistributedLevel.WantVisibility:
            zoneNums = list(self.zoneNums)
            zoneNums.remove(LevelConstants.UberZoneNum)
            self.setVisibility(zoneNums)

    def toonEnterZone(self, zoneNum):
        DistributedLevel.notify.debug('toonEnterZone%s' % zoneNum)
        if zoneNum != self.lastToonZone:
            self.lastToonZone = zoneNum
            print "made zone transition to %s" % zoneNum
            messenger.send("factoryZoneChanged", [zoneNum])
            self.smallTitleText.hide()
            self.spawnTitleText()
            
    def camEnterZone(self, zoneNum):
        DistributedLevel.notify.debug('camEnterZone%s' % zoneNum)
        self.enterZone(zoneNum)

    def enterZone(self, zoneNum):
        DistributedLevel.notify.debug("entering zone %s" % zoneNum)

        if not DistributedLevel.WantVisibility:
            return
        
        if zoneNum == self.curZoneNum:
            return

        if zoneNum not in self.zoneNum2entId:
            DistributedLevel.notify.error(
                'no ZoneEntity for this zone (%s)!!' % zoneNum)
            return

        zoneEntId = self.zoneNum2entId[zoneNum]
        zoneSpec = self.levelSpec.getEntitySpec(zoneEntId)
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
            DistributedLevel.notify.debug('showing zones %s' % addedZoneNums)
            for az in addedZoneNums:
                self.showZone(az)
            DistributedLevel.notify.debug('hiding zones %s' % removedZoneNums)
            for rz in removedZoneNums:
                self.hideZone(rz)

        self.setVisibility(visibleZoneNums.keys())

        self.curZoneNum = zoneNum
        self.curVisibleZoneNums = visibleZoneNums

    def setVisibility(self, vizList):
        # accepts list of visible zone numbers
        # convert the zone numbers into their actual zoneIds
        # always include Toontown and factory uberZones
        uberZone = self.getZoneId(zoneNum=LevelConstants.UberZoneNum)
        # the level itself is in the 'level zone'
        visibleZoneIds = [ToontownGlobals.UberZone, self.levelZone, uberZone]
        for vz in vizList:
            visibleZoneIds.append(self.getZoneId(zoneNum=vz))
        assert(uniqueElements(visibleZoneIds))
        DistributedLevel.notify.debug('new viz list: %s' % visibleZoneIds)

        toonbase.tcr.sendSetZoneMsg(self.levelZone, visibleZoneIds)

    if __debug__:
        # level editing stuff
        def setAttribChange(self, entId, attribName, valueStr, username):
            """every time the spec is edited, we get this message
            from the AI"""
            value = eval(valueStr)
            self.levelSpec.setAttribChange(entId, attribName, value, username)

    def spawnTitleText(self):
        def getDescription(zoneId, self=self):
            entId = self.zoneNum2entId.get(zoneId)
            if entId:
                ent = self.entities.get(entId)
                if ent and hasattr(ent, 'description'):
                    return ent.description
            return None

        description = getDescription(self.lastToonZone)
        if description and description != '':
            taskMgr.remove("titleText")
            self.smallTitleText.setText(description)
            self.titleText.setText(description)
            self.titleText.setColor(Vec4(*self.titleColor))
            self.titleText.setFg(self.titleColor)

            # Only show the big title once per session.
            # If we've already seen it, just show the small title

            titleSeq = None
            if not self.lastToonZone in self.zonesEnteredList:
                self.zonesEnteredList.append(self.lastToonZone)
                titleSeq = Task.sequence(
                    Task.Task(self.hideSmallTitleTextTask),
                    Task.Task(self.showTitleTextTask),
                    Task.pause(0.1),
                    Task.pause(6.0),
                    self.titleText.lerpColor(Vec4(self.titleColor[0],
                                                  self.titleColor[1],
                                                  self.titleColor[2],
                                                  self.titleColor[3]),
                                             Vec4(self.titleColor[0],
                                                  self.titleColor[1],
                                                  self.titleColor[2],
                                                  0.0),
                                             0.5),
                    )
            smallTitleSeq = Task.sequence(Task.Task(self.hideTitleTextTask),
                                          Task.Task(self.showSmallTitleTask),
                                          Task.Task(self.showSmallTitleTask))
            if titleSeq:
                seq = Task.sequence(titleSeq, smallTitleSeq)
            else:
                seq = smallTitleSeq
            taskMgr.add(seq, "titleText")
        
    def showTitleTextTask(self, task):
        assert(DistributedLevel.notify.debug("hideTitleTextTask()"))
        self.titleText.show()
        return Task.done

    def hideTitleTextTask(self, task):
        assert(DistributedLevel.notify.debug("hideTitleTextTask()"))
        self.titleText.hide()
        return Task.done

    def showSmallTitleTask(self, task):
        # make sure large title is hidden
        self.titleText.hide()
        # show the small title
        self.smallTitleText.show()
        return Task.done
    
    def hideSmallTitleTextTask(self, task):
        assert(DistributedLevel.notify.debug("hideTitleTextTask()"))
        self.smallTitleText.hide()
        return Task.done
