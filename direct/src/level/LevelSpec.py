"""LevelSpec module: contains the LevelSpec class"""

import DirectNotifyGlobal
from PythonUtil import list2dict, uniqueElements
import string

class LevelSpec:
    """contains spec data for a level, is responsible for handing the data
    out upon request, as well as recording changes made during editing, and
    saving out modified spec data"""
    notify = DirectNotifyGlobal.directNotify.newCategory("LevelSpec")
    
    def __init__(self, specDict, scenario=0):
        self.specDict = specDict

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
        return self.getGlobalEntIds() + self.getScenarioEntIds()

    def getEntitySpec(self, entId):
        assert entId in self.entId2specDict
        specDict = self.entId2specDict[entId]
        return specDict[entId]

    def getEntityType(self, entId):
        return self.getEntitySpec(entId)['type']

    def getEntType2ids(self, entIds):
        """given list of entIds, return dict of entType 2 entIds"""
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

    if __debug__:
        def setLevel(self, level):
            self.level = level

        def setEntityTypeReg(self, entTypeReg):
            self.entTypeReg = entTypeReg
            self.checkSpecIntegrity()

        def hasEntityTypeReg(self):
            return hasattr(self, 'entTypeReg')

        def setFilename(self, filename):
            self.filename = filename

        def setAttribChange(self, entId, attrib, value):
            """ we're being asked to change an attribute """
            LevelSpec.notify.debug("setAttribChange: %s, %s = '%s'" %
                                   (entId, attrib, value))
            assert entId in self.entId2specDict
            specDict = self.entId2specDict[entId]
            assert specDict[entId].has_key(attrib)
            specDict[entId][attrib] = value
            # let the level know that this attribute value has
            # officially changed
            self.level.handleAttribChange(entId, attrib, value)

        def insertEntity(self, entId, entType, parentEntId):
            LevelSpec.notify.debug('inserting entity %s' % entId)
            assert entId not in self.entId2specDict
            assert self.entTypeReg is not None
            globalEnts = self.privGetGlobalEntityDict()
            self.entId2specDict[entId] = globalEnts

            # create a new entity spec entry w/ default values
            globalEnts[entId] = {}
            spec = globalEnts[entId]
            attribDescs = self.entTypeReg.getAttribDescDict(entType)
            for name, desc in attribDescs.items():
                spec[name] = desc.getDefaultValue()
            spec['type'] = entType
            if 'parent' in spec:
                spec['parent'] = parentEntId

            # notify the level
            self.level.handleEntityInsert(entId)
            
        def removeEntity(self, entId):
            LevelSpec.notify.debug('removing entity %s' % entId)
            assert entId in self.entId2specDict
            # notify the level
            self.level.handleEntityRemove(entId)
            # remove the entity's spec
            dict = self.entId2specDict[entId]
            del dict[entId]
            del self.entId2specDict[entId]

        def getSpecImportsModuleName(self):
            # name of module that should be imported by spec py file
            return 'SpecImports'

        def saveToDisk(self, filename=None, createBackup=1):
            """returns zero on failure"""
            import os

            if filename is None:
                filename = self.filename

            if createBackup:
                # create a backup
                try:
                    # does the file exist?
                    exists = 0
                    try:
                        os.stat(filename)
                        exists = 1
                    except OSError:
                        pass
                    if exists:
                        def getBackupFilename(num, filename=filename):
                            return '%s.%03i' % (filename, num)
                        numBackups = 200
                        try:
                            os.unlink(getBackupFilename(numBackups-1))
                        except OSError:
                            pass
                        for i in range(numBackups-1,0,-1):
                            try:
                                os.rename(getBackupFilename(i-1),
                                          getBackupFilename(i))
                            except OSError:
                                pass
                        os.rename(filename, getBackupFilename(0))
                except OSError, e:
                    LevelSpec.notify.warning(
                        'error during backup: %s' % str(e))

            retval = 1
            # wb to create a UNIX-format file
            f = file(filename, 'wb')
            try:
                f.write(self.getPrettyString())
            except IOError:
                retval = 0
            f.close()
            return retval

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
                firstAttribs = ('type', 'name', 'comment', 'parent',
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
                        str += t(2)+'},\n'
                        
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
                    attribNames = self.entTypeReg.getAttribNames(entType)
                    attribDescs = self.entTypeReg.getAttribDescDict(entType)

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
