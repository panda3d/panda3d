####################################################################
#Dtool_funcToMethod(func, class)
#del func
#####################################################################

"""
NodePath-extensions module: contains methods to extend functionality
of the NodePath class
"""

from panda3d.core import NodePath
from .extension_native_helpers import Dtool_funcToMethod

####################################################################
def id(self):
        """Deprecated.  Returns a unique id identifying the NodePath instance"""
        print("Warning: NodePath.id() is deprecated.  Use hash(NodePath) or NodePath.get_key() instead.")
        return self.getKey()

Dtool_funcToMethod(id, NodePath)
del id
#####################################################################
def getChildrenAsList(self):
        """Deprecated.  Converts a node path's child NodePathCollection into a list"""
        print("Warning: NodePath.getChildrenAsList() is deprecated.  Use get_children() instead.")
        return list(self.getChildren())

Dtool_funcToMethod(getChildrenAsList, NodePath)
del getChildrenAsList
#####################################################################

def printChildren(self):
        """Deprecated.  Prints out the children of the bottom node of a node path"""
        print("Warning: NodePath.printChildren() is deprecated.")
        for child in self.getChildren():
            print(child.getName())
Dtool_funcToMethod(printChildren, NodePath)
del printChildren
#####################################################################

def removeChildren(self):
        """Deprecated.  Deletes the children of the bottom node of a node path"""
        print("Warning: NodePath.removeChildren() is deprecated.  Use get_children().detach() instead.")
        self.getChildren().detach()
Dtool_funcToMethod(removeChildren, NodePath)
del removeChildren
#####################################################################

def toggleVis(self):
        """Deprecated.  Toggles visibility of a nodePath"""
        print("Warning: NodePath.toggleVis() is deprecated.  Use is_hidden(), show() and hide() instead.")
        if self.isHidden():
            self.show()
            return 1
        else:
            self.hide()
            return 0
Dtool_funcToMethod(toggleVis, NodePath)
del toggleVis
#####################################################################

def showSiblings(self):
        """Deprecated.  Show all the siblings of a node path"""
        print("Warning: NodePath.showSiblings() is deprecated.")
        for sib in self.getParent().getChildren():
            if sib.node() != self.node():
                sib.show()
Dtool_funcToMethod(showSiblings, NodePath)
del showSiblings
#####################################################################

def hideSiblings(self):
        """Deprecated.  Hide all the siblings of a node path"""
        print("Warning: NodePath.hideSiblings() is deprecated.")
        for sib in self.getParent().getChildren():
            if sib.node() != self.node():
                sib.hide()
Dtool_funcToMethod(hideSiblings, NodePath)
del hideSiblings
#####################################################################

def showAllDescendants(self):
        """Deprecated.  Show the node path and all its children"""
        print("Warning: NodePath.showAllDescendants() is deprecated.")
        self.show()
        for child in self.getChildren():
            child.showAllDescendants()
Dtool_funcToMethod(showAllDescendants, NodePath)
del showAllDescendants
#####################################################################

def isolate(self):
        """Deprecated.  Show the node path and hide its siblings"""
        print("Warning: NodePath.isolate() is deprecated.")
        self.showAllDescendants()
        for sib in self.getParent().getChildren():
            if sib.node() != self.node():
                sib.hide()
Dtool_funcToMethod(isolate, NodePath)
del isolate
#####################################################################

def remove(self):
        """Deprecated.  Remove a node path from the scene graph"""
        print("Warning: NodePath.remove() is deprecated.  Use remove_node() instead.")
        # Send message in case anyone needs to do something
        # before node is deleted
        messenger.send('preRemoveNodePath', [self])
        # Remove nodePath
        self.removeNode()
Dtool_funcToMethod(remove, NodePath)
del remove
#####################################################################

def lsNames(self):
        """Deprecated. Walk down a tree and print out the path"""
        print("Warning: NodePath.lsNames() is deprecated.")
        if self.isEmpty():
            print("(empty)")
        else:
            type = self.node().getType().getName()
            name = self.getName()
            print(type + "  " + name)
            self.lsNamesRecurse()

