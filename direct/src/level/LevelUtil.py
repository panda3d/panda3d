"""LevelUtil module: contains Level utility funcs"""

import string

def getZoneNum2Node(levelModel):
    """ given model, returns dict of ZoneNumber -> ZoneNode """
    def findNumberedNodes(baseString, model):
        # finds nodes whose name follows the pattern 'baseString#blah'
        # returns dictionary that maps # to node
        potentialNodes = model.findAllMatches(
            '**/%s*' % baseString).asList()
        num2node = {}
        for potentialNode in potentialNodes:
            name = potentialNode.getName()
            print 'potential match for %s: %s' % (baseString, name)
            name = name[len(baseString):]
            numDigits = 0
            while numDigits < len(name):
                if name[numDigits] not in string.digits:
                    break
                numDigits += 1
            if numDigits == 0:
                continue
            num = int(name[:numDigits])
            # do we already have a ZoneNode for this zone num?
            assert not num in num2node
            num2node[num] = potentialNode

        return num2node

    zoneNum2node = findNumberedNodes('Zone', levelModel)
    # temp
    zoneNum2node.update(findNumberedNodes('ZONE', levelModel))
    # add the UberZone to the table
    zoneNum2node[0] = levelModel
    return zoneNum2node
