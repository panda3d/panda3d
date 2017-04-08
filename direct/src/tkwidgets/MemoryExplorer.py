from direct.showbase.DirectObject import DirectObject
from direct.showbase.TkGlobal import *
from .Tree import *
import Pmw

#--------------------------------------------------------------------------
#--------------------------------------------------------------------------
DEFAULT_BT_WIDTH = 50.0

#--------------------------------------------------------------------------
#--------------------------------------------------------------------------
class MemoryExplorer(Pmw.MegaWidget, DirectObject):

    #--------------------------------------------------------------------------
    # Init
    #--------------------------------------------------------------------------
    def __init__(self, parent = None, nodePath = None, **kw):
        if nodePath is None:
            nodePath = render

        optiondefs = (('menuItems',   [],   Pmw.INITOPT),)
        self.defineoptions(kw, optiondefs)
        Pmw.MegaWidget.__init__(self, parent)

        self.nodePath = nodePath
        self.renderItem = None
        self.render2dItem = None

        self.buttons = []
        self.labels = []
        self.rootItem = None

        self.btWidth = DEFAULT_BT_WIDTH

        self.createScrolledFrame()
        self.createScale()
        self.createRefreshBT()

        self.balloon = Pmw.Balloon(self.interior())

    def createScrolledFrame(self):
        self.frame = Pmw.ScrolledFrame(self.interior(),
                                       labelpos = 'n',
                                       label_text = 'ScrolledFrame',
                                       usehullsize = 1,
                                       hull_width = 200,
                                       hull_height = 220,)

        self.frame.pack(padx = 3, pady = 3, fill = BOTH, expand = 1)

    def createScale(self):
        self.scaleCtrl = Scale(self.interior(),
                               label = "Graph Scale",
                               from_= 0.0,
                               to = 20.0,
                               resolution = 0.1,
                               orient = HORIZONTAL,
                               command = self.onScaleUpdate)

        self.scaleCtrl.pack(side = LEFT, fill = BOTH, expand = 1)
        self.scaleCtrl.set(0.0)

    def createRefreshBT(self):
        self.refreshBT = Button(self.interior(), text = 'Refresh', command = self.refresh)
        self.refreshBT.pack(side = LEFT, fill = BOTH, expand = 1)

    #--------------------------------------------------------------------------
    # Item Ctrls
    #--------------------------------------------------------------------------
    def createDefaultCtrls(self):
        if self.renderItem == None or self.render2dItem == None:
            return

        totalBytes = self.renderItem.getVertexBytes()+self.render2dItem.getVertexBytes()

        self.addChildCtrl(self.renderItem, totalBytes)
        self.addChildCtrl(self.render2dItem, totalBytes)

        self.setTitle("ALL", totalBytes)

    def setTitle(self, parent, bytes):
        self.frame["label_text"] = "[%s] - %s bytes" % (parent, bytes)

    def resetCtrls(self):
        for button in self.buttons:
            self.balloon.unbind(button)
            button.destroy()
        self.buttons = []

        for label in self.labels:
            label.destroy()
        self.labels = []

    def getNewButton(self, width, ratio):
        newBT =  Button(self.frame.interior(),
                        anchor = W,
                        width = width)

        if ratio == 0.0:
            newBT['bg'] = "grey"
            newBT['text'] = "."
        else:
            newBT['bg'] = Pmw.Color.hue2name(0.0, 1.0-ratio)
            newBT['text'] = "%0.2f%%" % (ratio*100.0)

        return newBT

    def addSelfCtrl(self, item, totalBytes):
        self.addLabel("[self] : %s bytes" % item.getSelfVertexBytes())

        bt = self.addButton(item.getSelfVertexBytes(),
                            totalBytes,
                            self.onSelfButtonLClick,
                            self.onSelfButtonRClick,
                            item)

    def addChildCtrl(self, item, totalBytes):
        self.addLabel("%s [+%s] : %s bytes" % (item.getName(),
                                                item.getNumChildren(),
                                                item.getVertexBytes()))

        bt = self.addButton(item.getVertexBytes(),
                            totalBytes,
                            self.onChildButtonLClick,
                            self.onChildButtonRClick,
                            item)

    def addButton(self, vertexBytes, totalBytes, funcLClick, funcRClick, item):
        width = self.getBTWidth(vertexBytes, totalBytes)

        if totalBytes == 0:
            ratio = 0.0
        else:
            ratio = vertexBytes/float(totalBytes)

        bt = self.getNewButton(width, ratio)

        def callbackL(event):
            funcLClick(item)

        def callbackR(event):
            funcRClick(item)

        bt.bind("<Button-1>", callbackL)
        bt.bind("<Button-3>", callbackR)

        bt.pack(side = TOP, anchor = NW)
        self.buttons.append(bt)

        self.balloon.bind(bt, item.getPathName())

        return bt

    def addLabel(self, label):
        label = Label(self.frame.interior(), text = label)
        label.pack(side = TOP, anchor = NW,  expand = 0)
        self.labels.append(label)

    def getBTWidth(self, vertexBytes, totalBytes):
        if totalBytes == 0:
            return 1

        width = int(self.btWidth * vertexBytes / totalBytes)

        if width == 0:
            width = 1

        return width

    #--------------------------------------------------------------------------
    # Callback
    #--------------------------------------------------------------------------
    def onScaleUpdate(self, arg):
        self.btWidth = DEFAULT_BT_WIDTH +  DEFAULT_BT_WIDTH * float(arg)

        if self.rootItem:
            self.updateBTWidth()
        else:
            self.updateDefaultBTWidth()

    def updateBTWidth(self):
        self.buttons[0]['width'] = self.getBTWidth(self.rootItem.getSelfVertexBytes(),
                                                   self.rootItem.getVertexBytes())

        btIndex = 1
        for item in self.rootItem.getChildren():
            self.buttons[btIndex]['width'] = self.getBTWidth(item.getVertexBytes(),
                                                             self.rootItem.getVertexBytes())
            btIndex += 1

    def updateDefaultBTWidth(self):
        if self.renderItem == None or self.render2dItem == None:
            return
        totalBytes = self.renderItem.getVertexBytes() + self.render2dItem.getVertexBytes()
        self.buttons[0]['width'] = self.getBTWidth(self.renderItem.getVertexBytes(), totalBytes)
        self.buttons[1]['width'] = self.getBTWidth(self.render2dItem.getVertexBytes(), totalBytes)

    def onSelfButtonLClick(self, item):
        pass

    def onSelfButtonRClick(self, item):
        parentItem = item.getParent()
        self.resetCtrls()
        self.addItemCtrls(parentItem)

    def onChildButtonLClick(self, item):
        if item.getNumChildren() == 0:
            return

        self.resetCtrls()
        self.addItemCtrls(item)

    def onChildButtonRClick(self, item):
        parentItem = item.getParent()

        if parentItem:
            self.resetCtrls()
            self.addItemCtrls(parentItem.getParent())

    def addItemCtrls(self, item):
        self.rootItem = item
        if item == None:
            self.createDefaultCtrls()
        else:
            self.addSelfCtrl(item, item.getVertexBytes())

            for child in item.getChildren():
                self.addChildCtrl(child, item.getVertexBytes())

            self.setTitle(item.getPathName(), item.getVertexBytes())

    #--------------------------------------------------------------------------
    # List & Analyze
    #--------------------------------------------------------------------------
    def makeList(self):
        self.renderItem = MemoryExplorerItem(None, render)
        self.buildList(self.renderItem)

        self.render2dItem = MemoryExplorerItem(None, render2d)
        self.buildList(self.render2dItem)

    def buildList(self, parentItem):
        for nodePath in parentItem.nodePath.getChildren():
            item = MemoryExplorerItem(parentItem, nodePath)
            parentItem.addChild(item)
            self.buildList(item)

    def analyze(self):
        self.renderItem.analyze()
        self.render2dItem.analyze()

    def refresh(self):
        self.makeList()
        self.analyze()

        self.resetCtrls()
        self.createDefaultCtrls()

