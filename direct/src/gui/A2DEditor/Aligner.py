from pandac.PandaModules import NodePath, PGItem, Vec4
from direct.gui.DirectGui import DGG
from direct.gui.DirectGuiBase import DirectGuiWidget

def alignTo(obj, other, selfPos, otherPos=None, gap=(0,0)):
    '''
       Usage :
         myGui.alignTo( other, selfPos, otherPos, gap=(x,z) )
           OR
         alignTo( nodepath, other, selfPos, otherPos, gap=(x,z) )

         [+] selfPos is a position in myGui's coordinate space
         [+] otherPos is a position in other's coordinate space
         [x] if otherPos is missing, the same position will be used
         [+] they could be any of :
             LL (lower left)
             UL (upper left)
             LR (lower right)
             UR (upper right)
             C (center)
             CL (center left)
             CR (center right)
             CB (center bottom)
             CT (center top)
             O (origin)
         [+] gap is in the myGui/nodepath's coordinate space
    '''
    objNode = obj.node()
    otherNode = other.node()
    if otherPos is None:
       otherPos = selfPos
    if isinstance(objNode,PGItem):
       wB = Vec4(objNode.getFrame())
    else:
       isOrigin = selfPos==0
       if not NodePath(obj).getBounds().isEmpty() and not isOrigin:
          minb,maxb = obj.getTightBounds()
       else:
          minb = maxb = obj.getPos()
          if isOrigin:
             selfPos = (0,)*2 # any point is OK
       minb = obj.getRelativePoint(obj.getParent(),minb)
       maxb = obj.getRelativePoint(obj.getParent(),maxb)
       wB = Vec4(minb[0],maxb[0],minb[2],maxb[2])
    if isinstance(otherNode,PGItem):
       oB = Vec4(otherNode.getFrame())
    else:
       isOrigin = otherPos==0
       if not NodePath(other).getBounds().isEmpty() and not isOrigin:
          minb,maxb = other.getTightBounds()
       else:
          minb = maxb = other.getPos()
          if isOrigin:
             otherPos = (0,)*2 # any point is OK
       minb = other.getRelativePoint(other.getParent(),minb)
       maxb = other.getRelativePoint(other.getParent(),maxb)
       oB = Vec4(minb[0],maxb[0],minb[2],maxb[2])
    if selfPos[0]<0: # center
       selfPos=(0,selfPos[1])
       wB.setX(.5*(wB[0]+wB[1]))
    if selfPos[1]<0: # center
       selfPos=(selfPos[0],2)
       wB.setZ(.5*(wB[2]+wB[3]))
    if otherPos[0]<0: # center
       otherPos=(0,otherPos[1])
       oB.setX(.5*(oB[0]+oB[1]))
    if otherPos[1]<0: # center
       otherPos=(otherPos[0],2)
       oB.setZ(.5*(oB[2]+oB[3]))
    Xsign = 1-2*(selfPos[0]==otherPos[0])
    if ( (Xsign==-1 and selfPos[0]==1) or\
         (Xsign==1 and selfPos[0]==0) ):
       Xsign*=-1
    Zsign = 1-2*(selfPos[1]==otherPos[1])
    if ( (Zsign==-1 and selfPos[1]==3) or\
         (Zsign==1 and selfPos[1]==2) ):
       Zsign*=-1
    obj.setX( other, oB[otherPos[0]]-(wB[selfPos[0]]+gap[0]*Xsign)*obj.getSx(other) )
    obj.setZ( other, oB[otherPos[1]]-(wB[selfPos[1]]+gap[1]*Zsign)*obj.getSz(other) )
DirectGuiWidget.alignTo = alignTo
LL = DGG.LL = (0,2) #LOWER LEFT
UL = DGG.UL = (0,3) #UPPER LEFT
LR = DGG.LR = (1,2) #LOWER RIGHT
UR = DGG.UR = (1,3) #UPPER RIGHT
C = DGG.C = (-1,)*2 #CENTER
CL = DGG.CL = (0,-1) #CENTER LEFT
CR = DGG.CR = (1,-1) #CENTER RIGHT
CB = DGG.CB = (-1,2) #CENTER BOTTOM
CT = DGG.CT = (-1,3) #CENTER TOP
O = DGG.O = 0 #ORIGIN


