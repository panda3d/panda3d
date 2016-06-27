"""
This is the module for curve edit
"""

from pandac.PandaModules import *
from direct.wxwidgets.WxPandaShell import *
from direct.showbase.DirectObject import *
from direct.directtools.DirectSelection import SelectionRay
from direct.showutil.Rope import Rope
from .ActionMgr import *
from direct.task import Task


class CurveEditor(DirectObject):
    """ CurveEditor will create and edit the curve """
    def __init__(self, editor):
        self.editor = editor
        self.i = 0
        self.ropeNum = 0
        self.curve = []
        self.curveControl = []
        self.currentRope = None
        self.degree = 3

    def createCurve(self):
        if self.editor.mode == self.editor.CREATE_CURVE_MODE:
            self.view = self.editor.ui.currentView

            #Get the mouse position
            x = base.direct.dr.mouseX
            y = base.direct.dr.mouseY

            if self.editor.fMoveCamera == False and self.view != None:
                self.createControler(x,y)
                if self.currentRope != None:
                    self.currentRope.detachNode()
                self.ropeUpdate(self.curve)
                self.accept("DIRECT-enter", self.onBaseMode)

            self.accept("DIRECT-enter", self.onBaseMode)

    def editCurve(self, task):
        if self.editor.mode == self.editor.EDIT_CURVE_MODE:
            if self.editor.fMoveCamera == False:
                self.selected = None
                self.selected = base.direct.selected.last
                if self.selected != None:
                    for item in self.curveControl:
                        if item[1] == self.selected:
                            self.point = item  #temporarily save the controler information for further use
                            self.currentCurve = self.currentRope.ropeNode.getCurve()
                            self.currentCurve.setVertex(item[0], self.selected.getPos())
                            self.accept("DIRECT-delete", self.onControlerDelete)
                            return task.cont

    def onControlerDelete(self):
        if self.editor.mode == self.editor.EDIT_CURVE_MODE:
            self.curve.remove(self.curve[self.point[0]])
            #reset the controller list
            for item in self.curveControl:
                if item[0] > self.point[0]:
                    newname = 'controler%d' % (item[0]-1)
                    item[1].setName(newname)
                    self.curveControl[item[0]] = (item[0]-1, item[1])
            self.curveControl.remove(self.point)
            self.currentRope.setup(self.degree,self.curve)

    def ropeUpdate(self, curve):
        self.currentRope = Rope()
        self.currentRope.setup(self.degree, curve)
        self.currentRope.reparentTo(render)

    def onBaseMode(self):
        self.editor.preMode = self.editor.mode
        self.editor.mode = self.editor.BASE_MODE
        self.editor.ui.editCurveMenuItem.Check(False)
        self.editor.ui.createCurveMenuItem.Check(False)
        self.i = 0
        for item in self.curveControl:
            item[1].hide()
        if self.editor.preMode == self.editor.BASE_MODE :
            pass
        if self.editor.preMode == self.editor.CREATE_CURVE_MODE :
            self.updateScene()
        if self.editor.preMode == self.editor.EDIT_CURVE_MODE :
            self.doneEdit()
        self.curveControl = []
        self.curve = []
        self.currentRope = None
        base.direct.manipulationControl.enableManipulation()
        self.editor.ui.createCurveMenuItem.Check(False)
        self.editor.ui.editCurveMenuItem.Check(False)

    def updateScene(self):
        curveObjNP = self.editor.objectMgr.addNewCurve(self.curveControl, self.degree, nodePath=self.currentRope)
        curveObj = self.editor.objectMgr.findObjectByNodePath(curveObjNP)
        for item in self.curveControl:
            item[1].reparentTo(curveObjNP)
        self.editor.objectMgr.updateObjectPropValue(curveObj, 'Degree', self.degree, fSelectObject=False, fUndo=False)

    def doneEdit(self):
        base.direct.selected.last = None

    def createControler(self, x, y):
        if self.view != None:
            self.controler = render.attachNewNode("controler")
            self.controler = loader.loadModel('models/misc/smiley')
            controlerPathname = 'controler%d' % self.i
            self.controler.setName(controlerPathname)
            self.controler.setColor(0, 0, 0, 1)
            self.controler.setScale(0.2)
            self.controler.reparentTo(render)
            self.controler.setTag('OBJRoot','1')
            self.controler.setTag('Controller','1') #controller Tag
            self.i += 1

            iRay = SelectionRay(self.view.camera)
            iRay.collider.setFromLens(self.view.camNode, x, y)
            iRay.collideWithBitMask(BitMask32.bit(21))
            iRay.ct.traverse(self.view.collPlane)
            if iRay.getNumEntries() > 0:
                entry = iRay.getEntry(0)
                hitPt = entry.getSurfacePoint(entry.getFromNodePath())

            if hitPt:
                # create a temp nodePath to get the position
                np = NodePath('temp')
                np.setPos(self.view.camera, hitPt)

                if base.direct.manipulationControl.fGridSnap:
                    snappedPos = self.view.grid.computeSnapPoint(np.getPos())
                    np.setPos(snappedPos)

                # update temp nodePath's HPR and scale with newobj's
                np.setHpr(self.controler.getHpr())
                np.setScale(self.controler.getScale())

                # transform newobj to cursor position
                self.controler.setMat(Mat4(np.getMat()))
                np.remove()
            iRay.collisionNodePath.removeNode()
            del iRay

            self.curve.append((None, self.controler.getPos()))
            self.curveControl.append((self.i-1, self.controler))