#--------------------------------------------------------------------------
#--------------------------------------------------------------------------
class MemoryExplorerItem:
    def __init__(self, parent, nodePath):
        self.parent = parent
        self.nodePath = nodePath
        self.children = []

        self.selfVertexBytes = 0
        self.childrenVertexBytes = 0

        self.numFaces = 0
        self.textureBytes = 0

        if parent:
            self.pathName = parent.pathName + "/" +  nodePath.getName()
        else:
            self.pathName = nodePath.getName()

    def getParent(self):
        return self.parent

    def addChild(self, child):
        self.children.append(child)

    def getNumChildren(self):
        return len(self.children)

    def getChildren(self):
        return self.children

    def getName(self):
        return self.nodePath.getName()

    def getPathName(self):
        return self.pathName

    def getVertexBytes(self):
        return self.selfVertexBytes + self.childrenVertexBytes

    def getSelfVertexBytes(self):
        return self.selfVertexBytes

    def analyze(self):
        self.selfVertexBytes = 0
        self.childrenVertexBytes = 0

        self.numFaces = 0
        self.textureBytes = 0

        self.calcTextureBytes()

        if self.nodePath.node().isGeomNode():
            geomNode = self.nodePath.node()

            for i in range(geomNode.getNumGeoms()):
                geom = geomNode.getGeom(i)
                self.calcVertexBytes(geom)
                self.calcNumFaces(geom)

        self.analyzeChildren()

    def calcVertexBytes(self, geom):
        vData = geom.getVertexData()
        for j in range(vData.getNumArrays()):
            array = vData.getArray(j)
            self.selfVertexBytes += array.getDataSizeBytes()

    def calcTextureBytes(self):
        texCol = self.nodePath.findAllTextures()
        for i in range(texCol.getNumTextures()):
            tex = texCol.getTexture(i)
            self.textureBytes += tex.estimateTextureMemory()

            # what about shared textures by multiple nodes ?

    def calcNumFaces(self, geom):
        for k in range(geom.getNumPrimitives()):
            primitive = geom.getPrimitive(k)
            self.numFaces += primitive.getNumFaces()

    def analyzeChildren(self):
        for child in self.children:
            child.analyze()
            self.childrenVertexBytes += child.getVertexBytes()
            self.numFaces += child.numFaces

    def ls(self, indent = ""):
        print(indent + self.nodePath.getName() + " " + str(self.vertexBytes) + " " + str(self.numFaces) + " " + str(self.textureBytes))
        indent = indent +  " "

        for child in self.children:
            child.ls(indent)