if __name__ == '__main__':
   from direct.interval.IntervalGlobal import *
   from direct.gui.DirectGui import DirectButton
   from pandac.PandaModules import PandaNode
   import direct.directbase.DirectStart
   from random import random
   import sys

   def runInside(obj1,obj2):
       gap = (.3,.3)
       return Sequence(
          Func(alignTo, obj1, obj2, UR, gap=gap), Wait1,
          Func(alignTo, obj1, obj2, LR, gap=gap), Wait1,
          Func(alignTo, obj1, obj2, LL, gap=gap), Wait1,
          Func(alignTo, obj1, obj2, UL, gap=gap), Wait1,
       ), (1,0,0,1)
   def runInsideCenter(obj1,obj2):
       return Sequence(
          Func(alignTo, obj1, obj2, CR, gap=(.3,0)), Wait1,
          Func(alignTo, obj1, obj2, CB, gap=(0,.3)), Wait1,
          Func(alignTo, obj1, obj2, CL, gap=(.3,0)), Wait1,
          Func(alignTo, obj1, obj2, CT, gap=(0,.3)), Wait1,
          Func(alignTo, obj1, obj2, C), Wait1,
       ), (1,0,1,1)
   def runOutsideX(obj1,obj2):
       gap = (.3,0)
       return Sequence(
          Func(alignTo, obj1, obj2, UL, UR, gap), Wait1,
          Func(alignTo, obj1, obj2, LL, LR, gap), Wait1,
          Func(alignTo, obj1, obj2, LR, LL, gap), Wait1,
          Func(alignTo, obj1, obj2, UR, UL, gap), Wait1,
       ), (1,1,0,1)
   def runOutsideZ(obj1,obj2):
       gap = (0,.3)
       return Sequence(
          Func(alignTo, obj1, obj2, LR, UR, gap), Wait1,
          Func(alignTo, obj1, obj2, UR, LR, gap), Wait1,
          Func(alignTo, obj1, obj2, UL, LL, gap), Wait1,
          Func(alignTo, obj1, obj2, LL, UL, gap), Wait1,
       ), (0,1,0,1)
   def runOutsideCorner(obj1,obj2):
       return Sequence(
          Func(alignTo, obj1, obj2, LL, UR), Wait1,
          Func(alignTo, obj1, obj2, UL, LR), Wait1,
          Func(alignTo, obj1, obj2, UR, LL), Wait1,
          Func(alignTo, obj1, obj2, LR, UL), Wait1,
       ), (0,1,1,1)
   def runOutsideCenter(obj1,obj2):
       return Sequence(
          Func(alignTo, obj1, obj2, CL, CR, (.3,0)), Wait1,
          Func(alignTo, obj1, obj2, CT, CB, (0,.3)), Wait1,
          Func(alignTo, obj1, obj2, CR, CL, (.3,0)), Wait1,
          Func(alignTo, obj1, obj2, CB, CT, (0,.3)), Wait1,
       ), (1,0,1,1)
   def runAroundItself(obj1):
       obj1.setPos(-.8+random()*1.6,0,-.2+random()*.5)
       alignTo(obj1, obj1, CT, UR)
       return Sequence(
          Func(alignTo, obj1, obj1, UR, UL), Wait1,
          Func(alignTo, obj1, obj1, UR, LR), Wait1,
          Func(alignTo, obj1, obj1, LL, LR), Wait1,
          Func(alignTo, obj1, obj1, LL, UL), Wait1,
       ), (1,)*4


   def changeMode():
       global runSeq, run2Seq, run3Seq, modeIdx
       if runSeq: runSeq.finish()
       if run2Seq: run2Seq.finish()
       if run3Seq: run3Seq.finish()
       modeIdx = (modeIdx+1)%len(modes)
       lastMode = modeIdx==len(modes)-1
       modeArgs = [x] if lastMode else [x,b]
       runSeq, x['frameColor'] = modes[modeIdx](*modeArgs)
       runSeq.loop()
       modeArgs = [s] if lastMode else [s,b2]
       run2Seq, color = modes[modeIdx](*modeArgs)
       run2Seq.loop()
       s.setColor(color,1)
       modeArgs = [b3] if lastMode else [b3,smi]
       run3Seq, b3['frameColor'] = modes[modeIdx](*modeArgs)
       run3Seq.loop()

   Wait1 = Wait(1)
   runSeq = run2Seq = run3Seq = None
   modeIdx = -1
   modes = [runInside, runInsideCenter, runOutsideX, runOutsideZ, runOutsideCorner, runOutsideCenter, runAroundItself]

   b = DirectButton(text='CHANGE\nMODE',pad=(1,1), pos=(0,0,-.5), scale=.07, command=changeMode)
   x = DirectButton(text='EXIT',pad=(.2,.2), scale=.04, command=sys.exit)

   b2 = DirectButton(text='DO\nNOTHING\nAT\nALL',pad=(1,1), pos=(-.75,0,-.5), scale=.07)
   s = loader.loadModel('misc/sphere')
   s.setScale(.05)
   s.reparentTo(aspect2d)
   s.showTightBounds()

   smi = loader.loadModel('misc/lilsmiley')
   smi.setScale(.4)
   smi.reparentTo(aspect2d)
   smi.setPos(.75,0,-.5)
   smi.showTightBounds()
   b3 = DirectButton(text='DO\nNOTHING\nAT\nALL',pad=(.2,.2), scale=.032)


   ULcorner = DirectButton(parent=base.a2dTopLeft, text='RIGHT\nTO THE\nCORNER',scale=.05)
   ULcorner.alignTo(base.a2dTopLeft,UL,O)

   LRcorner = smi.copyTo(base.a2dBottomRight)
   LRcorner.setScale(.2)
   alignTo(LRcorner,LRcorner.getParent(),LR,O)

   dummy = DirectButton(text='dummy button\nto align\n4 empty nodes',scale=.05,pos=(-.8,0,.2))
   dummy.setR(-45)

   s2 = loader.loadModel('misc/sphere')
   s2.setScale(.015)
   s2.setColor(1,1,0,1)
   pos4 = (UL,UR,LR,LL)
   for i in range(4):
       emptyNP = aspect2d.attachNewNode(PandaNode(''))
       alignTo(emptyNP, dummy, pos4[i])
       s2.copyTo(emptyNP)

   changeMode()
   run()
# http://www.panda3d.org/phpbb2/viewtopic.php?t=8518
