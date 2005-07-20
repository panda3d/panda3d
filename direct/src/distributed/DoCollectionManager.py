#hack:
BAD_DO_ID = BAD_ZONE_ID = -1

class DoCollectionManager:
    def __init__(self):
        # Dict of {DistributedObject ids : DistributedObjects}
        self.doId2do = {}
        if wantOtpServer:
            # Dict of {
            #   parent DistributedObject id: 
            #     { zoneIds : [child DistributedObject ids] }}
            self.__doHierarchy = {}

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
        if len(self.zoneId2doIds) > 0:
            AIRepository.notify.warning(
                'zoneId2doIds table not empty: %s' % self.zoneId2doIds)
            self.zoneId2doIds = {}

    if wantOtpServer:
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
            if (parentId is None) or (zoneId is None):
                # Do not store null values
                return
            # TODO: check current location
            obj = self.doId2do.get(doId)
            if obj is not None:
                oldParentId = obj.parentId
                oldZoneId = obj.zoneId

                if oldParentId != parentId:
                    # Remove old location
                    parentZoneDict = self.__doHierarchy.get(oldParentId)
                    if parentZoneDict is not None:
                        zoneDoSet = parentZoneDict.get(oldZoneId)
                        if zoneDoSet is not None and doId in zoneDoSet:
                            zoneDoSet.remove(doId)
                            if len(zoneDoSet) == 0:
                                del parentZoneDict[oldZoneId]
                # Add to new location
                parentZoneDict = self.__doHierarchy.setdefault(parentId, {})
                zoneDoSet = parentZoneDict.setdefault(zoneId, set())
                zoneDoSet.add(doId)

        def deleteObjectLocation(self, objId, parentId, zoneId):
            # Do not worry about null values
            if ((parentId is None) or (zoneId is None)):
                return
            parentZoneDict = self.__doHierarchy.get(parentId)
            assert(parentZoneDict is not None, "deleteObjectLocation: parentId: %s not found" % (parentId))
            objList = parentZoneDict.get(zoneId)
            assert(objList is not None, "deleteObjectLocation: zoneId: %s not found" % (zoneId))
            assert(objId in objList, "deleteObjectLocation: objId: %s not found" % (objId))
            if len(objList) == 1:
                # If this is the last obj in this zone, delete the entire entry
                del parentZoneDict[zoneId]
            else:
                # Just remove the object
                objList.remove(objId)
    
    if wantOtpServer:
        def addDOToTables(self, do, location=None):
            assert self.notify.debugStateCall(self)
            if location is None:
                location = (do.parentId, do.zoneId)

            assert do.doId not in self.doId2do
            self.doId2do[do.doId]=do

            if self.isValidLocationTuple(location):
                assert do.doId not in self.zoneId2doIds.get(location,{})
                self.zoneId2doIds.setdefault(location, {})
                self.zoneId2doIds[location][do.doId]=do

        def isValidLocationTuple(self, location):
            return (location is not None
                and location != (0xffffffff, 0xffffffff)
                and location != (0, 0))
    else:
        # NON OTP
        def addDOToTables(self, do, zoneId=None):
            assert self.notify.debugStateCall(self)
            if zoneId is None:
                zoneId = do.zoneId

            assert do.doId not in self.doId2do
            self.doId2do[do.doId]=do

            if zoneId is not None:
                assert do.doId not in self.zoneId2doIds.get(zoneId,{})
                self.zoneId2doIds.setdefault(zoneId, {})
                self.zoneId2doIds[zoneId][do.doId]=do

    if wantOtpServer:
        def removeDOFromTables(self, do):
            assert self.notify.debugStateCall(self)
            assert do.doId in self.doId2do
            location = do.getLocation()
            if location is not None:
                if location not in self.zoneId2doIds:
                    AIRepository.notify.warning(
                        'dobj %s (%s) has invalid location: %s' %
                        (do, do.doId, location))
                else:
                    assert do.doId in self.zoneId2doIds[location]
                    del(self.zoneId2doIds[location][do.doId])
                    if len(self.zoneId2doIds[location]) == 0:
                        del self.zoneId2doIds[location]

            del self.doId2do[do.doId]
    else:
        def removeDOFromTables(self, do):
            assert self.notify.debugStateCall(self)
            assert do.doId in self.doId2do
            if do.zoneId is not None:
                if do.zoneId not in self.zoneId2doIds:
                    AIRepository.notify.warning(
                            'dobj %s (%s) has invalid zoneId: %s' %
                            (do, do.doId, do.zoneId))
                else:
                    assert do.doId in self.zoneId2doIds[do.zoneId]
                    del(self.zoneId2doIds[do.zoneId][do.doId])
                    if len(self.zoneId2doIds[do.zoneId]) == 0:
                        del self.zoneId2doIds[do.zoneId]

            del self.doId2do[do.doId]
        
    if wantOtpServer:
        def changeDOZoneInTables(self, do, newParentId, newZoneId, oldParentId, oldZoneId):
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

        def getObjectsInZone(self, location):
            """ call this to get a dict of doId:distObj for a zone.
            Creates a shallow copy, so you can do whatever you want with the
            dict. """
            assert self.notify.debugStateCall(self)
            return copy.copy(self.zoneId2doIds.get(location, {}))

        def getObjectsOfClassInZone(self, location, objClass):
            """ returns dict of doId:object for a zone, containing all objects
            that inherit from 'class'. returned dict is safely mutable. """
            assert self.notify.debugStateCall(self)
            doDict = {}
            for doId, do in self.zoneId2doIds.get(location, {}).items():
                if isinstance(do, objClass):
                    doDict[doId] = do
            return doDict

    else:
        # NON OTP

        def changeDOZoneInTables(self, do, newZoneId, oldZoneId):
            ##print "changeDOZoneInTables:%s, dclass:%s, newZoneId:%s OldZoneId:%s"%(do.doId, do.dclass.getName(), newZoneId,oldZoneId)

            assert self.notify.debugStateCall(self)
            assert oldZoneId in self.zoneId2doIds
            assert do.doId in self.zoneId2doIds[oldZoneId]
            assert do.doId not in self.zoneId2doIds.get(newZoneId,{})
            # remove from old zone
            del(self.zoneId2doIds[oldZoneId][do.doId])
            if len(self.zoneId2doIds[oldZoneId]) == 0:
                del self.zoneId2doIds[oldZoneId]
            # add to new zone
            self.zoneId2doIds.setdefault(newZoneId, {})
            self.zoneId2doIds[newZoneId][do.doId]=do

        def getObjectsInZone(self, zoneId):
            """ call this to get a dict of doId:distObj for a zone.
            Creates a shallow copy, so you can do whatever you want with the
            dict. """
            assert self.notify.debugStateCall(self)
            return copy.copy(self.zoneId2doIds.get(zoneId, {}))

        def getObjectsOfClassInZone(self, zoneId, objClass):
            """ returns dict of doId:object for a zone, containing all objects
            that inherit from 'class'. returned dict is safely mutable. """
            assert self.notify.debugStateCall(self)
            doDict = {}
            for doId, do in self.zoneId2doIds.get(zoneId, {}).items():
                if isinstance(do, objClass):
                    doDict[doId] = do
            return doDict
