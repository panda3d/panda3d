"""LevelUtil module: contains Level utility funcs"""

def getZoneNum2Node(levelModel):
    """ given model, returns dict of ZoneNumber -> ZoneNode """
    def findNumberedNodes(baseString, model):
        # finds nodes whose name follows the pattern 'baseString#'
        # where there are no characters after #
        # returns dictionary that maps # to node
        potentialNodes = model.findAllMatches(
            '**/%s*' % baseString).asList()
        num2node = {}
        for potentialNode in potentialNodes:
            name = potentialNode.getName()
            print 'potential match for %s: %s' % (baseString, name)
            try:
                num = int(name[len(baseString):])
            except:
                continue

            num2node[num] = potentialNode

        return num2node

    zoneNum2node = findNumberedNodes('Zone', levelModel)
    # add the UberZone to the table
    zoneNum2node[0] = levelModel
    return zoneNum2node
