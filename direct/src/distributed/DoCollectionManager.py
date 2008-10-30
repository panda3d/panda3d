from direct.distributed import DoHierarchy
import re

#hack:
BAD_DO_ID = BAD_ZONE_ID = 0 # 0xFFFFFFFF
BAD_CHANNEL_ID = 0 # 0xFFFFFFFFFFFFFFFF

class DoCollectionManager:
    def __init__(self):
        # Dict of {DistributedObject ids: DistributedObjects}
        self.doId2do = {}
        # (parentId, zoneId) to dict of doId->DistributedObjectAI
        ## self.zoneId2doIds={}
        if self.hasOwnerView():
            # Dict of {DistributedObject ids: DistributedObjects}
            # for 'owner' views of objects
            self.doId2ownerView = {}
        # Dict of {
        #   parent DistributedObject id:
        #     { zoneIds: [child DistributedObject ids] }}
        self._doHierarchy = DoHierarchy.DoHierarchy()

    def getDo(self, doId):
        return self.doId2do.get(doId)

    def callbackWithDo(self, doId, callback):
        do = self.doId2do.get(doId)
        if do is not None:
            callback(do)
        else:
            relatedObjectMgr(doId, allCallback=callback)

    def getOwnerView(self, doId):
        assert self.hasOwnerView()
        return self.doId2ownerView.get(doId)

    def callbackWithOwnerView(self, doId, callback):
        assert self.hasOwnerView()
        do = self.doId2ownerView.get(doId)
        if do is not None:
            callback(do)
        else:
            pass #relatedObjectMgr(doId, allCallback=callback)

    def getDoTable(self, ownerView):
        if ownerView:
            assert self.hasOwnerView()
            return self.doId2ownerView
        else:
            return self.doId2do

    def doFind(self, str):
        """
        Returns list of distributed objects with matching str in value.
        """
        for value in self.doId2do.values():
            if `value`.find(str) >= 0:
                return value

    def doFindAll(self, str):
        """
        Returns list of distributed objects with matching str in value.
        """
        matches = []
        for value in self.doId2do.values():
            if `value`.find(str) >= 0:
                matches.append(value)
        return matches

    def doFindAllMatching(self, str):
        """
        Returns list of distributed objects with matching str in value.
        """
        matches = []
        for value in self.doId2do.values():
            if re.search(str,`value`):
                matches.append(value)
        return matches


    def _getDistanceFromLA(self, do):
        if hasattr(do, 'getPos'):
            return do.getPos(localAvatar).length()
        return None

    def _compareDistance(self, do1, do2):
        dist1 = self._getDistanceFromLA(do1)
        dist2 = self._getDistanceFromLA(do2)
        if dist1 is None and dist2 is None:
            return 0
        if dist1 is None:
            return 1
        if dist2 is None:
            return -1
        if (dist1 < dist2):
            return -1
        return 1

    def dosByDistance(self):
        objs = self.doId2do.values()
        objs.sort(cmp=self._compareDistance)
        return objs

    def doByDistance(self):
        objs = self.dosByDistance()
        for obj in objs:
            print '%s\t%s\t%s' % (obj.doId, self._getDistanceFromLA(obj),
                                  obj.dclass.getName())

    if __debug__:
        def printObjects(self):
            format="%10s %10s %10s %30s %20s"
            title=format%("parentId", "zoneId", "doId", "dclass", "name")
            print title
            print '-'*len(title)
            for distObj in self.doId2do.values():
                print format%(
                    distObj.__dict__.get("parentId"),
                    distObj.__dict__.get("zoneId"),
                    distObj.__dict__.get("doId"),
                    distObj.dclass.getName(),
                    distObj.__dict__.get("name"))

    def _printObjects(self, table):
        class2count = {}
        for obj in self.getDoTable(ownerView=False).values():
            className = obj.__class__.__name__
            class2count.setdefault(className, 0)
            class2count[className] += 1
        count2classes = invertDictLossless(class2count)
        counts = count2classes.keys()
        counts.sort()
        counts.reverse()
        for count in counts:
            count2classes[count].sort()
            for name in count2classes[count]:
                print '%s %s' % (count, name)
        print ''

    def _returnObjects(self, table):
        class2count = {}
        stringToReturn = ''
        for obj in self.getDoTable(ownerView=False).values():
            className = obj.__class__.__name__
            class2count.setdefault(className, 0)
            class2count[className] += 1
        count2classes = invertDictLossless(class2count)
        counts = count2classes.keys()
        counts.sort()
        counts.reverse()
        for count in counts:
            count2classes[count].sort()
            for name in count2classes[count]:
                # print '%s %s' % (count, name)
                stringToReturn = '%s%s %s\n' % (stringToReturn, count, name)
        # print ''
        return stringToReturn

    def webPrintObjectCount(self):
        strToReturn = '==== OBJECT COUNT ====\n'
        if self.hasOwnerView():
            strToReturn = '%s == doId2do\n' % (strToReturn)
        strToReturn = '%s%s' % (strToReturn, self._returnObjects(self.getDoTable(ownerView=False)))
        if self.hasOwnerView():
            strToReturn = '%s\n== doId2ownerView\n' % (strToReturn)
            strToReturn = '%s%s' % (strToReturn, self._returnObjects(self.getDoTable(ownerView=False)))
        return strToReturn
        

    def printObjectCount(self):
        # print object counts by distributed object type
        print '==== OBJECT COUNT ===='
        if self.hasOwnerView():
            print '== doId2do'
        self._printObjects(self.getDoTable(ownerView=False))
        if self.hasOwnerView():
            print '== doId2ownerView'
            self._printObjects(self.getDoTable(ownerView=True))

    def getDoList(self, parentId, zoneId=None, classType=None):
        """
        parentId is any distributed object id.
        zoneId is a uint32, defaults to None (all zones).  Try zone 2 if
            you're not sure which zone to use (0 is a bad/null zone and
            1 has had reserved use in the past as a no messages zone, while
            2 has traditionally been a global, uber, misc stuff zone).
        dclassType is a distributed class type filter, defaults
            to None (no filter).

        If dclassName is None then all objects in the zone are returned;
        otherwise the list is filtered to only include objects of that type.
        """
        return [self.doId2do.get(i)
            for i in self.getDoIdList(parentId, zoneId, classType)]

    def getDoIdList(self, parentId, zoneId=None, classType=None):
        return self._doHierarchy.getDoIds(self.getDo,
                                          parentId, zoneId, classType)

    def hasOwnerViewDoId(self, doId):
        assert self.hasOwnerView()
        return doId in self.doId2ownerView
    
    def getOwnerViewDoList(self, classType):
        assert self.hasOwnerView()
        l = []
        for obj in self.doId2ownerView.values():
            if isinstance(obj, classType):
                l.append(obj)
        return l

    def getOwnerViewDoIdList(self, classType):
        assert self.hasOwnerView()
        l = []
        for doId, obj in self.doId2ownerView.items():
            if isinstance(obj, classType):
                l.append(doId)
        return l

    def countObjects(self, classType):
        """
        Counts the number of objects of the given type in the
        repository (for testing purposes)
        """
        count = 0
        for dobj in self.doId2do.values():
            if isinstance(dobj, classType):
                count += 1
        return count


    def getAllOfType(self, type):
        # Returns a list of all DistributedObjects in the repository
        # of a particular type.
        result = []
        for obj in self.doId2do.values():
            if isinstance(obj, type):
                result.append(obj)
        return result

    def findAnyOfType(self, type):
        # Searches the repository for any object of the given type.
        for obj in self.doId2do.values():
            if isinstance(obj, type):
                return obj
        return None

    #----------------------------------

    def deleteDistributedObjects(self):
        # Get rid of all the distributed objects
        for doId in self.doId2do.keys():
            # Look up the object
            do = self.doId2do[doId]
            self.deleteDistObject(do)

        # Get rid of everything that manages distributed objects
        self.deleteObjects()

        # the zoneId2doIds table should be empty now
        if not self._doHierarchy.isEmpty():
            self.notify.warning(
                '_doHierarchy table not empty: %s' % self._doHierarchy)
            self._doHierarchy.clear()

    def handleObjectLocation(self, di):
        # CLIENT_OBJECT_LOCATION
        doId = di.getUint32()
        parentId = di.getUint32()
        zoneId = di.getUint32()
        obj = self.doId2do.get(doId)
        if obj is not None:
            self.notify.debug(
                "handleObjectLocation: doId: %s parentId: %s zoneId: %s"%
                (doId, parentId, zoneId))
            # Let the object finish the job
            # calls storeObjectLocation()
            obj.setLocation(parentId, zoneId) 
        else:
            self.notify.warning(
                "handleObjectLocation: Asked to update non-existent obj: %s" % (doId))

    def handleSetLocation(self, di):
        # This was initially added because creating a distributed quest
        # object would cause a message like this to be generated.
        assert self.notify.debugStateCall(self)
        parentId = di.getUint32()
        zoneId = di.getUint32()
        distObj = self.doId2do.get(self.getMsgChannel())
        if distObj is not None:
            distObj.setLocation(parentId, zoneId)
        else:
            self.notify.warning('handleSetLocation: object %s not present' % self.getMsgChannel())

    @exceptionLogged()
    def storeObjectLocation(self, object, parentId, zoneId):
        oldParentId = object.parentId
        oldZoneId = object.zoneId
        if (oldParentId != parentId):
            # notify any existing parent that we're moving away
            oldParentObj = self.doId2do.get(oldParentId)
            if oldParentObj is not None:
                oldParentObj.handleChildLeave(object, oldZoneId)
            self.deleteObjectLocation(object, oldParentId, oldZoneId)
            
        elif (oldZoneId != zoneId):
            # Remove old location
            oldParentObj = self.doId2do.get(oldParentId)
            if oldParentObj is not None:
                oldParentObj.handleChildLeaveZone(object, oldZoneId)
            self.deleteObjectLocation(object, oldParentId, oldZoneId)
        else:
            # object is already at that parent and zone
            return

        if ((parentId is None) or (zoneId is None) or
            (parentId == zoneId == 0)):
            # Do not store null values
            object.parentId = None
            object.zoneId = None
        else:
            # Add to new location
            self._doHierarchy.storeObjectLocation(object, parentId, zoneId)
            # this check doesn't work because of global UD objects;
            # should they have a location?
            #assert len(self._doHierarchy) == len(self.doId2do)

            # Set the new parent and zone on the object
            object.parentId = parentId
            object.zoneId = zoneId


        if oldParentId != parentId:
            # Give the parent a chance to run code when a new child
            # sets location to it. For example, the parent may want to
            # scene graph reparent the child to some subnode it owns.
            parentObj = self.doId2do.get(parentId)
            if parentObj is not None:
                parentObj.handleChildArrive(object, zoneId)
            elif parentId not in (0, self.getGameDoId()):
                self.notify.warning('storeObjectLocation(%s): parent %s not present' %
                                    (object.doId, parentId))
        elif oldZoneId != zoneId:
            parentObj = self.doId2do.get(parentId)
            if parentObj is not None:
                parentObj.handleChildArriveZone(object, zoneId)
            elif parentId not in (0, self.getGameDoId()):
                self.notify.warning('storeObjectLocation(%s): parent %s not present' %
                                    (object.doId, parentId))
            
    def deleteObjectLocation(self, object, parentId, zoneId):
        # Do not worry about null values
        if ((parentId is None) or (zoneId is None) or
            (parentId == zoneId == 0)):
            return

        self._doHierarchy.deleteObjectLocation(object, parentId, zoneId)

    def addDOToTables(self, do, location=None, ownerView=False):
        assert self.notify.debugStateCall(self)
        #assert not hasattr(do, "isQueryAllResponse") or not do.isQueryAllResponse
        if not ownerView:
            if location is None:
                location = (do.parentId, do.zoneId)

        doTable = self.getDoTable(ownerView)

        # make sure the object is not already present
        if do.doId in doTable:
            if ownerView:
                tableName = 'doId2ownerView'
            else:
                tableName = 'doId2do'
            self.notify.error('doId %s already in %s [%s stomping %s]' % (
                do.doId, tableName, do.__class__.__name__,
                doTable[do.doId].__class__.__name__))

        doTable[do.doId]=do

        if not ownerView:
            if self.isValidLocationTuple(location):
                self.storeObjectLocation(do, location[0], location[1])
                ##assert do.doId not in self.zoneId2doIds.get(location, {})
                ##self.zoneId2doIds.setdefault(location, {})
                ##self.zoneId2doIds[location][do.doId]=do

    def isValidLocationTuple(self, location):
        return (location is not None
            and location != (0xffffffff, 0xffffffff)
            and location != (0, 0))

    if __debug__:
        def isInDoTables(self, doId):
            assert self.notify.debugStateCall(self)
            return doId in self.doId2do

    def removeDOFromTables(self, do):
        assert self.notify.debugStateCall(self)
        #assert not hasattr(do, "isQueryAllResponse") or not do.isQueryAllResponse
        #assert do.doId in self.doId2do
        location = do.getLocation()
        if location:
           oldParentId, oldZoneId = location
           oldParentObj = self.doId2do.get(oldParentId)
           if oldParentObj:
               oldParentObj.handleChildLeave(do, oldZoneId)
        self.deleteObjectLocation(do, do.parentId, do.zoneId)
        ## location = do.getLocation()
        ## if location is not None:
        ##     if location not in self.zoneId2doIds:
        ##         self.notify.warning(
        ##             'dobj %s (%s) has invalid location: %s' %
        ##             (do, do.doId, location))
        ##     else:
        ##         assert do.doId in self.zoneId2doIds[location]
        ##         del self.zoneId2doIds[location][do.doId]
        ##         if len(self.zoneId2doIds[location]) == 0:
        ##             del self.zoneId2doIds[location]
        if do.doId in self.doId2do:
            del self.doId2do[do.doId]

    ## def changeDOZoneInTables(self, do, newParentId, newZoneId, oldParentId, oldZoneId):
    ##     if 1:
    ##         self.storeObjectLocation(do.doId, newParentId, newZoneId)
    ##     else:
    ##         #assert not hasattr(do, "isQueryAllResponse") or not do.isQueryAllResponse
    ##         oldLocation = (oldParentId, oldZoneId)
    ##         newLocation = (newParentId, newZoneId)
    ##         # HACK: DistributedGuildMemberUD starts in -1, -1, which isnt ever put in the
    ##         # zoneId2doIds table
    ##         if self.isValidLocationTuple(oldLocation):
    ##             assert self.notify.debugStateCall(self)
    ##             assert oldLocation in self.zoneId2doIds
    ##             assert do.doId in self.zoneId2doIds[oldLocation]
    ##             assert do.doId not in self.zoneId2doIds.get(newLocation, {})
    ##             # remove from old zone
    ##             del(self.zoneId2doIds[oldLocation][do.doId])
    ##             if len(self.zoneId2doIds[oldLocation]) == 0:
    ##                 del self.zoneId2doIds[oldLocation]
    ##         if self.isValidLocationTuple(newLocation):
    ##             # add to new zone
    ##             self.zoneId2doIds.setdefault(newLocation, {})
    ##             self.zoneId2doIds[newLocation][do.doId]=do

    def getObjectsInZone(self, parentId, zoneId):
        """
        returns dict of doId:distObj for a zone.
        returned dict is safely mutable.
        """
        assert self.notify.debugStateCall(self)
        doDict = {}
        for doId in self.getDoIdList(parentId, zoneId):
            doDict[doId] = self.getDo(doId)
        return doDict

    def getObjectsOfClassInZone(self, parentId, zoneId, objClass):
        """
        returns dict of doId:object for a zone, containing all objects
        that inherit from 'class'. returned dict is safely mutable.
        """
        assert self.notify.debugStateCall(self)
        doDict = {}
        for doId in self.getDoIdList(parentId, zoneId, objClass):
            doDict[doId] = self.getDo(doId)
        return doDict
