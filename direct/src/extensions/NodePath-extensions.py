
    """
    NodePath-extensions module: contains methods to extend functionality
    of the NodePath class
    """

    def id(self):
        """Returns the bottom node's this pointer as a unique id"""
        return self.node().this

    def getNodePathName(self):
        from PandaModules import *
        # Initialize to a default value
        name = '<noname>'
        # Get the bottom node
        node = self.node()
        # Is it a named node?, If so, see if it has a name
        if issubclass(node.__class__, NamedNode):
            namedNodeName = node.getName()
            # Is it not zero length?
            if len(namedNodeName) != 0:
                name = namedNodeName
        return name

    # For iterating over children
    def getChildrenAsList(self):
        childrenList = []
        for childNum in range(self.getNumChildren()):
            childrenList.append(self.getChild(childNum))
        return childrenList

    def toggleViz(self):
        if self.isHidden():
            self.show()
        else:
            self.hide()
            
    def showSiblings(self):
        for sib in self.getParent().getChildrenAsList():
            if sib != self:
                sib.show()

    def hideSiblings(self):
        for sib in self.getParent().getChildrenAsList():
            if sib != aNodePath:
                sib.hide()

    def showAllDescendants(self):
	self.show()
        for child in self.getChildrenAsList():
            self.showAllDescendants(child)

    def isolate(self):
        self.showAllDescendants()
        self.hideSiblings()

    def remove(self):
        from PandaObject import *
        # Send message in case anyone needs to do something
        # before node is deleted
        messenger.send('preRemoveNodePath', [self])
        # Remove nodePath
        self.reparentTo(hidden)
        self.removeNode()

    def reversels(self):
        ancestry = self.getAncestry()
        indentString = ""
        for nodePath in ancestry:
            type = nodePath.node().getType().getName()
            name = nodePath.getNodePathName()
            print indentString + type + "  " + name
            indentString = indentString + " "

    def getAncestry(self):
        from PandaObject import *
        node = self.node()
        if ((node != render.node()) | (node != hidden.node())):
            ancestry = self.getParent().getAncestry()
            ancestry.append(self)
            return ancestry
        else:
            return [self]

    # private methods
    
    def __getBlend(self, blendType):
        """__getBlend(self, string)
        Return the C++ blend class corresponding to blendType string
        """
        import LerpBlendHelpers

        if (blendType == "easeIn"):
            return LerpBlendHelpers.LerpBlendHelpers.easeIn
        elif (blendType == "easeOut"):
            return LerpBlendHelpers.LerpBlendHelpers.easeOut
        elif (blendType == "easeInOut"):
            return LerpBlendHelpers.LerpBlendHelpers.easeInOut
        elif (blendType == "noBlend"):
            return LerpBlendHelpers.LerpBlendHelpers.noBlend
        else:
            raise Exception("Error: NodePath.__getBlend: Unknown blend type")

            
    def __lerp(self, functor, time, blendType, taskName=None):
        """__lerp(self, functor, float, string, string)
        Basic lerp functionality used by other lerps.
        Fire off a lerp. Make it a task if taskName given."""
        import Lerp
        # make the lerp
        lerp = Lerp.Lerp(functor, time, (self.__getBlend(blendType)))

        from TaskManagerGlobal import *
        # make the task function
        def lerpTaskFunc(task):
            import Task
            import ClockObject
            dt = ClockObject.ClockObject.getGlobalClock().getDt()
            task.lerp.setStepSize(dt)
            task.lerp.step()
            if (task.lerp.isDone()):
                return(Task.done)
            else:
                return(Task.cont)

        # make the lerp task
        lerpTask = Task.Task(lerpTaskFunc)
        lerpTask.lerp = lerp
        
        if (taskName == None):
            # don't spawn a task, return one instead
            return lerpTask
        else:
            # spawn the lerp task
            taskMgr.spawnTaskNamed(lerpTask, taskName)
            return lerpTask

    def __autoLerp(self, functor, time, blendType, taskName):
        """_autoLerp(self, functor, float, string, string)
        This lerp uses C++ to handle the stepping. Bonus is
        its more efficient, trade-off is there is less control"""
        import AutonomousLerp
        from ShowBaseGlobal import *

        # make a lerp that lives in C++ land
        lerp = AutonomousLerp.AutonomousLerp(functor, time,
                              self.__getBlend(blendType),
                              base.eventHandler)
        lerp.start()
        return lerp


    # user callable lerp methods
    def lerpColor(self, *posArgs, **keyArgs):
        """lerpColor(self, *positionArgs, **keywordArgs)
        determine which lerpColor* to call based on arguments
        """
        if (len(posArgs) == 2):
            return apply(self.lerpColorVBase4, posArgs, keyArgs)
        elif (len(posArgs) == 3):
            return apply(self.lerpColorVBase4VBase4, posArgs, keyArgs)
        elif (len(posArgs) == 5):
            return apply(self.lerpColorRGBA, posArgs, keyArgs)
        elif (len(posArgs) == 9):
            return apply(self.lerpColorRGBARGBA, posArgs, keyArgs)
        else:
            # bad args
            raise Exception("Error: NodePath.lerpColor: bad number of args")

            
    def lerpColorRGBA(self, r, g, b, a, time, blendType="noBlend",
                      auto=None, task=None):
        """lerpColorRGBA(self, float, float, float, float, float,
        string="noBlend", string=none, string=none)
        """
        import ColorLerpFunctor
        # just end rgba values, use current color rgba values for start
        startColor = self.getColor()
        functor = ColorLerpFunctor.ColorLerpFunctor(self,
                                   startColor[0], startColor[1],
                                   startColor[2], startColor[3],
                                   r, g, b, a)
        #determine whether to use auto, spawned, or blocking lerp
        if (auto != None):
            return self.__autoLerp(functor, time, blendType, auto)
        elif (task != None):
            return self.__lerp(functor, time, blendType, task)
        else:
            return self.__lerp(functor, time, blendType)

    def lerpColorRGBARGBA(self, sr, sg, sb, sa, er, eg, eb, ea, time,
                          blendType="noBlend", auto=None, task=None):
        """lerpColorRGBARGBA(self, float, float, float, float, float,
        float, float, float, float, string="noBlend", string=none, string=none)
        """
        import ColorLerpFunctor
        # start and end rgba values
        functor = ColorLerpFunctor.ColorLerpFunctor(self, sr, sg, sb, sa,
                                                    er, eg, eb, ea)
        #determine whether to use auto, spawned, or blocking lerp
        if (auto != None):
            return self.__autoLerp(functor, time, blendType, auto)
        elif (task != None):
            return self.__lerp(functor, time, blendType, task)
        else:
            return self.__lerp(functor, time, blendType)

    def lerpColorVBase4(self, endColor, time, blendType="noBlend",
                      auto=None, task=None):
        """lerpColorVBase4(self, VBase4, float, string="noBlend", string=none,
        string=none)
        """
        import ColorLerpFunctor
        # just end vec4, use current color for start
        startColor = self.getColor()
        functor = ColorLerpFunctor.ColorLerpFunctor(self, startColor, endColor)
        #determine whether to use auto, spawned, or blocking lerp
        if (auto != None):
            return self.__autoLerp(functor, time, blendType, auto)
        elif (task != None):
            return self.__lerp(functor, time, blendType, task)
        else:
            return self.__lerp(functor, time, blendType)

    def lerpColorVBase4VBase4(self, startColor, endColor, time,
                          blendType="noBlend", auto=None, task=None):
        """lerpColorVBase4VBase4(self, VBase4, VBase4, float, string="noBlend",
        string=none, string=none)
        """
        import ColorLerpFunctor
        # start color and end vec
        functor = ColorLerpFunctor.ColorLerpFunctor(self, startColor, endColor)
        #determine whether to use auto, spawned, or blocking lerp
        if (auto != None):
            return self.__autoLerp(functor, time, blendType, auto)
        elif (task != None):
            return self.__lerp(functor, time, blendType, task)
        else:
            return self.__lerp(functor, time, blendType)
            

    def lerpHpr(self, *posArgs, **keyArgs):
        """lerpHpr(self, *positionArgs, **keywordArgs)
        Determine whether to call lerpHprHPR or lerpHprVBase3
        based on first argument
        """
        # check to see if lerping with
        # three floats or a VBase3
        if (len(posArgs) == 4):
            return apply(self.lerpHprHPR, posArgs, keyArgs)
        elif(len(posArgs) == 2):
            return apply(self.lerpHprVBase3, posArgs, keyArgs)
        else:
            # bad args
            raise Exception("Error: NodePath.lerpHpr: bad number of args")
    
    def lerpHprHPR(self, h, p, r, time, blendType="noBlend", auto=None,
                   task=None, other=None):
        """lerpHprHPR(self, float, float, float, float, string="noBlend",
        string=none, string=none, NodePath=none)
        Perform a hpr lerp with three floats as the end point
        """
        import HprLerpFunctor
        # it's individual hpr components
        if (other != None):
            # lerp wrt other
            startHpr = self.getHpr(other)
            functor = HprLerpFunctor.HprLerpFunctor(self,
                                     startHpr[0], startHpr[1], startHpr[2],
                                     h, p, r, other)
        else:
            startHpr = self.getHpr()
            functor = HprLerpFunctor.HprLerpFunctor(self,
                                     startHpr[0], startHpr[1], startHpr[2],
                                     h, p, r)
        #determine whether to use auto, spawned, or blocking lerp
        if (auto != None):
            return self.__autoLerp(functor, time, blendType, auto)
        elif (task != None):
            return self.__lerp(functor, time, blendType, task)
        else:
            return self.__lerp(functor, time, blendType)
        
    
    def lerpHprVBase3(self, hpr, time, blendType="noBlend", auto=None,
                    task=None, other=None):
        """lerpHprVBase3(self, VBase3, float, string="noBlend", string=none,
        string=none, NodePath=None)
        Perform a hpr lerp with a VBase3 as the end point
        """
        import HprLerpFunctor
        # it's a vbase3 hpr
        if (other != None):
            # lerp wrt other
            functor = HprLerpFunctor.HprLerpFunctor(self, (self.getHpr(other)),
                                                    hpr, other)
        else:
            functor = HprLerpFunctor.HprLerpFunctor(self, (self.getHpr()),
                                                    hpr)
        #determine whether to use auto, spawned, or blocking lerp
        if (auto != None):
            return self.__autoLerp(functor, time, blendType, auto)
        elif (task != None):
            return self.__lerp(functor, time, blendType, task)
        else:
            return self.__lerp(functor, time, blendType)
        

    def lerpPos(self, *posArgs, **keyArgs):
        """lerpPos(self, *positionArgs, **keywordArgs)
        Determine whether to call lerpPosXYZ or lerpPosPoint3
        based on the first argument
        """
        # check to see if lerping with three
        # floats or a Point3
        if (len(posArgs) == 4):
            return apply(self.lerpPosXYZ, posArgs, keyArgs)
        elif(len(posArgs) == 2):
            return apply(self.lerpPosPoint3, posArgs, keyArgs)
        else:
            # bad number off args
            raise Exception("Error: NodePath.lerpPos: bad number of args")
        
    def lerpPosXYZ(self, x, y, z, time, blendType="noBlend", auto=None,
                   task=None, other=None):
        """lerpPosXYZ(self, float, float, float, float, string="noBlend",
        string=None, NodePath=None)
        Perform a pos lerp with three floats as the end point
        """
        import PosLerpFunctor
        if (other != None):
            # lerp wrt other
            startPos = self.getPos(other)
            functor = PosLerpFunctor.PosLerpFunctor(self,
                                     startPos[0], startPos[1], startPos[2],
                                     x, y, z, other)
        else:
            startPos = self.getPos()
            functor = PosLerpFunctor.PosLerpFunctor(self, startPos[0],
                                     startPos[1], startPos[2], x, y, z)
        #determine whether to use auto, spawned, or blocking lerp
        if (auto != None):
            return  self.__autoLerp(functor, time, blendType, auto)
        elif (task != None):
            return self.__lerp(functor, time, blendType, task)
        else:
            return self.__lerp(functor, time, blendType)

    def lerpPosPoint3(self, pos, time, blendType="noBlend", auto=None,
                    task=None, other=None):
        """lerpPosPoint3(self, Point3, float, string="noBlend", string=None,
        string=None, NodePath=None)
        Perform a pos lerp with a Point3 as the end point
        """
        import PosLerpFunctor
        if (other != None):
            #lerp wrt other
            functor = PosLerpFunctor.PosLerpFunctor(self, (self.getPos(other)),
                                                    pos, other)
        else:
            functor = PosLerpFunctor.PosLerpFunctor(self, (self.getPos()), pos)
        #determine whether to use auto, spawned, or blocking lerp
        if (auto != None):
            return self.__autoLerp(functor, time, blendType, auto)
        elif (task != None):
            return self.__lerp(functor, time, blendType, task)
        else:
            return self.__lerp(functor, time, blendType)


    def lerpPosHpr(self, *posArgs, **keyArgs):
        """lerpPosHpr(self, *positionArgs, **keywordArgs)
        Determine whether to call lerpPosHprXYZHPR or lerpHprPoint3VBase3
        based on first argument
        """
        # check to see if lerping with
        # six floats or a Point3 and a VBase3
        if (len(posArgs) == 7):
            return apply(self.lerpPosHprXYZHPR, posArgs, keyArgs)
        elif(len(posArgs) == 3):
            return apply(self.lerpPosHprPoint3VBase3, posArgs, keyArgs)
        else:
            # bad number off args
            raise Exception("Error: NodePath.lerpPosHpr: bad number of args")

    def lerpPosHprPoint3VBase3(self, pos, hpr, time, blendType="noBlend",
                             auto=None, task=None, other=None):
        """lerpPosHprPoint3VBase3(self, Point3, VBase3, string="noBlend",
        string=none, string=none, NodePath=None)
        """
        import PosHprLerpFunctor
        if (other != None):
            # lerp wrt other
            startPos = self.getPos(other)
            startHpr = self.getHpr(other)
            functor = PosHprLerpFunctor.PosHprLerpFunctor(self,
                                                          startPos, pos,
                                                          startHpr, hpr, other)
        else:
            startPos = self.getPos()
            startHpr = self.getHpr()
            functor = PosHprLerpFunctor.PosHprLerpFunctor(self,
                                                          startPos, pos,
                                                          startHpr, hpr)
        #determine whether to use auto, spawned, or blocking lerp
        if (auto != None):
            return self.__autoLerp(functor, time, blendType, auto)
        elif (task != None):
            return self.__lerp(functor, time, blendType, task)
        else:
            return self.__lerp(functor, time, blendType)

    def lerpPosHprXYZHPR(self, x, y, z, h, p, r, time, blendType="noBlend",
                         auto=None, task=None, other=None):
        """lerpPosHpr(self, float, string="noBlend", string=none,
        string=none, NodePath=None)
        """
        import PosHprLerpFunctor
        if (other != None):
            # lerp wrt other
            startPos = self.getPos(other)
            startHpr = self.getHpr(other)
            functor = PosHprLerpFunctor.PosHprLerpFunctor(self,
                                        startPos[0], startPos[1],
                                        startPos[2], x, y, z,
                                        startHpr[0], startHpr[1],
                                        startHpr[2], h, p, r,
                                        other)
        else:
            startPos = self.getPos()
            startHpr = self.getHpr()
            functor = PosHprLerpFunctor.PosHprLerpFunctor(self,
                                        startPos[0], startPos[1],
                                        startPos[2], x, y, z,
                                        startHpr[0], startHpr[1],
                                        startHpr[2], h, p, r)
        #determine whether to use auto, spawned, or blocking lerp
        if (auto != None):
            return self.__autoLerp(functor, time, blendType, auto)
        elif (task != None):
            return self.__lerp(functor, time, blendType, task)
        else:
            return self.__lerp(functor, time, blendType)


    def lerpPosHprScale(self, pos, hpr, scale, time, blendType="noBlend",
                        auto=None, task=None, other=None):
        """lerpPosHpr(self, Point3, VBase3, float, float, string="noBlend",
        string=none, string=none, NodePath=None)
        Only one case, no need for extra args. Call the appropriate lerp
        (auto, spawned, or blocking) based on how(if) a task name is given
        """
        import PosHprScaleLerpFunctor
        if (other != None):
            # lerp wrt other
            startPos = self.getPos(other)
            startHpr = self.getHpr(other)
            startScale = self.getScale(other)
            functor = PosHprScaleLerpFunctor.PosHprScaleLerpFunctor(self,
                                             startPos, pos,
                                             startHpr, hpr,
                                             startScale, scale, other)
        else:
            startPos = self.getPos()
            startHpr = self.getHpr()
            startScale = self.getScale()
            functor = PosHprScaleLerpFunctor.PosHprScaleLerpFunctor(self,
                                             startPos, pos,
                                             startHpr, hpr,
                                             startScale, scale)
            
        #determine whether to use auto, spawned, or blocking lerp
        if (auto != None):
            return self.__autoLerp(functor, time, blendType, auto)
        elif (task != None):
            return self.__lerp(functor, time, blendType, task)
        else:
            return self.__lerp(functor, time, blendType)


    def lerpScale(self, *posArgs, **keyArgs):
        """lerpSclae(self, *positionArgs, **keywordArgs)
        Determine whether to call lerpScaleXYZ or lerpScaleaseV3
        based on the first argument
        """
        # check to see if lerping with three
        # floats or a Point3
        if (len(posArgs) == 4):
            return apply(self.lerpScaleXYZ, posArgs, keyArgs)
        elif(len(posArgs) == 2):
            return apply(self.lerpScaleVBase3, posArgs, keyArgs)
        else:
            # bad number off args
            raise Exception("Error: NodePath.lerpScale: bad number of args")

    def lerpScaleVBase3(self, scale, time, blendType="noBlend", auto=None,
                      task=None, other=None):
        """lerpPos(self, VBase3, float, string="noBlend", string=none,
        string=none, NodePath=None)
        """
        import ScaleLerpFunctor
        if (other != None):
            # lerp wrt other
            functor = ScaleLerpFunctor.ScaleLerpFunctor(self,
                                       (self.getScale(other)),
                                       scale, other)
        else:
            functor = ScaleLerpFunctor.ScaleLerpFunctor(self,
                                       (self.getScale()), scale)

        #determine whether to use auto, spawned, or blocking lerp
        if (auto != None):
            return self.__autoLerp(functor, time, blendType, auto)
        elif (task != None):
            return self.__lerp(functor, time, blendType, task)
        else:
            return self.__lerp(functor, time, blendType)

    def lerpScaleXYZ(self, sx, sy, sz, time, blendType="noBlend",
                     auto=None, task=None, other=None):
        """lerpPos(self, float, float, float, float, string="noBlend",
        string=none, string=none, NodePath=None)
        """
        import ScaleLerpFunctor
        if (other != None):
            # lerp wrt other
            startScale = self.getScale(other)
            functor = ScaleLerpFunctor.ScaleLerpFunctor(self,
                                       startScale[0], startScale[1],
                                       startScale[2], sx, sy, sz, other)
        else:
            startScale = self.getScale()
            functor = ScaleLerpFunctor.ScaleLerpFunctor(self,
                                       startScale[0], startScale[1],
                                       startScale[2], sx, sy, sz)
        #determine whether to use auto, spawned, or blocking lerp
        if (auto != None):
            return self.__autoLerp(functor, time, blendType, auto)
        elif (task != None):
            return self.__lerp(functor, time, blendType, task)
        else:
            return self.__lerp(functor, time, blendType)
            