Dtool_funcToMethod(lsNames, NodePath)
del lsNames
#####################################################################
def lsNamesRecurse(self, indentString=' '):
        """Deprecated.  Walk down a tree and print out the path"""
        print("Warning: NodePath.lsNamesRecurse() is deprecated.")
        for nodePath in self.getChildren():
            type = nodePath.node().getType().getName()
            name = nodePath.getName()
            print(indentString + type + "  " + name)
            nodePath.lsNamesRecurse(indentString + " ")

Dtool_funcToMethod(lsNamesRecurse, NodePath)
del lsNamesRecurse
#####################################################################
def reverseLsNames(self):
        """Deprecated.  Walk up a tree and print out the path to the root"""
        print("Warning: NodePath.reverseLsNames() is deprecated.")
        ancestors = list(self.getAncestors())
        ancestry = ancestors.reverse()
        indentString = ""
        for nodePath in ancestry:
            type = nodePath.node().getType().getName()
            name = nodePath.getName()
            print(indentString + type + "  " + name)
            indentString = indentString + " "

Dtool_funcToMethod(reverseLsNames, NodePath)
del reverseLsNames
#####################################################################
def getAncestry(self):
        """Deprecated.  Get a list of a node path's ancestors"""
        print("NodePath.getAncestry() is deprecated.  Use get_ancestors() instead.")
        ancestors = list(self.getAncestors())
        ancestors.reverse()
        return ancestors

Dtool_funcToMethod(getAncestry, NodePath)
del getAncestry
#####################################################################

def pPrintString(self, other = None):
        """
        Deprecated.  pretty print
        """
        print("NodePath.pPrintString() is deprecated.")
        if __debug__:
            # Normally I would have put the if __debug__ around
            # the entire funciton, the that doesn't seem to work
            # with -extensions.  Maybe someone will look into
            # this further.
            if other:
                pos = self.getPos(other)
                hpr = self.getHpr(other)
                scale = self.getScale(other)
                shear = self.getShear(other)
                otherString = "  'other': %s,\n" % (other.getName(),)
            else:
                pos = self.getPos()
                hpr = self.getHpr()
                scale = self.getScale()
                shear = self.getShear()
                otherString = '\n'
            return (
                "%s = {"%(self.getName()) +
                otherString +
                "  'Pos':   (%s),\n" % pos.pPrintValues() +
                "  'Hpr':   (%s),\n" % hpr.pPrintValues() +
                "  'Scale': (%s),\n" % scale.pPrintValues() +
                "  'Shear': (%s),\n" % shear.pPrintValues() +
                "}")
Dtool_funcToMethod(pPrintString, NodePath)
del pPrintString
#####################################################################

def printPos(self, other = None, sd = 2):
        """ Deprecated.  Pretty print a node path's pos """
        print("NodePath.printPos() is deprecated.")
        formatString = '%0.' + '%d' % sd + 'f'
        if other:
            pos = self.getPos(other)
            otherString = other.getName() + ', '
        else:
            pos = self.getPos()
            otherString = ''
        print((self.getName() + '.setPos(' + otherString +
               formatString % pos[0] + ', ' +
               formatString % pos[1] + ', ' +
               formatString % pos[2] +
               ')\n'))
Dtool_funcToMethod(printPos, NodePath)
del printPos
#####################################################################

def printHpr(self, other = None, sd = 2):
        """ Deprecated.  Pretty print a node path's hpr """
        print("NodePath.printHpr() is deprecated.")
        formatString = '%0.' + '%d' % sd + 'f'
        if other:
            hpr = self.getHpr(other)
            otherString = other.getName() + ', '
        else:
            hpr = self.getHpr()
            otherString = ''
        print((self.getName() + '.setHpr(' + otherString +
               formatString % hpr[0] + ', ' +
               formatString % hpr[1] + ', ' +
               formatString % hpr[2] +
               ')\n'))
Dtool_funcToMethod(printHpr, NodePath)
del printHpr
#####################################################################

def printScale(self, other = None, sd = 2):
        """ Deprecated.  Pretty print a node path's scale """
        print("NodePath.printScale() is deprecated.")
        formatString = '%0.' + '%d' % sd + 'f'
        if other:
            scale = self.getScale(other)
            otherString = other.getName() + ', '
        else:
            scale = self.getScale()
            otherString = ''
        print((self.getName() + '.setScale(' + otherString +
               formatString % scale[0] + ', ' +
               formatString % scale[1] + ', ' +
               formatString % scale[2] +
               ')\n'))

