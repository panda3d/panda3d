"""LevelSpec module: contains the LevelSpec class"""

import DirectNotifyGlobal
from PythonUtil import list2dict, uniqueElements
import string
import LevelConstants
import types
if __dev__:
    import os

class LevelSpec:
    """contains spec data for a level, is responsible for handing the data
    out upon request, as well as recording changes made during editing, and
    saving out modified spec data"""
    notify = DirectNotifyGlobal.directNotify.newCategory("LevelSpec")
    
    def __init__(self, spec=None, scenario=0):
        """spec must be passed in as a python module or a dictionary.
        If not passed in, will create a new spec."""
        newSpec = 0
        if type(spec) is types.ModuleType:
            if __dev__:
                # reload the spec module to pick up changes
                reload(spec)
            self.specDict = spec.levelSpec
            if __dev__:
                self.setFilename(spec.__file__)
        elif type(spec) is types.DictType:
            # we need this for repr/eval-ing LevelSpecs
            self.specDict = spec
        elif spec is None:
            if __dev__:
                newSpec = 1
                self.specDict = {
                    'globalEntities': {},
                    'scenarios': [[{}, 1]],
                    }

        assert hasattr(self, 'specDict')

        # this maps an entId to the dict that holds its spec;
        # entities are either in the global dict or a scenario dict
        # update the map of entId to spec dict
        self.entId2specDict = {}
        self.entId2specDict.update(
            list2dict(self.getGlobalEntIds(),
                      value=self.privGetGlobalEntityDict()))
        for i in range(self.getNumScenarios()):
            self.entId2specDict.update(
                list2dict(self.getScenarioEntIds(i),
                          value=self.privGetScenarioEntityDict(i)))

        self.setScenario(scenario)

        if __dev__:
            if newSpec:
                # add basic required entities
                import EntityTypes
                import EntityTypeRegistry
                etr = EntityTypeRegistry.EntityTypeRegistry(EntityTypes)
                self.setEntityTypeReg(etr)

                # UberZone
                entId = LevelConstants.UberZoneEntId
                self.insertEntity(entId, 'zone')
                self.doSetAttrib(entId, 'name', 'UberZone')
                # LevelMgr
                entId = LevelConstants.LevelMgrEntId
                self.insertEntity(entId, 'levelMgr')
                self.doSetAttrib(entId, 'name', 'LevelMgr')
                # EditMgr
                entId = LevelConstants.EditMgrEntId
                self.insertEntity(entId, 'editMgr')
                self.doSetAttrib(entId, 'name', 'EditMgr')
                                       
    def getNumScenarios(self):
        return len(self.specDict['scenarios'])

    def getScenarioWeights(self):
        weights = []
        for entry in self.specDict['scenarios']:
            weights.append(entry[1])
        return weights

    def setScenario(self, scenario):
        assert scenario in range(0, self.getNumScenarios())
        self.scenario = scenario

    def getScenario(self):
        return self.scenario

    def getGlobalEntIds(self):
        return self.privGetGlobalEntityDict().keys()

    def getScenarioEntIds(self, scenario=None):
        if scenario is None:
            scenario = self.scenario
        return self.privGetScenarioEntityDict(scenario).keys()

    def getAllEntIds(self):
        """this returns all of the entIds involved in the current scenario"""
        return self.getGlobalEntIds() + self.getScenarioEntIds()

    def getAllEntIdsFromAllScenarios(self):
        """this returns all of the entIds involved in all scenarios"""
        entIds = self.getGlobalEntIds()
        for scenario in xrange(self.getNumScenarios()):
            entIds.extend(self.getScenarioEntIds(scenario))
        return entIds

    def getEntitySpec(self, entId):
        assert entId in self.entId2specDict
        specDict = self.entId2specDict[entId]
        return specDict[entId]

    def getEntityType(self, entId):
        return self.getEntitySpec(entId)['type']

    def getEntityZoneEntId(self, entId):
        """ return the entId of the zone that entity is in; if entity
        is a zone, returns its entId """
        spec = self.getEntitySpec(entId)
        type = spec['type']
        # if it's a zone, this is our entity
        if type == 'zone':
            return entId
        # keep looking up the heirarchy for a zone entity
        return self.getEntityZoneEntId(spec['parentEntId'])

    def getEntType2ids(self, entIds):
        """given list of entIds, return dict of entType->entIds"""
        entType2ids = {}
        for entId in entIds:
            type = self.getEntityType(entId)
            entType2ids.setdefault(type, [])
            entType2ids[type].append(entId)
        return entType2ids

    # private support functions to abstract dict structure
    def privGetGlobalEntityDict(self):
        return self.specDict['globalEntities']

    def privGetScenarioEntityDict(self, scenario):
        return self.specDict['scenarios'][scenario][0]

    def printZones(self):
        """currently prints list of zoneNum->zone name"""
        # this could be more efficient
        allIds = self.getAllEntIds()
        type2id = self.getEntType2ids(allIds)
        zoneIds = type2id['zone']
        # omit the UberZone
        if 0 in zoneIds:
            zoneIds.remove(0)
        zoneIds.sort()
        for zoneNum in zoneIds:
            spec = self.getEntitySpec(zoneNum)
            print 'zone %s : %s' % (zoneNum, spec['name'])

    if __dev__:
        def setLevel(self, level):
            self.level = level

        def hasLevel(self):
            return hasattr(self, 'level')

        def setEntityTypeReg(self, entTypeReg):
            self.entTypeReg = entTypeReg
            self.checkSpecIntegrity()

        def hasEntityTypeReg(self):
            return hasattr(self, 'entTypeReg')

        def setFilename(self, filename):
            self.filename = filename

        def doSetAttrib(self, entId, attrib, value):
            """ do the dirty work of changing an attrib value """
            assert entId in self.entId2specDict
            specDict = self.entId2specDict[entId]
            assert specDict[entId].has_key(attrib)
            specDict[entId][attrib] = value

        def setAttribChange(self, entId, attrib, value, username):
            """ we're being asked to change an attribute """
            LevelSpec.notify.info("setAttribChange(%s): %s, %s = %s" %
                                  (username, entId, attrib, repr(value)))
            self.doSetAttrib(entId, attrib, value)
            if self.hasLevel():
                # let the level know that this attribute value has
                # officially changed
                self.level.handleAttribChange(entId, attrib, value, username)

        def insertEntity(self, entId, entType, parentEntId='unspecified'):
            LevelSpec.notify.info('inserting entity %s (%s)' % (entId, entType))
            assert entId not in self.entId2specDict
            assert self.entTypeReg is not None
            globalEnts = self.privGetGlobalEntityDict()
            self.entId2specDict[entId] = globalEnts

            # create a new entity spec entry w/ default values
            globalEnts[entId] = {}
            spec = globalEnts[entId]
            attribDescs = self.entTypeReg.getTypeDesc(entType
                                                      ).getAttribDescDict()
            for name, desc in attribDescs.items():
                spec[name] = desc.getDefaultValue()
            spec['type'] = entType
            if parentEntId != 'unspecified':
                spec['parentEntId'] = parentEntId

            if self.hasLevel():
                # notify the level
                self.level.handleEntityInsert(entId)
            else:
                LevelSpec.notify.warning('no level to be notified of insertion')
            
        def removeEntity(self, entId):
            LevelSpec.notify.info('removing entity %s' % entId)
            assert entId in self.entId2specDict

            if self.hasLevel():
                # notify the level
                self.level.handleEntityRemove(entId)
            else:
                LevelSpec.notify.warning('no level to be notified of removal')

            # remove the entity's spec
            dict = self.entId2specDict[entId]
            del dict[entId]
            del self.entId2specDict[entId]

        def removeZoneReferences(self, removedZoneNums):
            """call with a list of zoneNums of zone entities that have just
            been removed; will clean up references to those zones"""
            assert self.hasEntityTypeReg()
            # get dict of entType->entIds, for ALL scenarios
            type2ids = self.getEntType2ids(self.getAllEntIdsFromAllScenarios())
            # figure out which entity types have attributes that need to be
            # updated
            for type in type2ids:
                typeDesc = self.entTypeReg.getTypeDesc(type)
                visZoneListAttribs = typeDesc.getAttribsOfType('visZoneList')
                if len(visZoneListAttribs) > 0:
                    # this entity type has at least one attrib of type
                    # 'visZoneList'.
                    # run through all of the existing entities of this type
                    for entId in type2ids[type]:
                        spec = self.getEntitySpec(entId)
                        # for each attrib of type 'visZoneList'...
                        for attribName in visZoneListAttribs:
                            # remove each of the removed zoneNums
                            for zoneNum in removedZoneNums:
                                while zoneNum in spec[attribName]:
                                    spec[attribName].remove(zoneNum)

        def getSpecImportsModuleName(self):
            # name of module that should be imported by spec py file
            return 'SpecImports'

        def getFilename(self):
            return self.filename

        def privGetBackupFilename(self, filename):
            return '%s.bak' % filename

        def saveToDisk(self, filename=None, makeBackup=1):
            """returns zero on failure"""
            if filename is None:
                filename = self.filename

            if makeBackup and self.privFileExists(filename):
                # create a backup
                try:
                    backupFilename = self.privGetBackupFilename(filename)
                    self.privRemoveFile(backupFilename)
                    os.rename(filename, backupFilename)
                except OSError, e:
                    LevelSpec.notify.warning(
                        'error during backup: %s' % str(e))

            LevelSpec.notify.info("writing to '%s'" % filename)
            self.privRemoveFile(filename)
            self.privSaveToDisk(filename)

        def privSaveToDisk(self, filename):
            """internal. saves spec to file. returns zero on failure"""
            retval = 1
            # wb to create a UNIX-format file
            f = file(filename, 'wb')
            try:
                f.write(self.getPrettyString())
            except IOError:
                retval = 0
            f.close()
            return retval

        def privFileExists(self, filename):
            try:
                os.stat(filename)
                return 1
            except OSError:
                return 0

        def privRemoveFile(self, filename):
            try:
                os.remove(filename)
                return 1
            except OSError:
                return 0

        def getPrettyString(self):
            """Returns a string that contains the spec data, nicely formatted.
            This should be used when writing the spec out to file."""
            import pprint
            
            tabWidth = 4
            tab = ' ' * tabWidth
            # structure names
            globalEntitiesName = 'GlobalEntities'
            scenarioEntitiesName = 'Scenario%s'
            scenarioWeightName = 'Scenarios'
            topLevelName = 'levelSpec'
            def getPrettyEntityDictStr(name, dict, tabs=0):
                def t(n):
                    return (tabs+n)*tab
                def sortList(lst, firstElements=[]):
                    """sort list; elements in firstElements will be put
                    first, in the order that they appear in firstElements;
                    rest of elements will follow, sorted"""
                    elements = list(lst)
                    # put elements in order
                    result = []
                    for el in firstElements:
                        if el in elements:
                            result.append(el)
                            elements.remove(el)
                    elements.sort()
                    result.extend(elements)
                    return result
   
                firstTypes = ('levelMgr', 'editMgr', 'zone',)
                firstAttribs = ('type', 'name', 'comment', 'parentEntId',
                                'pos', 'x', 'y', 'z',
                                'hpr', 'h', 'p', 'r',
                                'scale', 'sx', 'sy', 'sz',
                                'color',
                                'model',
                                )
                str = t(0)+'%s = {\n' % name
                # get list of types
                entIds = dict.keys()
                entType2ids = self.getEntType2ids(entIds)
                # put types in order
                types = sortList(entType2ids.keys(), firstTypes)
                for type in types:
                    str += t(1)+'# %s\n' % string.upper(type)
                    entIds = entType2ids[type]
                    entIds.sort()
                    for entId in entIds:
                        str += t(1)+'%s: {\n' % entId
                        spec = dict[entId]
                        attribs = sortList(spec.keys(), firstAttribs)
                        for attrib in attribs:
                            str += t(2)+"'%s': %s,\n" % (attrib,
                                                         repr(spec[attrib]))
                        # maybe this will help with CVS merges?
                        str += t(2)+'}, # end entity %s\n' % entId
                        
                str += t(1)+'}\n'
                return str
            def getPrettyScenarioWeightTableStr(tabs=0, self=self):
                def t(n):
                    return (tabs+n)*tab
                str  = t(0)+'%s = [\n' % scenarioWeightName
                for i in range(self.getNumScenarios()):
                    str += t(1)+'[%s, %s],\n' % (scenarioEntitiesName % i,
                                                  self.getScenarioWeights()[i])
                str += t(1)+']\n'
                return str
            def getPrettyTopLevelDictStr(tabs=0):
                def t(n):
                    return (tabs+n)*tab
                str  = t(0)+'%s = {\n' % topLevelName
                str += t(1)+"'globalEntities': %s,\n" % globalEntitiesName
                str += t(1)+"'scenarios': %s,\n" % scenarioWeightName
                str += t(1)+'}\n'
                return str
            
            str  = 'from %s import *\n' % self.getSpecImportsModuleName()
            str += '\n'

            # add the global entities
            str += getPrettyEntityDictStr('GlobalEntities',
                                          self.privGetGlobalEntityDict())
            str += '\n'

            # add the scenario entities
            numScenarios = self.getNumScenarios()
            for i in range(numScenarios):
                str += getPrettyEntityDictStr('Scenario%s' % i,
                                              self.privGetScenarioEntityDict(i))
                str += '\n'

            # add the scenario weight table
            str += getPrettyScenarioWeightTableStr()
            str += '\n'

            # add the top-level table
            str += getPrettyTopLevelDictStr()

            self.testPrettyString(prettyString=str)

            return str

        def testPrettyString(self, prettyString=None):
            # execute the pretty output in our local scope
            if prettyString is None:
                prettyString=self.getPrettyString()
            exec(prettyString)
            assert levelSpec == self.specDict, (
                'LevelSpec pretty string does not match spec data.\n'
                'pretty=%s\n'
                'specData=%s' %
                (levelSpec, self.specDict)
                )

        def checkSpecIntegrity(self):
            # make sure there are no duplicate entIds
            entIds = self.getGlobalEntIds()
            assert uniqueElements(entIds)
            entIds = list2dict(entIds)
            for i in range(self.getNumScenarios()):
                for id in self.getScenarioEntIds(i):
                    assert not entIds.has_key(id)
                    entIds[id] = None

            if self.entTypeReg is not None:
                # check each spec
                allEntIds = entIds
                for entId in allEntIds:
                    spec = self.getEntitySpec(entId)

                    assert spec.has_key('type')
                    entType = spec['type']
                    typeDesc = self.entTypeReg.getTypeDesc(entType)
                    attribNames = typeDesc.getAttribNames()
                    attribDescs = typeDesc.getAttribDescDict()

                    # are there any unknown attribs in the spec?
                    for attrib in spec.keys():
                        if attrib not in attribNames:
                            LevelSpec.notify.warning(
                                "entId %s (%s): unknown attrib '%s', omitting"
                                % (entId, spec['type'], attrib))
                            del spec[attrib]

                    # does the spec have all of its attributes?
                    for attribName in attribNames:
                        if not spec.has_key(attribName):
                            default = attribDescs[attribName].getDefaultValue()
                            LevelSpec.notify.warning(
                                "entId %s (%s): missing attrib '%s', setting "
                                "to default (%s)" % (entId, spec['type'],
                                                     attribName, repr(default)))
                            spec[attribName] = default

        def __hash__(self):
            return hash(repr(self))

        def __str__(self):
            return 'LevelSpec'

        def __repr__(self):
            return 'LevelSpec(%s, scenario=%s)' % (repr(self.specDict),
                                                   self.scenario)
