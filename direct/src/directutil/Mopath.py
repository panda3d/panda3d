from direct.showbase.DirectObject import DirectObject
from direct.directtools.DirectGeometry import *

from pandac.PandaModules import NodePath

class Mopath(DirectObject):

    nameIndex = 1

    def __init__(self, name = None, fluid = 1):
        if (name == None):
            name = 'mopath%d' % self.nameIndex
            self.nameIndex = self.nameIndex + 1
        self.name = name
        self.fluid = fluid
        self.tPoint = Point3(0)
        self.posPoint = Point3(0)
        self.hprPoint = Point3(0)
        self.tangentVec = Vec3(0)
        self.fFaceForward = 0
        self.timeScale = 1
        self.reset()

    def getMaxT(self):
        return self.maxT * self.timeScale

    def loadFile(self, filename, fReset = 1):
        nodePath = loader.loadModel(filename)
        if nodePath:
            self.loadNodePath(nodePath)
            nodePath.removeNode()
        else:
            print 'Mopath: no data in file: %s' % filename


    def loadNodePath(self, nodePath, fReset = 1):
        if fReset:
            self.reset()

        self.__extractCurves(nodePath)
        if (self.tNurbsCurve != []):
            self.maxT = self.tNurbsCurve[-1].getMaxT()
        elif (self.xyzNurbsCurve != None):
            self.maxT = self.xyzNurbsCurve.getMaxT()
        elif (self.hprNurbsCurve != None):
            self.maxT = self.hprNurbsCurve.getMaxT()
        else:
            print 'Mopath: no valid curves in file.'


    def reset(self):
        self.maxT = 0.0
        self.loop = 0
        self.xyzNurbsCurve = None
        self.hprNurbsCurve = None
        self.tNurbsCurve = []
        self.node = None

    def __extractCurves(self, nodePath):
        node = nodePath.node()
        if isinstance(node, ParametricCurve):
            if node.getCurveType() == PCTXYZ:
                self.xyzNurbsCurve = node
            elif node.getCurveType() == PCTHPR:
                self.hprNurbsCurve = node
            elif node.getCurveType() == PCTNONE:
                if (self.xyzNurbsCurve == None):
                    self.xyzNurbsCurve = node
                else:
                    print 'Mopath: got a PCT_NONE curve and an XYZ Curve!'
            elif (node.getCurveType() == PCTT):
                self.tNurbsCurve.append(node)
        else:
            # Iterate over children if any
            for child in nodePath.getChildren():
                self.__extractCurves(child)

    def calcTime(self, tIn):
        return self.__calcTime(tIn, self.tNurbsCurve)

    def __calcTime(self, tIn, tCurveList):
        if tCurveList:
            tCurveList[-1].getPoint(tIn, self.tPoint)
            return self.__calcTime(self.tPoint[0], tCurveList[:-1])
        else:
            return tIn

    def getFinalState(self):
        pos = Point3(0)
        if (self.xyzNurbsCurve != None):
            self.xyzNurbsCurve.getPoint(self.maxT, pos)
        hpr = Point3(0)
        if (self.hprNurbsCurve != None):
            self.hprNurbsCurve.getPoint(self.maxT, hpr)
        return (pos, hpr)

    def goTo(self, node, time):
        if (self.xyzNurbsCurve == None) and (self.hprNurbsCurve == None):
            print 'Mopath: Mopath has no curves'
            return
        time /= self.timeScale
        self.playbackTime = self.calcTime(CLAMP(time, 0.0, self.maxT))
        if (self.xyzNurbsCurve != None):
            self.xyzNurbsCurve.getPoint(self.playbackTime, self.posPoint)
            if self.fluid:
                node.setFluidPos(self.posPoint)
            else:
                node.setPos(self.posPoint)
        if (self.hprNurbsCurve != None):
            self.hprNurbsCurve.getPoint(self.playbackTime, self.hprPoint)
            node.setHpr(self.hprPoint)
        elif (self.fFaceForward and (self.xyzNurbsCurve != None)):
            self.xyzNurbsCurve.getTangent(self.playbackTime, self.tangentVec)
            node.lookAt(Point3(self.posPoint + self.tangentVec))

    def play(self, node, time = 0.0, loop = 0):
        if (self.xyzNurbsCurve == None) and (self.hprNurbsCurve == None):
            print 'Mopath: Mopath has no curves'
            return
        self.node = node
        self.loop = loop
        self.stop()
        t = taskMgr.add(self.__playTask, self.name + '-play')
        t.currentTime = time
        t.lastTime = globalClock.getFrameTime()

    def stop(self):
        taskMgr.remove(self.name + '-play')

    def __playTask(self, state):
        time = globalClock.getFrameTime()
        dTime = time - state.lastTime
        state.lastTime = time
        if (self.loop):
            cTime = (state.currentTime + dTime) % self.getMaxT()
        else:
            cTime = state.currentTime + dTime
        if ((self.loop == 0) and (cTime > self.getMaxT())):
            self.stop()
            messenger.send(self.name + '-done')
            self.node = None
            return Task.done
        self.goTo(self.node, cTime)
        state.currentTime = cTime
        return Task.cont