Dtool_funcToMethod(printScale, NodePath)
del printScale
#####################################################################
def printPosHpr(self, other = None, sd = 2):
        """ Deprecated.  Pretty print a node path's pos and, hpr """
        print("NodePath.printPosHpr() is deprecated.")
        formatString = '%0.' + '%d' % sd + 'f'
        if other:
            pos = self.getPos(other)
            hpr = self.getHpr(other)
            otherString = other.getName() + ', '
        else:
            pos = self.getPos()
            hpr = self.getHpr()
            otherString = ''
        print((self.getName() + '.setPosHpr(' + otherString +
               formatString % pos[0] + ', ' +
               formatString % pos[1] + ', ' +
               formatString % pos[2] + ', ' +
               formatString % hpr[0] + ', ' +
               formatString % hpr[1] + ', ' +
               formatString % hpr[2] +
               ')\n'))

Dtool_funcToMethod(printPosHpr, NodePath)
del printPosHpr
#####################################################################
def printPosHprScale(self, other = None, sd = 2):
        """ Deprecated.  Pretty print a node path's pos, hpr, and scale """
        print("NodePath.printPosHprScale() is deprecated.")
        formatString = '%0.' + '%d' % sd + 'f'
        if other:
            pos = self.getPos(other)
            hpr = self.getHpr(other)
            scale = self.getScale(other)
            otherString = other.getName() + ', '
        else:
            pos = self.getPos()
            hpr = self.getHpr()
            scale = self.getScale()
            otherString = ''
        print((self.getName() + '.setPosHprScale(' + otherString +
               formatString % pos[0] + ', ' +
               formatString % pos[1] + ', ' +
               formatString % pos[2] + ', ' +
               formatString % hpr[0] + ', ' +
               formatString % hpr[1] + ', ' +
               formatString % hpr[2] + ', ' +
               formatString % scale[0] + ', ' +
               formatString % scale[1] + ', ' +
               formatString % scale[2] +
               ')\n'))

Dtool_funcToMethod(printPosHprScale, NodePath)
del printPosHprScale
#####################################################################

def printTransform(self, other = None, sd = 2, fRecursive = 0):
    "Deprecated."
    print("NodePath.printTransform() is deprecated.")
    from panda3d.core import Vec3
    fmtStr = '%%0.%df' % sd
    name = self.getName()
    if other == None:
        transform = self.getTransform()
    else:
        transform = self.getTransform(other)
    if transform.hasPos():
        pos = transform.getPos()
        if not pos.almostEqual(Vec3(0)):
            outputString = '%s.setPos(%s, %s, %s)' % (name, fmtStr, fmtStr, fmtStr)
            print(outputString % (pos[0], pos[1], pos[2]))
    if transform.hasHpr():
        hpr = transform.getHpr()
        if not hpr.almostEqual(Vec3(0)):
            outputString = '%s.setHpr(%s, %s, %s)' % (name, fmtStr, fmtStr, fmtStr)
            print(outputString % (hpr[0], hpr[1], hpr[2]))
    if transform.hasScale():
        if transform.hasUniformScale():
            scale = transform.getUniformScale()
            if scale != 1.0:
                outputString = '%s.setScale(%s)' % (name, fmtStr)
                print(outputString % scale)
        else:
            scale = transform.getScale()
            if not scale.almostEqual(Vec3(1)):
                outputString = '%s.setScale(%s, %s, %s)' % (name, fmtStr, fmtStr, fmtStr)
                print(outputString % (scale[0], scale[1], scale[2]))
    if fRecursive:
        for child in self.getChildren():
            child.printTransform(other, sd, fRecursive)

Dtool_funcToMethod(printTransform, NodePath)
del printTransform
#####################################################################


def iPos(self, other = None):
        """ Deprecated.  Set node path's pos to 0, 0, 0 """
        print("NodePath.iPos() is deprecated.")
        if other:
            self.setPos(other, 0, 0, 0)
        else:
            self.setPos(0, 0, 0)
Dtool_funcToMethod(iPos, NodePath)
del iPos
#####################################################################

