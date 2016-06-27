"""
Defines AnimMgrBase
"""

import os, math

from direct.interval.IntervalGlobal import *
from panda3d.core import VBase3
from . import ObjectGlobals as OG
from . import AnimGlobals as AG

class AnimMgrBase:
    """ AnimMgr will create, manage, update animations in the scene """

    def __init__(self, editor):
        self.editor = editor
        self.graphEditorCounter = 0

        self.keyFramesInfo = {}
        self.curveAnimation = {}

        #normal properties
        self.lerpFuncs={
            'H' : self.lerpFuncH,
            'P' : self.lerpFuncP,
            'R' : self.lerpFuncR,
            'SX' : self.lerpFuncSX,
            'SY' : self.lerpFuncSY,
            'SZ' : self.lerpFuncSZ,
            'CR' : self.lerpFuncCR,
            'CG' : self.lerpFuncCG,
            'CB' : self.lerpFuncCB,
            'CA' : self.lerpFuncCA
            }

        #Properties which has animation curves
        self.curveLerpFuncs={
            'X' : [ self.lerpFuncX, self.lerpCurveFuncX ],
            'Y' : [ self.lerpFuncY, self.lerpCurveFuncY ],
            'Z' : [ self.lerpFuncZ, self.lerpCurveFuncZ ]
            }

    def reset(self):
        self.keyFramesInfo = {}
        self.curveAnimation = {}

    def generateKeyFrames(self):
        #generate keyFrame list
        self.keyFrames = []
        for property in list(self.keyFramesInfo.keys()):
            for frameInfo in self.keyFramesInfo[property]:
                frame = frameInfo[AG.FRAME]
                exist = False
                for keyFrame in self.keyFrames:
                    if frame == keyFrame:
                        exist = True
                        break
                if exist == False:
                    self.keyFrames.append(frame)

    def generateSlope(self, list):
        #generate handler slope of every keyframe for animation curve
        listLen = len(list)
        if listLen == 2:
            slope =[float(list[1][AG.FRAME]-list[0][AG.FRAME]),(float(list[1][AG.VALUE])-float(list[0][AG.VALUE]))]
            list[0][AG.INSLOPE] = slope
            list[1][AG.INSLOPE] = slope
            list[0][AG.OUTSLOPE] = list[0][AG.INSLOPE]
            list[1][AG.OUTSLOPE] = list[1][AG.INSLOPE]
            return

        if listLen >= 3:
            list[0][AG.INSLOPE] = [float(list[1][AG.FRAME] - list[0][AG.FRAME]),(float(list[1][AG.VALUE]) - float(list[0][AG.VALUE]))]
            list[0][AG.OUTSLOPE] = list[0][AG.INSLOPE]
            for i in range(1, listLen-1):
                list[i][AG.INSLOPE] = [float(list[i+1][AG.FRAME] - list[i-1][AG.FRAME]),(float(list[i+1][AG.VALUE]) - float(list[i-1][AG.VALUE]))]
                list[i][AG.OUTSLOPE] = list[i][AG.INSLOPE]
            list[listLen-1][AG.INSLOPE] = [float(list[listLen-1][AG.FRAME] - list[listLen-2][AG.FRAME]),(float(list[listLen-1][AG.VALUE]) - float(list[listLen-2][AG.VALUE]))]
            list[listLen-1][AG.OUTSLOPE] = list[listLen-1][AG.INSLOPE]
            return

    def removeAnimInfo(self, uid):
        for property in list(self.keyFramesInfo.keys()):
            if property[AG.UID] == uid:
                del self.keyFramesInfo[property]
        self.generateKeyFrames()
        if self.editor.mode == self.editor.ANIM_MODE:
            self.editor.ui.animUI.OnPropKey()

    def singleCurveAnimation(self, nodePath, curve, time):
        rope = curve[OG.OBJ_NP]
        self.points = rope.getPoints(time)
        self.hprs = []
        temp = render.attachNewNode("temp")
        temp.setHpr(0,0,0)
        for i in range(len(self.points)-1):
            temp.setPos(self.points[i])
            temp.lookAt(self.points[i+1])
            hpr = temp.getHpr()
            ## self.hprs.append(hpr)
            self.hprs.append(VBase3(hpr[0]+180,hpr[1],hpr[2]))
        self.hprs.append(self.hprs[len(self.points)-2])

        curveSequenceName = str(nodePath[OG.OBJ_UID])+' '+str(curve[OG.OBJ_UID])+' '+str(time)
        self.curveSequence = Sequence(name = curveSequenceName)

        for i in range(len(self.points)-1):
            myLerp = LerpPosHprInterval(nodePath[OG.OBJ_NP], float(1)/float(24), self.points[i+1], self.hprs[i+1], self.points[i], self.hprs[i])
            self.curveSequence.append(myLerp)

        return self.curveSequence

    def createParallel(self, startFrame, endFrame):
        self.parallel = []
        self.parallel = Parallel(name="Current Parallel")

        self.createCurveAnimation(self.parallel)
        self.createActorAnimation(self.parallel, startFrame, endFrame)
        self.createKeyFrameAnimation(self.parallel, startFrame, endFrame)
        self.createCurveKeyFrameAnimation(self.parallel, startFrame, endFrame)

        return self.parallel

    def createCurveAnimation(self, parallel):
        for key in self.curveAnimation:
            curveInfo = self.curveAnimation[key]
            nodePath = self.editor.objectMgr.findObjectById(curveInfo[AG.NODE])
            curve = self.editor.objectMgr.findObjectById(curveInfo[AG.CURVE])
            time = curveInfo[AG.TIME]
            sequence = self.singleCurveAnimation(nodePath, curve, time)
            parallel.append(sequence)

    def createActorAnimation(self, parallel, startFrame, endFrame):
        self.editor.objectMgr.findActors(render)
        for actor in self.editor.objectMgr.Actor:
            actorAnim = os.path.basename(actor[OG.OBJ_ANIM])
            myInterval = ActorInterval(actor[OG.OBJ_NP], actorAnim, loop=1, duration = float(endFrame-startFrame+1)/float(24))
            parallel.append(myInterval)

    def createKeyFrameAnimation(self, parallel, startFrame, endFrame):
        #generate key frame animation for normal property
        self.editor.objectMgr.findNodes(render)
        for node in self.editor.objectMgr.Nodes:
            for property in list(self.keyFramesInfo.keys()):
                if property[AG.UID] == node[OG.OBJ_UID] and property[AG.PROP_NAME] != 'X' and property[AG.PROP_NAME] != 'Y' and property[AG.PROP_NAME] != 'Z':
                    mysequence = Sequence(name = node[OG.OBJ_UID])
                    keyFramesInfo = self.keyFramesInfo[property]
                    if len(keyFramesInfo) == 1:
                        myLerp = LerpFunc(self.lerpFuncs[property[AG.PROP_NAME]],fromData=float(keyFramesInfo[0][AG.VALUE]),toData=float(keyFramesInfo[0][AG.VALUE]),duration = float(endFrame-startFrame)/float(24),blendType = 'noBlend',extraArgs = [node[OG.OBJ_NP]])
                        mysequence.append(myLerp)
                        parallel.append(mysequence)

                    if len(keyFramesInfo) != 1:
                        myLerp = LerpFunc(self.lerpFuncs[property[AG.PROP_NAME]],fromData=float(keyFramesInfo[0][AG.VALUE]),toData=float(keyFramesInfo[0][AG.VALUE]),duration = float(keyFramesInfo[0][AG.FRAME]-startFrame)/float(24),blendType = 'noBlend',extraArgs = [node[OG.OBJ_NP]])
                        mysequence.append(myLerp)

                        for key in range(0,len(keyFramesInfo)-1):
                            myLerp = LerpFunc(self.lerpFuncs[property[AG.PROP_NAME]],fromData=float(keyFramesInfo[key][AG.VALUE]),toData=float(keyFramesInfo[key+1][AG.VALUE]),duration = float(keyFramesInfo[key+1][AG.FRAME]-keyFramesInfo[key][AG.FRAME])/float(24),blendType = 'noBlend',extraArgs = [node[OG.OBJ_NP]])
                            mysequence.append(myLerp)

                        myLerp = LerpFunc(self.lerpFuncs[property[AG.PROP_NAME]],fromData=float(keyFramesInfo[len(keyFramesInfo)-1][AG.VALUE]),toData=float(keyFramesInfo[len(keyFramesInfo)-1][AG.VALUE]),duration = float(endFrame-keyFramesInfo[len(keyFramesInfo)-1][AG.FRAME])/float(24),blendType = 'noBlend',extraArgs = [node[OG.OBJ_NP]])
                        mysequence.append(myLerp)
                        parallel.append(mysequence)

    def createCurveKeyFrameAnimation(self, parallel, startFrame, endFrame):
        #generate key frame animation for the property which is controled by animation curve
        self.editor.objectMgr.findNodes(render)
        for node in self.editor.objectMgr.Nodes:
            for property in list(self.keyFramesInfo.keys()):
                if property[AG.UID] == node[OG.OBJ_UID]:
                    if property[AG.PROP_NAME] == 'X' or property[AG.PROP_NAME] == 'Y' or property[AG.PROP_NAME] == 'Z':
                        mysequence = Sequence(name = node[OG.OBJ_UID])
                        keyFramesInfo = self.keyFramesInfo[property]
                        if len(keyFramesInfo) == 1:
                            myLerp = LerpFunc(self.curveLerpFuncs[property[AG.PROP_NAME]][0],fromData=float(keyFramesInfo[0][AG.VALUE]),toData=float(keyFramesInfo[0][AG.VALUE]),duration = float(endFrame-startFrame)/float(24),blendType = 'noBlend',extraArgs = [node[OG.OBJ_NP]])
                            mysequence.append(myLerp)
                            parallel.append(mysequence)

                        if len(keyFramesInfo) == 2:
                            myLerp = LerpFunc(self.curveLerpFuncs[property[AG.PROP_NAME]][0],fromData=float(keyFramesInfo[0][AG.VALUE]),toData=float(keyFramesInfo[0][AG.VALUE]),duration = float(keyFramesInfo[0][AG.FRAME]-startFrame)/float(24),blendType = 'noBlend',extraArgs = [node[OG.OBJ_NP]])
                            mysequence.append(myLerp)

                            for key in range(0,len(keyFramesInfo)-1):
                                self.keyFrameInfoForSingleLerp = keyFramesInfo
                                self.keyInfoForSingleLerp = key
                                myLerp = LerpFunc(self.curveLerpFuncs[property[AG.PROP_NAME]][0],fromData=float(keyFramesInfo[key][AG.VALUE]),toData=float(keyFramesInfo[key+1][AG.VALUE]),duration = float(keyFramesInfo[key+1][AG.FRAME]-keyFramesInfo[key][AG.FRAME])/float(24),blendType = 'noBlend',extraArgs = [node[OG.OBJ_NP]])
                                mysequence.append(myLerp)

                            myLerp = LerpFunc(self.curveLerpFuncs[property[AG.PROP_NAME]][0],fromData=float(keyFramesInfo[len(keyFramesInfo)-1][AG.VALUE]),toData=float(keyFramesInfo[len(keyFramesInfo)-1][AG.VALUE]),duration = float(endFrame-keyFramesInfo[len(keyFramesInfo)-1][AG.FRAME])/float(24),blendType = 'noBlend',extraArgs = [node[OG.OBJ_NP]])
                            mysequence.append(myLerp)
                            parallel.append(mysequence)

                        if len(keyFramesInfo) > 2:
                            myLerp = LerpFunc(self.curveLerpFuncs[property[AG.PROP_NAME]][0],fromData=float(keyFramesInfo[0][AG.VALUE]),toData=float(keyFramesInfo[0][1]),duration = float(keyFramesInfo[0][AG.FRAME]-startFrame)/float(24),blendType = 'noBlend',extraArgs = [node[OG.OBJ_NP]])
                            mysequence.append(myLerp)

                            for key in range(0,len(keyFramesInfo)-1):
                                myLerp = LerpFunc(self.curveLerpFuncs[property[AG.PROP_NAME]][1],fromData=float(keyFramesInfo[key][AG.FRAME]),toData=float(keyFramesInfo[key+1][AG.FRAME]),duration = float(keyFramesInfo[key+1][AG.FRAME]-keyFramesInfo[key][AG.FRAME])/float(24),blendType = 'noBlend',extraArgs = [[node[OG.OBJ_NP], keyFramesInfo, key]])
                                mysequence.append(myLerp)

                            myLerp = LerpFunc(self.curveLerpFuncs[property[AG.PROP_NAME]][0],fromData=float(keyFramesInfo[len(keyFramesInfo)-1][AG.VALUE]),toData=float(keyFramesInfo[len(keyFramesInfo)-1][AG.VALUE]),duration = float(endFrame-keyFramesInfo[len(keyFramesInfo)-1][AG.FRAME])/float(24),blendType = 'noBlend',extraArgs = [node[OG.OBJ_NP]])
                            mysequence.append(myLerp)
                            parallel.append(mysequence)

    def getPos(self, x, list, i):
        #get the value from animation curve
        x1 = float(list[i][AG.FRAME])
        y1 = float(list[i][AG.VALUE])

        x4 = float(list[i+1][AG.FRAME])
        y4 = float(list[i+1][AG.VALUE])

        t1x = list[i][AG.OUTSLOPE][0]
        t1y = list[i][AG.OUTSLOPE][1]

        t2x = list[i+1][AG.INSLOPE][0]
        t2y = list[i+1][AG.INSLOPE][1]

        x2 = x1 + (x4 - x1) / float(3)
        scale1 = (x2 - x1) / t1x
        y2 = y1 + t1y * scale1

        x3 = x4 - (x4 - x1) / float(3)
        scale2 = (x4 - x3) / t2x
        y3 = y4 - t2y * scale2

        ax = - float(1) * x1 + float(3) * x2 - float(3) * x3 + float(1) * x4
        bx =   float(3) * x1 - float(6) * x2 + float(3) * x3 + float(0) * x4
        cx = - float(3) * x1 + float(3) * x2 + float(0) * x3 + float(0) * x4
        dx =   float(1) * x1 + float(0) * x2 - float(0) * x3 + float(0) * x4

        ay = - float(1) * y1 + float(3) * y2 - float(3) * y3 + float(1) * y4
        by =   float(3) * y1 - float(6) * y2 + float(3) * y3 + float(0) * y4
        cy = - float(3) * y1 + float(3) * y2 + float(0) * y3 + float(0) * y4
        dy =   float(1) * y1 + float(0) * y2 - float(0) * y3 + float(0) * y4

        if ax == 0 and bx == 0 and cx == 0:
            return 0

        if ax == 0 and bx == 0 and cx != 0:
            a = cx
            b = dx-x
            t = -b/a
            y = ay * t*t*t + by * t*t + cy * t + dy
            return y

        if ax == 0 and bx!= 0:
            a=bx
            b=cx
            c=dx-x
            t=(-b+math.sqrt(b**2-4.0*a*c))/2*a
            if t>=0 and t<=1:
                y = ay * t*t*t + by * t*t + cy * t + dy
                return y
            else:
                t=(-b-math.sqrt(b**2-4.0*a*c))/2*a
                y = ay * t*t*t + by * t*t + cy * t + dy
                return y

        if ax != 0:
            a = ax
            b = bx
            c = cx
            d = dx - float(x)
            t = self.calculateT(a, b, c, d, x)
            y = ay * t*t*t + by * t*t + cy * t + dy
            return y

    def calculateT(self, a, b, c, d, x):
        #Newton EQUATION
        t = float(1)
        t2 = t
        t -= (a*t*t*t+b*t*t+c*t+d)/(float(3)*a*t*t+float(2)*b*t+c)
        if abs(t-t2) <= 0.000001:
            return t
        else:
            while abs(t - t2) > 0.000001:
                t2 = t
                t -= (a*t*t*t+b*t*t+c*t+d)/(float(3)*a*t*t+float(2)*b*t+c)
            return t

    def lerpFuncX(self,pos,np):
        np.setX(pos)

    def lerpFuncY(self,pos,np):
        np.setY(pos)

    def lerpFuncZ(self,pos,np):
        np.setZ(pos)

    def lerpCurveFuncX(self,t,extraArgs):
        np = extraArgs[0]
        pos = self.getPos(t, extraArgs[1], extraArgs[2])
        np.setX(pos)

    def lerpCurveFuncY(self,t,extraArgs):
        np = extraArgs[0]
        pos = self.getPos(t, extraArgs[1], extraArgs[2])
        np.setY(pos)

    def lerpCurveFuncZ(self,t,extraArgs):
        np = extraArgs[0]
        pos = self.getPos(t, extraArgs[1], extraArgs[2])
        np.setZ(pos)

    def lerpFuncH(self,angle,np):
        np.setH(angle)

    def lerpFuncP(self,angle,np):
        np.setP(angle)

    def lerpFuncR(self,angle,np):
        np.setR(angle)

    def lerpFuncSX(self,scale,np):
        np.setSx(scale)

    def lerpFuncSY(self,scale,np):
        np.setSy(scale)

    def lerpFuncSZ(self,scale,np):
        np.setSz(scale)

    def lerpFuncCR(self,R,np):
        obj = self.editor.objectMgr.findObjectByNodePath(np)
        r = obj[OG.OBJ_RGBA][0]
        g = obj[OG.OBJ_RGBA][1]
        b = obj[OG.OBJ_RGBA][2]
        a = obj[OG.OBJ_RGBA][3]
        self.colorUpdate(R,g,b,a,np)

    def lerpFuncCG(self,G,np):
        obj = self.editor.objectMgr.findObjectByNodePath(np)
        r = obj[OG.OBJ_RGBA][0]
        g = obj[OG.OBJ_RGBA][1]
        b = obj[OG.OBJ_RGBA][2]
        a = obj[OG.OBJ_RGBA][3]
        self.colorUpdate(r,G,b,a,np)

    def lerpFuncCB(self,B,np):
        obj = self.editor.objectMgr.findObjectByNodePath(np)
        r = obj[OG.OBJ_RGBA][0]
        g = obj[OG.OBJ_RGBA][1]
        b = obj[OG.OBJ_RGBA][2]
        a = obj[OG.OBJ_RGBA][3]
        self.colorUpdate(r,g,B,a,np)

    def lerpFuncCA(self,A,np):
        obj = self.editor.objectMgr.findObjectByNodePath(np)
        r = obj[OG.OBJ_RGBA][0]
        g = obj[OG.OBJ_RGBA][1]
        b = obj[OG.OBJ_RGBA][2]
        a = obj[OG.OBJ_RGBA][3]
        self.colorUpdate(r,g,b,A,np)

    def colorUpdate(self, r, g, b, a, np):
        if base.direct.selected.last == None:
            self.editor.objectMgr.updateObjectColor(r, g, b, a, np)
        elif self.editor.objectMgr.findObjectByNodePath(np) == self.editor.objectMgr.findObjectByNodePath(base.direct.selected.last):
            self.editor.ui.objectPropertyUI.propCR.setValue(r)
            self.editor.ui.objectPropertyUI.propCG.setValue(g)
            self.editor.ui.objectPropertyUI.propCB.setValue(b)
            self.editor.ui.objectPropertyUI.propCA.setValue(a)
            self.editor.objectMgr.updateObjectColor(r, g, b, a, np)
        else:
            self.editor.objectMgr.updateObjectColor(r, g, b, a, np)

