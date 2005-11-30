#hack:
BAD_DO_ID = BAD_ZONE_ID = 0xFFFFFFFF
BAD_CHANNEL_ID = 0xFFFFFFFFFFFFFFFF

class DoCollectionManager:
    def __init__(self):
        # Dict of {DistributedObject ids : DistributedObjects}
        self.doId2do = {}
        # (parentId, zoneId) to dict of doId->DistributedObjectAI
        ## self.zoneId2doIds={}
        if self.hasOwnerView():
            # Dict of {DistributedObject ids : DistributedObjects} for 'owner' views of objects
            self.doId2ownerView = {}
        # Dict of {
        #   parent DistributedObject id: 
        #     { zoneIds : [child DistributedObject ids] }}
        self.__doHierarchy = {}

    def getDo(self, doId):
        return self.doId2do.get(doId)
    def getOwnerView(self, doId):
        assert self.hasOwnerView()
        return self.doId2ownerView.get(doId)

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

    def getDoHierarchy(self):
        return self.__doHierarchy

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
        parent=self.__doHierarchy.get(parentId)
        if parent is None:
            return []
        if zoneId is None:
            r = []
            for zone in parent.values():
                for obj in zone:
                    r.append(obj)
        else:
            r = parent.get(zoneId, [])
        if classType is not None:
            a = []
            for obj in r:
                if isinstance(obj, classType):
                    a.append(obj)
            r = a
        return r
    
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
        if len(self.__doHierarchy) > 0:
            self.notify.warning(
                '__doHierarchy table not empty: %s' % self.__doHierarchy)
            self.__doHierarchy = {}

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
            obj.setLocation(parentId, zoneId)
            self.storeObjectLocation(doId, parentId, zoneId)
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

    def storeObjectLocation(self, doId, parentId, zoneId):
        obj = self.doId2do.get(doId)
        if obj is not None:
            oldParentId = obj.parentId
            oldZoneId = obj.zoneId
            if oldParentId != parentId:
                # Remove old location
                self.deleteObjectLocation(doId, oldParentId, oldZoneId)
            if oldParentId == parentId and oldZoneId == zoneId:
                # object is already at that parent and zone
                return
            if (parentId is None) or (zoneId is None):
                # Do not store null values
                return
            # Add to new location
            parentZoneDict = self.__doHierarchy.setdefault(parentId, {})
            zoneDoSet = parentZoneDict.setdefault(zoneId, set())
            zoneDoSet.add(doId)

    def deleteObjectLocation(self, doId, parentId, zoneId):
        # Do not worry about null values
        if (parentId is None) or (zoneId is None):
            return
        parentZoneDict = self.__doHierarchy.get(parentId)
        if parentZoneDict is not None:
            zoneDoSet = parentZoneDict.get(zoneId)
            if zoneDoSet is not None:
                if doId in zoneDoSet:
                    zoneDoSet.remove(doId)
                    if len(zoneDoSet) == 0:
                        del parentZoneDict[zoneId]
                else:
                    self.notify.warning(
                        "deleteObjectLocation: objId: %s not found"%(doId,))
            else:
                self.notify.warning(
                    "deleteObjectLocation: zoneId: %s not found"%(zoneId,))
        else:
            self.notify.warning(
                "deleteObjectLocation: parentId: %s not found"%(parentId,))
    
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
                self.storeObjectLocation(do.doId, location[0], location[1])
                ##assert do.doId not in self.zoneId2doIds.get(location,{})
                ##self.zoneId2doIds.setdefault(location, {})
                ##self.zoneId2doIds[location][do.doId]=do

    def isValidLocationTuple(self, location):
        return (location is not None
            and location != (0xffffffff, 0xffffffff)
            and location != (0, 0))

    def removeDOFromTables(self, do):
        assert self.notify.debugStateCall(self)
        #assert not hasattr(do, "isQueryAllResponse") or not do.isQueryAllResponse
        #assert do.doId in self.doId2do
        self.deleteObjectLocation(do.doId, do.parentId, do.zoneId)
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
        
    def changeDOZoneInTables(self, do, newParentId, newZoneId, oldParentId, oldZoneId):
        if 1:
            self.storeObjectLocation(do.doId, newParentId, newZoneId)
        else:
            #assert not hasattr(do, "isQueryAllResponse") or not do.isQueryAllResponse
            oldLocation = (oldParentId, oldZoneId)
            newLocation = (newParentId, newZoneId)
            # HACK: DistributedGuildMemberUD starts in -1,-1, which isnt ever put in the
            # zoneId2doIds table
            if self.isValidLocationTuple(oldLocation):
                assert self.notify.debugStateCall(self)
                assert oldLocation in self.zoneId2doIds
                assert do.doId in self.zoneId2doIds[oldLocation]
                assert do.doId not in self.zoneId2doIds.get(newLocation,{})
                # remove from old zone
                del(self.zoneId2doIds[oldLocation][do.doId])
                if len(self.zoneId2doIds[oldLocation]) == 0:
                    del self.zoneId2doIds[oldLocation]
            if self.isValidLocationTuple(newLocation):
                # add to new zone
                self.zoneId2doIds.setdefault(newLocation, {})
                self.zoneId2doIds[newLocation][do.doId]=do

    ## def getObjectsInZone(self, parentId, zoneId):
    ##     """ call this to get a dict of doId:distObj for a zone.
    ##     Creates a shallow copy, so you can do whatever you want with the
    ##     dict. """
    ##     assert self.notify.debugStateCall(self)
    ##     return copy.copy(self.zoneId2doIds.get((parentId, zoneId), {}))

    def getObjectsOfClassInZone(self, parentId, zoneId, objClass):
        """
        returns dict of doId:object for a zone, containing all objects
        that inherit from 'class'. returned dict is safely mutable.
        """
        assert self.notify.debugStateCall(self)
        doDict = {}
        for doId in self.getDoIdList(parentId, zoneId, objClass):
            if isinstance(do, objClass):
                doDict[doId] = self.doId2do.get(do)
        return doDict