def iHpr(self, other = None):
        """ Deprecated.  Set node path's hpr to 0, 0, 0 """
        print("NodePath.iHpr() is deprecated.")
        if other:
            self.setHpr(other, 0, 0, 0)
        else:
            self.setHpr(0, 0, 0)

Dtool_funcToMethod(iHpr, NodePath)
del iHpr
#####################################################################
def iScale(self, other = None):
        """ Deprecated.  Set node path's scale to 1, 1, 1 """
        print("NodePath.iScale() is deprecated.")
        if other:
            self.setScale(other, 1, 1, 1)
        else:
            self.setScale(1, 1, 1)

Dtool_funcToMethod(iScale, NodePath)
del iScale
#####################################################################
def iPosHpr(self, other = None):
        """ Deprecated.  Set node path's pos and hpr to 0, 0, 0 """
        print("NodePath.iPosHpr() is deprecated.")
        if other:
            self.setPosHpr(other, 0, 0, 0, 0, 0, 0)
        else:
            self.setPosHpr(0, 0, 0, 0, 0, 0)

Dtool_funcToMethod(iPosHpr, NodePath)
del iPosHpr
#####################################################################
def iPosHprScale(self, other = None):
        """ Deprecated.  Set node path's pos and hpr to 0, 0, 0 and scale to 1, 1, 1 """
        print("NodePath.iPosHprScale() is deprecated.")
        if other:
            self.setPosHprScale(other, 0, 0, 0, 0, 0, 0, 1, 1, 1)
        else:
            self.setPosHprScale(0, 0, 0, 0, 0, 0, 1, 1, 1)

    # private methods
Dtool_funcToMethod(iPosHprScale, NodePath)
del iPosHprScale
#####################################################################
def place(self):
        base.startDirect(fWantTk = 1)
        # Don't use a regular import, to prevent ModuleFinder from picking
        # it up as a dependency when building a .p3d package.
        import importlib
        Placer = importlib.import_module('direct.tkpanels.Placer')
        return Placer.place(self)

Dtool_funcToMethod(place, NodePath)
del place
#####################################################################
def explore(self):
        base.startDirect(fWantTk = 1)
        # Don't use a regular import, to prevent ModuleFinder from picking
        # it up as a dependency when building a .p3d package.
        import importlib
        SceneGraphExplorer = importlib.import_module('direct.tkwidgets.SceneGraphExplorer')
        return SceneGraphExplorer.explore(self)

Dtool_funcToMethod(explore, NodePath)
del explore
#####################################################################
def rgbPanel(self, cb = None):
        base.startTk()
        # Don't use a regular import, to prevent ModuleFinder from picking
        # it up as a dependency when building a .p3d package.
        import importlib
        Valuator = importlib.import_module('direct.tkwidgets.Valuator')
        return Valuator.rgbPanel(self, cb)

Dtool_funcToMethod(rgbPanel, NodePath)
del rgbPanel
#####################################################################
def select(self):
        base.startDirect(fWantTk = 0)
        base.direct.select(self)

Dtool_funcToMethod(select, NodePath)
del select
#####################################################################
def deselect(self):
        base.startDirect(fWantTk = 0)
        base.direct.deselect(self)

Dtool_funcToMethod(deselect, NodePath)
del deselect
#####################################################################
def showCS(self, mask = None):
        """
        Deprecated.
        Shows the collision solids at or below this node.  If mask is
        not None, it is a BitMask32 object (e.g. WallBitmask,
        CameraBitmask) that indicates which particular collision
        solids should be made visible; otherwise, all of them will be.
        """
        print("NodePath.showCS() is deprecated.  Use findAllMatches('**/+CollisionNode').show() instead.")
        npc = self.findAllMatches('**/+CollisionNode')
        for p in range(0, npc.getNumPaths()):
            np = npc[p]
            if (mask == None or (np.node().getIntoCollideMask() & mask).getWord()):
                np.show()

Dtool_funcToMethod(showCS, NodePath)
del showCS
#####################################################################
def hideCS(self, mask = None):
        """
        Deprecated.
        Hides the collision solids at or below this node.  If mask is
        not None, it is a BitMask32 object (e.g. WallBitmask,
        CameraBitmask) that indicates which particular collision
        solids should be hidden; otherwise, all of them will be.
        """
        print("NodePath.hideCS() is deprecated.  Use findAllMatches('**/+CollisionNode').hide() instead.")
        npc = self.findAllMatches('**/+CollisionNode')
        for p in range(0, npc.getNumPaths()):
            np = npc[p]
            if (mask == None or (np.node().getIntoCollideMask() & mask).getWord()):
                np.hide()

