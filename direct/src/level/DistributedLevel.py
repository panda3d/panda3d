"""DistributedLevel.py: contains the DistributedLevel class"""

from ClockDelta import *
from PandaModules import *
from PythonUtil import Functor, sameElements, list2dict, uniqueElements
from IntervalGlobal import *
from ToontownMsgTypes import *
import ToontownGlobals
import DistributedObject
import Level
import LevelConstants
import DirectNotifyGlobal
import EntityCreator
import OnscreenText
import Task
import LevelUtil
import FactoryCameraViews

class DistributedLevel(DistributedObject.DistributedObject,
                       Level.Level):
    """DistributedLevel"""
    notify = DirectNotifyGlobal.directNotify.newCategory('DistributedLevel')

    WantVisibility = config.GetBool('level-visibility', 1)
    HideZones = config.GetBool('level-hidezones', 1)
    # set this to true to get all distrib objs when showing hidden zones
    ColorZonesAllDOs = 0

    # TODO: move level-model stuff to LevelMgr or FactoryLevelMgr?
    FloorCollPrefix = 'zoneFloor'

    OuchTaskName = 'ouchTask'
    VisChangeTaskName = 'visChange'

    def __init__(self, cr):
        DistributedObject.DistributedObject.__init__(self, cr)
        Level.Level.__init__(self)
        self.lastToonZone = None
        self.lastCamZone = 0
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
        self.fColorZones = 0
        # we use these to track setZone requests
        self.setZonesRequested = 0
        self.setZonesReceived = 0

    def generate(self):
        DistributedLevel.notify.debug('generate')
        DistributedObject.DistributedObject.generate(self)

        # this dict stores entity reparents if the parent hasn't been
        # created yet
        self.parent2pendingChildren = {}

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

        # add special camera views
        self.factoryViews = FactoryCameraViews.FactoryCameraViews(self)


    # the real required fields
    def setLevelZoneId(self, zoneId):
        # this is the zone that the level is in; we should listen to this
        # zone the entire time we're in here
        self.levelZone = zoneId

    def setPlayerIds(self, avIdList):
        self.avIdList = avIdList
        assert toonbase.localToon.doId in self.avIdList

    def setEntranceId(self, entranceId):
        self.entranceId = entranceId

    def getEntranceId(self):
        return self.entranceId

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
        if __dev__:
            # if we're in dev, give the server the opportunity to send us
            # a full spec
            self.candidateSpec = levelSpec
            self.sendUpdate('requestCurrentLevelSpec',
                            [hash(levelSpec),
                             levelSpec.entTypeReg.getHashStr()])
        else:
            self.privGotSpec(levelSpec)

    if __dev__:
        def reportModelSpecSyncError(self, msg):
            DistributedLevel.notify.error(
                '%s\n'
                '\n'
                'your spec does not match the level model\n'
                'use SpecUtil.updateSpec, then restart your AI and client' %
                (msg))

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
        assert len(self.parent2pendingChildren) == 0
        # make sure the zoneNums from the model match the zoneNums from
        # the zone entities
        modelZoneNums = self.zoneNums
        entityZoneNums = self.zoneNum2entId.keys()
        if not sameElements(modelZoneNums, entityZoneNums):
            self.reportModelSpecSyncError(
                'model zone nums (%s) do not match entity zone nums (%s)' %
                (modelZoneNums, entityZoneNums))

        # load stuff
        self.initVisibility()
        self.placeLocalToon()

    def announceLeaving(self):
        """call this just before leaving the level; this may result in
        the factory being destroyed on the AI"""
        DistributedLevel.notify.info('announceLeaving')
        self.doneBarrier()

    def placeLocalToon(self):
        # the entrancePoint entities register themselves with us
        if self.entranceId not in self.entranceId2entity:
            self.notify.warning('unknown entranceId %s' % self.entranceId)
            toonbase.localToon.setPos(0,0,0)
        else:
            epEnt = self.entranceId2entity[self.entranceId]
            epEnt.placeToon(toonbase.localToon,
                            self.avIdList.index(toonbase.localToon.doId),
                            len(self.avIdList))
            # kickstart the visibility
            firstZoneEnt = self.getEntity(epEnt.getZoneEntId())
            self.enterZone(firstZoneEnt.getZoneNum())

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
            # don't do this to the uberzone
            if zoneNum == LevelConstants.UberZoneNum:
                continue
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
                    self.toonEnterZone(zoneNum)
                    floorNode = collisionEntry.getIntoNode()
                    if floorNode.hasTag('ouch'):
                        ouchLevel = int(floorNode.getTag('ouch'))
                        self.startOuch(ouchLevel)
                self.accept('enter%s' % floorCollName, handleZoneEnter)

                # also listen for zone exit events for the sake of the
                # ouch system
                def handleZoneExit(collisionEntry,
                                   self=self, zoneNum=zoneNum):
                    floorNode = collisionEntry.getIntoNode()
                    if floorNode.hasTag('ouch'):
                        self.stopOuch()
                self.accept('exit%s' % floorCollName, handleZoneExit)

    def announceGenerate(self):
        DistributedLevel.notify.debug('announceGenerate')
        DistributedObject.DistributedObject.announceGenerate(self)

    def disable(self):
        DistributedLevel.notify.debug('disable')

        # geom is owned by the levelMgr
        if hasattr(self, 'geom'):
            del self.geom

        self.shutdownVisibility()
        self.destroyLevel()
        self.ignoreAll()

        # NOTE:  this should be moved to FactoryInterior
        taskMgr.remove(self.uniqueName("titleText"))
        if self.smallTitleText:
            self.smallTitleText.cleanup()
            self.smallTitleText = None
        if self.titleText:
            self.titleText.cleanup()
            self.titleText = None
        self.zonesEnteredList = []

        DistributedObject.DistributedObject.disable(self)

    def delete(self):
        DistributedLevel.notify.debug('delete')
        DistributedObject.DistributedObject.delete(self)
        # remove factory menu to SpeedChat
        toonbase.localToon.chatMgr.chatInputSpeedChat.removeFactoryMenu()
        # remove special camera views
        del self.factoryViews
        # make sure the ouch task is stopped
        self.stopOuch()
        
    def getZoneNode(self, zoneNum):
        return self.zoneNum2node.get(zoneNum)

    def requestReparent(self, entity, parentId):
        if __debug__:
            # some things (like cogs) are not actually entities yet;
            # they don't have an entId. Big deal, let it go through.
            if hasattr(entity, 'entId'):
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

            entity.reparentTo(hidden)

            # if this parent doesn't already have another child pending,
            # do some setup
            if not self.parent2pendingChildren.has_key(parentId):
                self.parent2pendingChildren[parentId] = []

                # do the reparent(s) once the parent is initialized
                def doReparent(parentId=parentId, self=self):
                    assert self.parent2pendingChildren.has_key(parentId)
                    parent=self.getEntity(parentId)
                    for child in self.parent2pendingChildren[parentId]:
                        DistributedLevel.notify.debug(
                            'performing pending reparent of %s to %s' %
                            (child, parent))
                        child.reparentTo(parent.getNodePath())
                    del self.parent2pendingChildren[parentId]
                    self.ignore(self.getEntityCreateEvent(parentId))
                    
                self.accept(self.getEntityCreateEvent(parentId), doReparent)

            self.parent2pendingChildren[parentId].append(entity)
    
    def showZone(self, zoneNum):
        zone = self.zoneNum2node[zoneNum]
        zone.unstash()
        zone.clearColor()

    def setColorZones(self, fColorZones):
        self.fColorZones = fColorZones
        self.resetVisibility()

    def getColorZones(self):
        return self.fColorZones

    def hideZone(self, zoneNum):
        zone = self.zoneNum2node[zoneNum]
        if self.fColorZones:
            zone.unstash()
            zone.setColor(1,0,0)
        else:
            zone.stash()

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

        self.visChangedThisFrame = 0

        # listen for camera-ray/floor collision events
        def handleCameraRayFloorCollision(collEntry, self=self):
            name = collEntry.getIntoNode().getName()
            print 'camera floor ray collided with: %s' % name
            prefixLen = len(DistributedLevel.FloorCollPrefix)
            if (name[:prefixLen] == DistributedLevel.FloorCollPrefix):
                try:
                    zoneNum = int(name[prefixLen:])
                except:
                    DistributedLevel.notify.warning(
                        'Invalid zone floor collision node: %s'
                        % name)
                else:
                    self.camEnterZone(zoneNum)
        self.accept('on-floor', handleCameraRayFloorCollision)

        # register our datagram handler to listen for setZone msgs
        self.oldTcrHandler = toonbase.tcr.handler
        toonbase.tcr.handler = self.handleDatagram

        # if no viz, listen to all the zones
        if not DistributedLevel.WantVisibility:
            zoneNums = list(self.zoneNums)
            zoneNums.remove(LevelConstants.UberZoneNum)
            self.setVisibility(zoneNums)

        # send out any zone changes at the end of the frame, just before
        # rendering
        taskMgr.add(self.visChangeTask,
                    self.uniqueName(DistributedLevel.VisChangeTaskName),
                    priority=49)

    def shutdownVisibility(self):
        taskMgr.remove(self.uniqueName(DistributedLevel.VisChangeTaskName))

        if toonbase.tcr.handler == self.handleDatagram:
            toonbase.tcr.handler = self.oldTcrHandler
        del self.oldTcrHandler

    def getSetZoneCompleteEvent(self, num):
        return self.uniqueName('setZoneComplete-%s' % num)

    def getNextSetZoneCompleteEvent(self):
        return self.uniqueName('setZoneComplete-%s' % self.setZonesRequested)

    def handleDatagram(self, msgType, di):
        if msgType == CLIENT_DONE_SET_ZONE_RESP:
            # snoop to see what zone we're talking about
            di2 = DatagramIterator(di)
            zone = di2.getUint32()
            if zone != self.levelZone:
                self.notify.warning('got setZoneComplete for unknown zone %s' %
                                    zone)
            else:
                self.notify.info('setZone #%s complete' % self.setZonesReceived)
                messenger.send(self.getSetZoneCompleteEvent(
                    self.setZonesReceived))
                self.setZonesReceived += 1
            
        if self.oldTcrHandler is None:
            toonbase.tcr.handleUnexpectedMsgType(msgType, di)
        else:
            self.oldTcrHandler(msgType, di)

    def toonEnterZone(self, zoneNum, ouchLevel=None):
        """
        zoneNum is an int.
        ouchLevel is a ??.
        
        The avatar (and not necessarily the camera) has entered
        a zone.
        See camEnterZone()
        """
        DistributedLevel.notify.info('toonEnterZone%s' % zoneNum)

        if zoneNum != self.lastToonZone:
            self.lastToonZone = zoneNum
            print "toon is standing in zone %s" % zoneNum
            messenger.send("factoryZoneChanged", [zoneNum])

    def camEnterZone(self, zoneNum):
        """
        zoneNum is an int.
        
        The camera (and not necessarily the avatar) has entered
        a zone.
        See toonEnterZone()
        """
        DistributedLevel.notify.info('camEnterZone%s' % zoneNum)
        self.enterZone(zoneNum)

        if zoneNum != self.lastCamZone:
            self.lastCamZone = zoneNum
            self.smallTitleText.hide()
            self.spawnTitleText()

    def lockVisibility(self, zoneNum=None, zoneId=None):
        """call this to lock the visibility to a particular zone
        pass in either network zoneId or model zoneNum

        this was added for battles in the HQ factories; if you engage a suit
        in zone A with your camera in zone B, and you don't call this func,
        your client will remain in zone B. If there's a door between A and B,
        and it closes, zone B might disappear, along with the suit and the
        battle objects.
        """
        assert (zoneNum is None) or (zoneId is None)
        assert not ((zoneNum is None) and (zoneId is None))
        if zoneId is not None:
            zoneNum = self.getZoneNumFromId(zoneId)

        self.notify.info('lockVisibility to zoneNum %s' % zoneNum)
        self.lockVizZone = zoneNum
        self.enterZone(self.lockVizZone)

    def unlockVisibility(self):
        """release the visibility lock"""
        self.notify.info('unlockVisibility')
        if not hasattr(self, 'lockVizZone'):
            self.notify.warning('visibility already unlocked')
        else:
            del self.lockVizZone
            self.updateVisibility()
            

    def enterZone(self, zoneNum):
        DistributedLevel.notify.info("entering zone %s" % zoneNum)

        if not DistributedLevel.WantVisibility:
            return
        
        if zoneNum == self.curZoneNum:
            return

        if zoneNum not in self.zoneNum2entId:
            DistributedLevel.notify.error(
                'no ZoneEntity for this zone (%s)!!' % zoneNum)

        self.updateVisibility(zoneNum)

    def updateVisibility(self, zoneNum=None):
        """update the visibility assuming that we're in the specified
        zone; don't check to see if it's the zone we're already in"""
        #print 'updateVisibility %s' % globalClock.getFrameCount()
        if zoneNum is None:
            zoneNum = self.curZoneNum
        if hasattr(self, 'lockVizZone'):
            zoneNum = self.lockVizZone
            
        zoneEntId = self.zoneNum2entId[zoneNum]
        zoneEnt = self.getEntity(zoneEntId)
        # use dicts to efficiently ensure that there are no duplicates
        visibleZoneNums = list2dict([zoneNum])
        visibleZoneNums.update(list2dict(zoneEnt.getVisibleZoneNums()))

        if not __debug__:
            # HACK
            # make sure that the visibility list includes the zone that the toon
            # is standing in
            if self.lastToonZone not in visibleZoneNums:
                # make sure there IS a last zone
                if self.lastToonZone is not None:
                    self.notify.warning(
                        'adding zoneNum %s to visibility list '
                        'because toon is standing in that zone!' %
                        self.lastToonZone)
                    visibleZoneNums.update(list2dict([self.lastToonZone]))

        # we should not have the uberZone in the list at this point
        assert not 0 in visibleZoneNums
        
        if DistributedLevel.HideZones:
            # figure out which zones are new and which are going invisible
            # use dicts because it's faster to use dict.has_key(x)
            # than 'x in list'
            addedZoneNums = []
            removedZoneNums = []
            allVZ = dict(visibleZoneNums)
            allVZ.update(self.curVisibleZoneNums)
            for vz,dummy in allVZ.items():
                new = vz in visibleZoneNums
                old = vz in self.curVisibleZoneNums
                if new and old:
                    continue
                if new:
                    addedZoneNums.append(vz)
                else:
                    removedZoneNums.append(vz)
            # show the new, hide the old
            DistributedLevel.notify.info('showing zones %s' % addedZoneNums)
            for az in addedZoneNums:
                self.showZone(az)
            DistributedLevel.notify.info('hiding zones %s' % removedZoneNums)
            for rz in removedZoneNums:
                self.hideZone(rz)

        self.setVisibility(visibleZoneNums.keys())
        self.setZonesRequested += 1

        self.curZoneNum = zoneNum
        self.curVisibleZoneNums = visibleZoneNums

    def setVisibility(self, vizList):
        """
        vizList is a list of visible zone numbers.
        """
        # if we're showing all zones, get all the DOs
        if self.fColorZones and DistributedLevel.ColorZonesAllDOs:
            vizList = list(self.zoneNums)
            vizList.remove(LevelConstants.UberZoneNum)
        # convert the zone numbers into their actual zoneIds
        # always include Toontown and factory uberZones
        uberZone = self.getZoneId(zoneNum=LevelConstants.UberZoneNum)
        # the level itself is in the 'level zone'
        visibleZoneIds = [ToontownGlobals.UberZone, self.levelZone, uberZone]
        for vz in vizList:
            visibleZoneIds.append(self.getZoneId(zoneNum=vz))
        assert(uniqueElements(visibleZoneIds))
        DistributedLevel.notify.info('new viz list: %s' % visibleZoneIds)

        toonbase.tcr.sendSetZoneMsg(self.levelZone, visibleZoneIds)

    def resetVisibility(self):
        # start out with every zone visible, since none of the zones have
        # been hidden
        self.curVisibleZoneNums = list2dict(self.zoneNums)
        # the UberZone is always visible, so it's not included in the
        # zones' viz lists
        del self.curVisibleZoneNums[0]
        # Make sure every zone is visible
        for vz,dummy in self.curVisibleZoneNums.items():
            self.showZone(vz)
        # Redo visibility using current zone num
        self.updateVisibility()

    def handleVisChange(self):
        """the zone visibility lists have changed on-the-fly"""
        Level.Level.handleVisChange(self)
        self.visChangedThisFrame = 1

    def visChangeTask(self, task):
        # this runs just before igloop; if viz lists have changed
        # this frame, updates the visibility and sends out a setZoneMsg
        if self.visChangedThisFrame:
            self.updateVisibility()
            self.visChangedThisFrame = 0
        return Task.cont

    if __dev__:
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

        description = getDescription(self.lastCamZone)
        if description and description != '':
            taskMgr.remove(self.uniqueName("titleText"))
            self.smallTitleText.setText(description)
            self.titleText.setText(description)
            self.titleText.setColor(Vec4(*self.titleColor))
            self.titleText.setFg(self.titleColor)

            # Only show the big title once per session.
            # If we've already seen it, just show the small title

            titleSeq = None
            if not self.lastCamZone in self.zonesEnteredList:
                self.zonesEnteredList.append(self.lastCamZone)
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
                                          Task.Task(self.showSmallTitleTask))
            if titleSeq:
                seq = Task.sequence(titleSeq, smallTitleSeq)
            else:
                seq = smallTitleSeq
            taskMgr.add(seq, self.uniqueName("titleText"))
        
    def showTitleTextTask(self, task):
        assert(DistributedLevel.notify.debug("hideTitleTextTask()"))
        self.titleText.show()
        return Task.done

    def hideTitleTextTask(self, task):
        assert(DistributedLevel.notify.debug("hideTitleTextTask()"))
        if self.titleText:
            self.titleText.hide()
        return Task.done

    def showSmallTitleTask(self, task):
        # make sure large title is hidden
        if self.titleText:
            self.titleText.hide()
        # show the small title
        self.smallTitleText.show()
        return Task.done
    
    def hideSmallTitleTextTask(self, task):
        assert(DistributedLevel.notify.debug("hideTitleTextTask()"))
        if self.smallTitleText:
            self.smallTitleText.hide()
        return Task.done

    # Ouch!
    def startOuch(self, ouchLevel, period=2):
        print 'startOuch %s' % ouchLevel
        if not hasattr(self, 'doingOuch'):
            def doOuch(task, self=self, ouchLevel=ouchLevel, period=period):
                self.b_setOuch(ouchLevel)
                self.lastOuchTime = globalClock.getFrameTime()
                taskMgr.doMethodLater(period, doOuch,
                                      DistributedLevel.OuchTaskName)

            # check to make sure we haven't done an ouch too recently
            delay = 0
            if hasattr(self, 'lastOuchTime'):
                curFrameTime = globalClock.getFrameTime()
                timeSinceLastOuch = (curFrameTime - self.lastOuchTime)
                if timeSinceLastOuch < period:
                    delay = period - timeSinceLastOuch

            if delay > 0:
                taskMgr.doMethodLater(
                        period, doOuch,
                        DistributedLevel.OuchTaskName)
            else:
                doOuch(None)
            self.doingOuch = 1

    def stopOuch(self):
        if hasattr(self, 'doingOuch'):
            taskMgr.remove(DistributedLevel.OuchTaskName)
            del self.doingOuch

    def b_setOuch(self, penalty, anim=None):
        self.notify.debug('b_setOuch %s' % penalty)
        av = toonbase.localToon

        # play the stun track (flashing toon) 
        if not av.isStunned:
            self.d_setOuch(penalty)
            self.setOuch(penalty, anim)

    def d_setOuch(self, penalty):
        self.sendUpdate("setOuch", [penalty])

    def setOuch(self, penalty, anim = None):
        if anim == "Squish":
            toonbase.tcr.playGame.getPlace().fsm.request('squished')
        elif anim == "Fall":
            toonbase.tcr.playGame.getPlace().fsm.request('fallDown')
            
        av = toonbase.localToon
        av.stunToon()
        av.playDialogueForString("!")
