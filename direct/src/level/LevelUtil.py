"""LevelUtil module: contains Level utility funcs"""

import string

def getZoneNum2Node(levelModel):
    """ given model, returns dict of ZoneNumber -> ZoneNode """
    def findNumberedNodes(baseString, model, caseInsens=1):
        # finds nodes whose name follows the pattern 'baseString#blah'
        # returns dictionary that maps # to node
        srch = '**/%s*' % baseString
        if caseInsens:
            srch += ';+i'
        potentialNodes = model.findAllMatches(srch).asList()
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
            if num in num2node:
                print 'warning: zone %s already assigned to %s. ignoring %s' % (
                    num, num2node[num], potentialNode)
            num2node[num] = potentialNode

        return num2node

    zoneNum2node = findNumberedNodes('zone', levelModel)
    # add the UberZone to the table
    zoneNum2node[0] = levelModel
    return zoneNum2node