Dtool_funcToMethod(hideCS, NodePath)
del hideCS
#####################################################################
def posInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpPosInterval(self, *args, **kw)

Dtool_funcToMethod(posInterval, NodePath)
del posInterval
#####################################################################
def hprInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpHprInterval(self, *args, **kw)

Dtool_funcToMethod(hprInterval, NodePath)
del hprInterval
#####################################################################
def quatInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpQuatInterval(self, *args, **kw)

Dtool_funcToMethod(quatInterval, NodePath)
del quatInterval
#####################################################################
def scaleInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpScaleInterval(self, *args, **kw)

Dtool_funcToMethod(scaleInterval, NodePath)
del scaleInterval
#####################################################################
def shearInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpShearInterval(self, *args, **kw)

Dtool_funcToMethod(shearInterval, NodePath)
del shearInterval
#####################################################################
def posHprInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpPosHprInterval(self, *args, **kw)

Dtool_funcToMethod(posHprInterval, NodePath)
del posHprInterval
#####################################################################
def posQuatInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpPosQuatInterval(self, *args, **kw)

Dtool_funcToMethod(posQuatInterval, NodePath)
del posQuatInterval
#####################################################################
def hprScaleInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpHprScaleInterval(self, *args, **kw)

Dtool_funcToMethod(hprScaleInterval, NodePath)
del hprScaleInterval
#####################################################################
def quatScaleInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpQuatScaleInterval(self, *args, **kw)

Dtool_funcToMethod(quatScaleInterval, NodePath)
del quatScaleInterval
#####################################################################
def posHprScaleInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpPosHprScaleInterval(self, *args, **kw)

Dtool_funcToMethod(posHprScaleInterval, NodePath)
del posHprScaleInterval
#####################################################################
def posQuatScaleInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpPosQuatScaleInterval(self, *args, **kw)

Dtool_funcToMethod(posQuatScaleInterval, NodePath)
del posQuatScaleInterval
#####################################################################
def posHprScaleShearInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpPosHprScaleShearInterval(self, *args, **kw)

Dtool_funcToMethod(posHprScaleShearInterval, NodePath)
del posHprScaleShearInterval
#####################################################################
def posQuatScaleShearInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpPosQuatScaleShearInterval(self, *args, **kw)

Dtool_funcToMethod(posQuatScaleShearInterval, NodePath)
del posQuatScaleShearInterval
#####################################################################
def colorInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpColorInterval(self, *args, **kw)

Dtool_funcToMethod(colorInterval, NodePath)
del colorInterval
#####################################################################
def colorScaleInterval(self, *args, **kw):
        from direct.interval import LerpInterval
        return LerpInterval.LerpColorScaleInterval(self, *args, **kw)

Dtool_funcToMethod(colorScaleInterval, NodePath)
del colorScaleInterval
#####################################################################
def attachCollisionSphere(self, name, cx, cy, cz, r, fromCollide, intoCollide):
        from panda3d.core import CollisionSphere
        from panda3d.core import CollisionNode
        coll = CollisionSphere(cx, cy, cz, r)
        collNode = CollisionNode(name)
        collNode.addSolid(coll)
        collNode.setFromCollideMask(fromCollide)
        collNode.setIntoCollideMask(intoCollide)
        collNodePath = self.attachNewNode(collNode)
        return collNodePath

Dtool_funcToMethod(attachCollisionSphere, NodePath)
del attachCollisionSphere
#####################################################################
def attachCollisionSegment(self, name, ax, ay, az, bx, by, bz, fromCollide, intoCollide):
        from panda3d.core import CollisionSegment
        from panda3d.core import CollisionNode
        coll = CollisionSegment(ax, ay, az, bx, by, bz)
        collNode = CollisionNode(name)
        collNode.addSolid(coll)
        collNode.setFromCollideMask(fromCollide)
        collNode.setIntoCollideMask(intoCollide)
        collNodePath = self.attachNewNode(collNode)
        return collNodePath

Dtool_funcToMethod(attachCollisionSegment, NodePath)
del attachCollisionSegment
#####################################################################
def attachCollisionRay(self, name, ox, oy, oz, dx, dy, dz, fromCollide, intoCollide):
        from panda3d.core import CollisionRay
        from panda3d.core import CollisionNode
        coll = CollisionRay(ox, oy, oz, dx, dy, dz)
        collNode = CollisionNode(name)
        collNode.addSolid(coll)
        collNode.setFromCollideMask(fromCollide)
        collNode.setIntoCollideMask(intoCollide)
        collNodePath = self.attachNewNode(collNode)
        return collNodePath

Dtool_funcToMethod(attachCollisionRay, NodePath)
del attachCollisionRay
#####################################################################
def flattenMultitex(self, stateFrom = None, target = None,
                        useGeom = 0, allowTexMat = 0, win = None):
        from panda3d.core import MultitexReducer
        mr = MultitexReducer()
        if target != None:
            mr.setTarget(target)
        mr.setUseGeom(useGeom)
        mr.setAllowTexMat(allowTexMat)

        if win == None:
            win = base.win

        if stateFrom == None:
            mr.scan(self)
        else:
            mr.scan(self, stateFrom)
        mr.flatten(win)
Dtool_funcToMethod(flattenMultitex, NodePath)
del flattenMultitex
#####################################################################
def getNumDescendants(self):
        return len(self.findAllMatches('**')) - 1
Dtool_funcToMethod(getNumDescendants, NodePath)
del getNumDescendants
#####################################################################
def removeNonCollisions(self):
        # remove anything that is not collision-related
        print("NodePath.removeNonCollisions() is deprecated")
        stack = [self]
        while len(stack):
                np = stack.pop()
                # if there are no CollisionNodes under this node, remove it
                if np.find('**/+CollisionNode').isEmpty():
                        np.detachNode()
                else:
                        stack.extend(np.getChildren())
Dtool_funcToMethod(removeNonCollisions, NodePath)
del removeNonCollisions
#####################################################################

def subdivideCollisions(self, numSolidsInLeaves):
        """
        expand CollisionNodes out into balanced trees, with a particular number
        of solids in the leaves
        TODO: better splitting logic at each level of the tree wrt spatial separation
        and cost of bounding volume tests vs. cost of collision solid tests
        """
        colNps = self.findAllMatches('**/+CollisionNode')
        for colNp in colNps:
            node = colNp.node()
            numSolids = node.getNumSolids()
            if numSolids <= numSolidsInLeaves:
                # this CollisionNode doesn't need to be split
                continue
            solids = []
            for i in range(numSolids):
                solids.append(node.getSolid(i))
            # recursively subdivide the solids into a spatial binary tree
            solidTree = self.r_subdivideCollisions(solids, numSolidsInLeaves)
            root = colNp.getParent().attachNewNode('%s-subDivRoot' % colNp.getName())
            self.r_constructCollisionTree(solidTree, root, colNp.getName())
            colNp.stash()

def r_subdivideCollisions(self, solids, numSolidsInLeaves):
        # takes a list of solids, returns a list containing some number of lists,
        # with the solids evenly distributed between them (recursively nested until
        # the lists at the leaves contain no more than numSolidsInLeaves)
        # if solids is already small enough, returns solids unchanged
        if len(solids) <= numSolidsInLeaves:
            return solids
        origins = []
        avgX = 0; avgY = 0; avgZ = 0
        minX = None; minY = None; minZ = None
        maxX = None; maxY = None; maxZ = None
        for solid in solids:
            origin = solid.getCollisionOrigin()
            origins.append(origin)
            x = origin.getX(); y = origin.getY(); z = origin.getZ()
            avgX += x; avgY += y; avgZ += z
            if minX is None:
                minX = x; minY = y; minZ = z
                maxX = x; maxY = y; maxZ = z
            else:
                minX = min(x, minX); minY = min(y, minY); minZ = min(z, minZ)
                maxX = max(x, maxX); maxY = max(y, maxY); maxZ = max(z, maxZ)
        avgX /= len(solids); avgY /= len(solids); avgZ /= len(solids)
        extentX = maxX - minX; extentY = maxY - minY; extentZ = maxZ - minZ
        maxExtent = max(max(extentX, extentY), extentZ)
        # sparse octree
        xyzSolids = []
        XyzSolids = []
        xYzSolids = []
        XYzSolids = []
        xyZSolids = []
        XyZSolids = []
        xYZSolids = []
        XYZSolids = []
        midX = avgX
        midY = avgY
        midZ = avgZ
        # throw out axes that are not close to the max axis extent; try and keep
        # the divisions square/spherical
        if extentX < (maxExtent * .75) or extentX > (maxExtent * 1.25):
                midX += maxExtent
        if extentY < (maxExtent * .75) or extentY > (maxExtent * 1.25):
                midY += maxExtent
        if extentZ < (maxExtent * .75) or extentZ > (maxExtent * 1.25):
                midZ += maxExtent
        for i in range(len(solids)):
                origin = origins[i]
                x = origin.getX(); y = origin.getY(); z = origin.getZ()
                if x < midX:
                        if y < midY:
                                if z < midZ:
                                        xyzSolids.append(solids[i])
                                else:
                                        xyZSolids.append(solids[i])
                        else:
                                if z < midZ:
                                        xYzSolids.append(solids[i])
                                else:
                                        xYZSolids.append(solids[i])
                else:
                        if y < midY:
                                if z < midZ:
                                        XyzSolids.append(solids[i])
                                else:
                                        XyZSolids.append(solids[i])
                        else:
                                if z < midZ:
                                        XYzSolids.append(solids[i])
                                else:
                                        XYZSolids.append(solids[i])
        newSolids = []
        if len(xyzSolids):
                newSolids.append(self.r_subdivideCollisions(xyzSolids, numSolidsInLeaves))
        if len(XyzSolids):
                newSolids.append(self.r_subdivideCollisions(XyzSolids, numSolidsInLeaves))
        if len(xYzSolids):
                newSolids.append(self.r_subdivideCollisions(xYzSolids, numSolidsInLeaves))
        if len(XYzSolids):
                newSolids.append(self.r_subdivideCollisions(XYzSolids, numSolidsInLeaves))
        if len(xyZSolids):
                newSolids.append(self.r_subdivideCollisions(xyZSolids, numSolidsInLeaves))
        if len(XyZSolids):
                newSolids.append(self.r_subdivideCollisions(XyZSolids, numSolidsInLeaves))
        if len(xYZSolids):
                newSolids.append(self.r_subdivideCollisions(xYZSolids, numSolidsInLeaves))
        if len(XYZSolids):
                newSolids.append(self.r_subdivideCollisions(XYZSolids, numSolidsInLeaves))
        #import pdb;pdb.set_trace()
        return newSolids

def r_constructCollisionTree(self, solidTree, parentNode, colName):
        from panda3d.core import CollisionNode
        for item in solidTree:
            if type(item[0]) == type([]):
                newNode = parentNode.attachNewNode('%s-branch' % colName)
                self.r_constructCollisionTree(item, newNode, colName)
            else:
                cn = CollisionNode('%s-leaf' % colName)
                for solid in item:
                    cn.addSolid(solid)
                parentNode.attachNewNode(cn)

Dtool_funcToMethod(subdivideCollisions, NodePath)
Dtool_funcToMethod(r_subdivideCollisions, NodePath)
Dtool_funcToMethod(r_constructCollisionTree, NodePath)
del subdivideCollisions
del r_subdivideCollisions
del r_constructCollisionTree

#####################################################################
def analyze(self):
        """
        Analyzes the geometry below this node and reports the
        number of vertices, triangles, etc.  This is the same
        information reported by the bam-info program.
        """
        from panda3d.core import SceneGraphAnalyzer
        sga = SceneGraphAnalyzer()
        sga.addNode(self.node())
        if sga.getNumLodNodes() == 0:
                print(sga)
        else:
                print("At highest LOD:")
                sga2 = SceneGraphAnalyzer()
                sga2.setLodMode(sga2.LMHighest)
                sga2.addNode(self.node())
                print(sga2)

                print("\nAt lowest LOD:")
                sga2.clear()
                sga2.setLodMode(sga2.LMLowest)
                sga2.addNode(self.node())
                print(sga2)

                print("\nAll nodes:")
                print(sga)

Dtool_funcToMethod(analyze, NodePath)
del analyze
#####################################################################
