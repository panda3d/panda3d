"""Actor module: contains the Actor class"""

from PandaObject import *
import LODNode

class Actor(PandaObject, NodePath):
    """
    Actor class: Contains methods for creating, manipulating
    and playing animations on characters
    """
    notify = directNotify.newCategory("Actor")
    partPrefix = "__Actor_"
    
    def __init__(self, models=None, anims=None, other=None):
        """__init__(self, string | string:string{}, string:string{} |
        string:(string:string{}){}, Actor=None)
        Actor constructor: can be used to create single or multipart
        actors. If another Actor is supplied as an argument this
        method acts like a copy constructor. Single part actors are
        created by calling with a model and animation dictionary
        (animName:animPath{}) as follows:

           a = Actor("panda-3k.egg", {"walk":"panda-walk.egg" \
                                      "run":"panda-run.egg"})

        This could be displayed and animated as such:

           a.reparentTo(render)
           a.loop("walk")
           a.stop()

        Multipart actors expect a dictionary of parts and a dictionary
        of animation dictionaries (partName:(animName:animPath{}){}) as
        below:
        
            a = Actor(
        
                # part dictionary
                {"head":"char/dogMM/dogMM_Shorts-head-mod", \
                 "torso":"char/dogMM/dogMM_Shorts-torso-mod", \
                 "legs":"char/dogMM/dogMM_Shorts-legs-mod"} , \

                # dictionary of anim dictionaries
                {"head":{"walk":"char/dogMM/dogMM_Shorts-head-walk", \
                         "run":"char/dogMM/dogMM_Shorts-head-run"}, \
                 "torso":{"walk":"char/dogMM/dogMM_Shorts-torso-walk", \
                          "run":"char/dogMM/dogMM_Shorts-torso-run"}, \
                 "legs":{"walk":"char/dogMM/dogMM_Shorts-legs-walk", \
                         "run":"char/dogMM/dogMM_Shorts-legs-run"} \
                 })

        In addition multipart actor parts need to be connected together
        in a meaningful fashion:
        
            a.attach("head", "torso", "joint-head")
            a.attach("torso", "legs", "joint-hips")

        #
        # ADD LOD COMMENT HERE!
        #

        Other useful Actor class functions:

            #fix actor eye rendering
            a.drawInFront("joint-pupil?", "eyes*")

            #fix bounding volumes - this must be done after drawing
            #the actor for a few frames, otherwise it has no effect
            a.fixBounds()
        """
        try:
            self.Actor_initialized
            return
        except:
            self.Actor_initialized = 1

        # initialize our NodePath essence
        NodePath.__init__(self)

        # create data structures
        self.__partBundleDict = {}
        self.__animControlDict = {}
        self.__controlJoints = {}
        
        self.__LODNode = None

        if (other == None):
            # act like a normal contructor

            # create base hierarchy
            self.gotName = 0
            root = ModelNode('actor')
            root.setPreserveTransform(1)
            self.assign(NodePath(root))
            self.setGeomNode(self.attachNewNode(ModelNode('actorGeom')))
            self.__hasLOD = 0
            
            # load models
            #
            # four cases:
            #
            #   models, anims{} = single part actor
            #   models{}, anims{} =  single part actor w/ LOD
            #   models{}, anims{}{} = multi-part actor
            #   models{}{}, anims{}{} = multi-part actor w/ LOD
            #
            # make sure we have models
            if (models):
                # do we have a dictionary of models?
                if (type(models)==type({})):
                    # if this is a dictionary of dictionaries
                    if (type(models[models.keys()[0]]) == type({})):
                        # then it must be a multipart actor w/LOD
                        self.setLODNode()
                        # preserve numerical order for lod's
                        # this will make it easier to set ranges
                        sortedKeys = models.keys()
                        sortedKeys.sort()
                        for lodName in sortedKeys:
                            # make a node under the LOD switch
                            # for each lod (just because!)
                            self.addLOD(str(lodName))
                            # iterate over both dicts
                            for modelName in models[lodName].keys():
                                self.loadModel(models[lodName][modelName],
                                               modelName, lodName)
                    # then if there is a dictionary of dictionaries of anims
                    elif (type(anims[anims.keys()[0]])==type({})):
                        # then this is a multipart actor w/o LOD
                        for partName in models.keys():
                            # pass in each part
                            self.loadModel(models[partName], partName)
                    else:
                        # it is a single part actor w/LOD
                        self.setLODNode()
                        # preserve order of LOD's
                        sortedKeys = models.keys()
                        sortedKeys.sort()
                        for lodName in sortedKeys:
                            self.addLOD(str(lodName))
                            # pass in dictionary of parts
                            self.loadModel(models[lodName], lodName=lodName)
                else:
                    # else it is a single part actor
                    self.loadModel(models)

            # load anims
            # make sure the actor has animations
            if (anims):
                if (len(anims) >= 1):
                    # if so, does it have a dictionary of dictionaries?
                    if (type(anims[anims.keys()[0]])==type({})):
                        # are the models a dict of dicts too?
                        if (type(models)==type({})):
                            if (type(models[models.keys()[0]]) == type({})):
                                # then we have a multi-part w/ LOD
                                sortedKeys = models.keys()
                                sortedKeys.sort()
                                for lodName in sortedKeys:
                                    # iterate over both dicts
                                    for partName in anims.keys():
                                        self.loadAnims(
                                            anims[partName], partName, lodName)
                            else:
                                # then it must be multi-part w/o LOD
                                for partName in anims.keys():
                                    self.loadAnims(anims[partName], partName)
                    elif (type(models)==type({})):
                        # then we have single-part w/ LOD
                        sortedKeys = models.keys()
                        sortedKeys.sort()
                        for lodName in sortedKeys:
                            self.loadAnims(anims, lodName=lodName)
                    else:
                        # else it is single-part w/o LOD
                        self.loadAnims(anims)

        else:
            # act like a copy constructor

            # copy the scene graph elements of other
            otherCopy = other.copyTo(hidden)
            otherCopy.detachNode()
            # assign these elements to ourselve
            self.gotName = other.gotName
            self.assign(otherCopy)
            self.setGeomNode(otherCopy.getChild(0))
            
            # copy the part dictionary from other
            self.__copyPartBundles(other)
            
            # copy the anim dictionary from other
            self.__copyAnimControls(other)

        # For now, all Actors will by default set their top bounding
        # volume to be the "final" bounding volume: the bounding
        # volumes below the top volume will not be tested.  If a cull
        # test passes the top bounding volume, the whole Actor is
        # rendered.

        # We do this partly because an Actor is likely to be a fairly
        # small object relative to the scene, and is pretty much going
        # to be all onscreen or all offscreen anyway; and partly
        # because of the Character bug that doesn't update the
        # bounding volume for pieces that animate away from their
        # original position.  It's disturbing to see someone's hands
        # disappear; better to cull the whole object or none of it.
        self.__geomNode.node().setFinal(1)
            
    def delete(self):
        try:
            self.Actor_deleted
            return
        except:
            self.Actor_deleted = 1
            self.cleanup()

    def __cmp__(self, other):
        # Actor inherits from NodePath, which inherits a definition of
        # __cmp__ from FFIExternalObject that uses the NodePath's
        # compareTo() method to compare different NodePaths.  But we
        # don't want this behavior for Actors; Actors should only be
        # compared pointerwise.  A NodePath that happens to reference
        # the same node is still different from the Actor.
        if self is other:
            return 0
        else:
            return 1

    def __str__(self):
        """
        Actor print function
        """
        return "Actor: partBundleDict = %s,\n animControlDict = %s" % \
               (self.__partBundleDict, self.__animControlDict)

    def getActorInfo(self):
        """
        Utility function to create a list of information about a actor.
        Useful for iterating over details of an actor.
        """
        lodInfo = []
        for lodName in self.__animControlDict.keys():
            partDict = self.__animControlDict[lodName]
            partInfo = []
            for partName in partDict.keys():
                partBundle = self.__partBundleDict[lodName][partName]
                animDict = partDict[partName]
                animInfo = []
                for animName in animDict.keys():
                    file = animDict[animName][0]
                    animControl = animDict[animName][1]
                    animInfo.append([animName, file, animControl])
                partInfo.append([partName, partBundle, animInfo])
            lodInfo.append([lodName, partInfo])
        return lodInfo

    def getAnimNames(self):
        animNames = []
        for lodName, lodInfo in self.getActorInfo():
            for partName, bundle, animInfo in lodInfo:
                for animName, file, animControl in animInfo:
                    if animName not in animNames:
                        animNames.append(animName)
        return animNames

    def pprint(self):
        """
        Pretty print actor's details
        """
        for lodName, lodInfo in self.getActorInfo():
            print 'LOD:', lodName
            for partName, bundle, animInfo in lodInfo:
                print '  Part:', partName
                print '  Bundle:', `bundle`
                for animName, file, animControl in animInfo:
                    print '    Anim:', animName
                    print '      File:', file
                    if animControl == None:
                        print ' (not loaded)'
                    else:
                        print ('      NumFrames: %d PlayRate: %0.2f' %
                               (animControl.getNumFrames(),
                                animControl.getPlayRate()))

    def cleanup(self):
        """
        Actor cleanup function
        """
        self.stop()
        del self.__partBundleDict
        del self.__animControlDict
        self.__geomNode.removeNode()
        del self.__geomNode
        if self.__LODNode:
            self.__LODNode.removeNode()
        del self.__LODNode
        self.__hasLOD = 0
        if not self.isEmpty():
            self.removeNode()
    # accessing

    def getAnimControlDict(self):
        return self.__animControlDict

    def getPartBundleDict(self):
        return self.__partBundleDict

        
    def getLODNames(self):
        """getLODNames(self):
        Return list of Actor LOD names. If not an LOD actor,
        returns 'lodRoot'
        Sorts them from highest lod to lowest.
        """
        lodNames = self.__partBundleDict.keys()
        # Reverse sort the doing a string->int
        lodNames.sort(lambda x,y : cmp(int(y), int(x)))
        return lodNames
    
    def getPartNames(self):
        """getPartNames(self):
        Return list of Actor part names. If not an multipart actor,
        returns 'modelRoot' NOTE: returns parts of arbitrary LOD"""
        return self.__partBundleDict.values()[0].keys()
    
    def getGeomNode(self):
        """getGeomNode(self)
        Return the node that contains all actor geometry"""
        return self.__geomNode

    def setGeomNode(self, node):
        """
        Set the node that contains all actor geometry"""
        self.__geomNode = node

    def getLODNode(self):
        """
        Return the node that switches actor geometry in and out"""
        return self.__LODNode.node()

    def setLODNode(self, node=None):
        """
        Set the node that switches actor geometry in and out.
        If one is not supplied as an argument, make one"""
        if (node == None):
            lod = LODNode.LODNode("lod")
            self.__LODNode = self.__geomNode.attachNewNode(lod)
        else:
            self.__LODNode = self.__geomNode.attachNewNode(node)            
        self.__hasLOD = 1
        self.switches = {}

    def useLOD(self, lodName):
        """useLOD(self, string)
        Make the Actor ONLY display the given LOD"""
        # make sure we don't call this twice in a row
        # and pollute the the switches dictionary
        self.resetLOD()
        # store the data in the switches for later use
        sortedKeys = self.switches.keys()
        sortedKeys.sort()
        for eachLod in sortedKeys:
            index = sortedKeys.index(eachLod)
            # set the switches to not display ever
            self.__LODNode.node().setSwitch(index, 0, 10000)
        # turn the given LOD on 'always'
        index = sortedKeys.index(lodName)
        self.__LODNode.node().setSwitch(index, 10000, 0)

    def printLOD(self):
        sortedKeys = self.switches.keys()
        sortedKeys.sort()
        for eachLod in sortedKeys:
            print "python switches for %s: in: %d, out %d" % (eachLod,
                                              self.switches[eachLod][0],
                                              self.switches[eachLod][1])

        switchNum = self.__LODNode.node().getNumSwitches()
        for eachSwitch in range(0, switchNum):
            print "c++ switches for %d: in: %d, out: %d" % (eachSwitch,
                   self.__LODNode.node().getIn(eachSwitch),
                   self.__LODNode.node().getOut(eachSwitch))
                                                        
            
    def resetLOD(self):
        """
        Restore all switch distance info (usually after a useLOD call)"""
        sortedKeys = self.switches.keys()
        sortedKeys.sort()
        for eachLod in sortedKeys:
            index = sortedKeys.index(eachLod)
            self.__LODNode.node().setSwitch(index, self.switches[eachLod][0],
                                     self.switches[eachLod][1])
                    
    def addLOD(self, lodName, inDist=0, outDist=0):
        """addLOD(self, string) 
        Add a named node under the LODNode to parent all geometry
        of a specific LOD under."""
        self.__LODNode.attachNewNode(str(lodName))
        # save the switch distance info
        self.switches[lodName] = [inDist, outDist]
        # add the switch distance info
        self.__LODNode.node().addSwitch(inDist, outDist)

    def setLOD(self, lodName, inDist=0, outDist=0):
        """setLOD(self, string)
        Set the switch distance for given LOD"""
        # save the switch distance info
        self.switches[lodName] = [inDist, outDist]
        # add the switch distance info
        sortedKeys = self.switches.keys()
        sortedKeys.sort()
        index = sortedKeys.index(lodName)
        self.__LODNode.node().setSwitch(index, inDist, outDist)

    def getLOD(self, lodName):
        """getLOD(self, string)
        Get the named node under the LOD to which we parent all LOD
        specific geometry to. Returns 'None' if not found"""
        lod = self.__LODNode.find("**/" + str(lodName))
        if lod.isEmpty():
            return None
        else:
            return lod
        
    def hasLOD(self):
        """hasLOD(self)
        Return 1 if the actor has LODs, 0 otherwise"""
        return self.__hasLOD

    def update(self, lod=0):
        """ update(lod)
        """
        lodnames = self.getLODNames()
        if (lod < len(lodnames)):
            partBundles = self.__partBundleDict[lodnames[lod]].values()
            for partBundle in partBundles:
                # print "updating: %s" % (partBundle.node())
                partBundle.node().updateToNow()
        else:
            self.notify.warning('update() - no lod: %d' % lod)
    
    def getFrameRate(self, animName=None, partName=None):
        """getFrameRate(self, string, string=None)
        Return actual frame rate of given anim name and given part.
        If no anim specified, use the currently playing anim.
        If no part specified, return anim durations of first part.
        NOTE: returns info only for an arbitrary LOD"""
        lodName = self.__animControlDict.keys()[0]
        controls = self.getAnimControls(animName, partName)
        if len(controls) == 0:
            return None
        
        return controls[0].getFrameRate()
    
    def getBaseFrameRate(self, animName=None, partName=None):
        """getBaseFrameRate(self, string, string=None)
        Return frame rate of given anim name and given part, unmodified
        by any play rate in effect.
        """
        lodName = self.__animControlDict.keys()[0]
        controls = self.getAnimControls(animName, partName)
        if len(controls) == 0:
            return None

        return controls[0].getAnim().getBaseFrameRate()

    def getPlayRate(self, animName=None, partName=None):
        """getPlayRate(self, string=None, string=None)
        Return the play rate of given anim for a given part.
        If no part is given, assume first part in dictionary.
        If no anim is given, find the current anim for the part.
        NOTE: Returns info only for an arbitrary LOD"""
        # use the first lod
        lodName = self.__animControlDict.keys()[0]
        controls = self.getAnimControls(animName, partName)
        if len(controls) == 0:
            return None

        return controls[0].getPlayRate()
    
    def setPlayRate(self, rate, animName, partName=None):
        """setPlayRate(self, float, string, string=None)
        Set the play rate of given anim for a given part.
        If no part is given, set for all parts in dictionary.

        It used to be legal to let the animName default to the
        currently-playing anim, but this was confusing and could lead
        to the wrong anim's play rate getting set.  Better to insist
        on this parameter.
        
        NOTE: sets play rate on all LODs"""
        for control in self.getAnimControls(animName, partName):
            control.setPlayRate(rate)

    def getDuration(self, animName=None, partName=None):
        """getDuration(self, string, string=None)
        Return duration of given anim name and given part.
        If no anim specified, use the currently playing anim.
        If no part specified, return anim duration of first part.
        NOTE: returns info for arbitrary LOD"""
        lodName = self.__animControlDict.keys()[0]
        controls = self.getAnimControls(animName, partName)
        if len(controls) == 0:
            return None

        animControl = controls[0]
        return animControl.getNumFrames() / animControl.getFrameRate()

    def getNumFrames(self, animName=None, partName=None):
        lodName = self.__animControlDict.keys()[0]
        controls = self.getAnimControls(animName, partName)
        if len(controls) == 0:
            return None
        return controls[0].getNumFrames()
        
    def getCurrentAnim(self, partName=None):
        """getCurrentAnim(self, string=None)
        Return the anim currently playing on the actor. If part not
        specified return current anim of an arbitrary part in dictionary.
        NOTE: only returns info for an arbitrary LOD"""
        lodName, animControlDict = self.__animControlDict.items()[0]
        if partName == None:
            partName, animDict = animControlDict.items()[0]
        else:
            animDict = animControlDict.get(partName)
            if animDict == None:
                # part was not present
                Actor.notify.warning("couldn't find part: %s" % (partName))
                return None

        # loop through all anims for named part and find if any are playing
        for animName, anim in animDict.items():
            if isinstance(anim[1], AnimControl) and anim[1].isPlaying():
                return animName

        # we must have found none, or gotten an error
        return None

    def getCurrentFrame(self, animName=None, partName=None):
        """getCurrentAnim(self, string=None)
        Return the current frame number of the anim current playing on
        the actor. If part not specified return current anim of first
        part in dictionary.
        NOTE: only returns info for an arbitrary LOD"""
        lodName, animControlDict = self.__animControlDict.items()[0]
        if partName == None:
            partName, animDict = animControlDict.items()[0]
        else:
            animDict = animControlDict.get(partName)
            if animDict == None:
                # part was not present
                Actor.notify.warning("couldn't find part: %s" % (partName))
                return None

        # loop through all anims for named part and find if any are playing
        for animName, anim in animDict.items():
            if isinstance(anim[1], AnimControl) and anim[1].isPlaying():
                return anim[1].getFrame()

        # we must have found none, or gotten an error
        return None


    # arranging

    def getPart(self, partName, lodName="lodRoot"):
        """getPart(self, string, key="lodRoot")
        Find the named part in the optional named lod and return it, or
        return None if not present"""
        if (self.__partBundleDict.has_key(lodName)):
            partBundleDict = self.__partBundleDict[lodName]
        else:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None
                
        if (partBundleDict.has_key(partName)):
            return partBundleDict[partName]
        else:
            return None

    def removePart(self, partName, lodName="lodRoot"):
        """removePart(self, string, key="lodRoot")
        Remove the geometry and animations of the named part of the
        optional named lod if present.
        NOTE: this will remove child geometry also!"""
        # find the corresponding part bundle dict
        if (self.__partBundleDict.has_key(lodName)):
            partBundleDict = self.__partBundleDict[lodName]
        else:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None

        # find the corresponding anim control dict
        if (self.__animControlDict.has_key(lodName)):
            animControlDict = self.__animControlDict[lodName]
        else:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None

        # remove the part
        if (partBundleDict.has_key(partName)):
            partBundleDict[partName].removeNode()
            del(partBundleDict[partName])

        # remove the animations
        if (animControlDict.has_key(partName)):
            del(animControlDict[partName])
            
    def hidePart(self, partName, lodName="lodRoot"):
        """hidePart(self, string, key="lodName")
        Make the given part of the optionally given lod not render,
        even though still in the tree.
        NOTE: this will affect child geometry"""
        if (self.__partBundleDict.has_key(lodName)):
            partBundleDict = self.__partBundleDict[lodName]
        else:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None

        if (partBundleDict.has_key(partName)):
            partBundleDict[partName].hide()
        else:
            Actor.notify.warning("no part named %s!" % (partName))

    def showPart(self, partName, lodName="lodRoot"):
        """showPart(self, string, key="lodRoot")
        Make the given part render while in the tree.
        NOTE: this will affect child geometry"""
        if (self.__partBundleDict.has_key(lodName)):
            partBundleDict = self.__partBundleDict[lodName]
        else:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None

        if (partBundleDict.has_key(partName)):
            partBundleDict[partName].show()
        else:
            Actor.notify.warning("no part named %s!" % (partName))

    def showAllParts(self, partName, lodName="lodRoot"):
        """showAllParts(self, string, key="lodRoot")
        Make the given part and all its children render while in the tree.
        NOTE: this will affect child geometry"""
        if (self.__partBundleDict.has_key(lodName)):
            partBundleDict = self.__partBundleDict[lodName]
        else:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None

        if (partBundleDict.has_key(partName)):
            partBundleDict[partName].show()
            children = partBundleDict[partName].getChildren()
            numChildren = children.getNumPaths()
            for childNum in range(0, numChildren):
                (children.getPath(childNum)).show()
        else:
            Actor.notify.warning("no part named %s!" % (partName))

    def exposeJoint(self, node, partName, jointName, lodName="lodRoot"):
        """exposeJoint(self, NodePath, string, string, key="lodRoot")
        Starts the joint animating the indicated node.  As the joint
        animates, it will transform the node by the corresponding
        amount.  This will replace whatever matrix is on the node each
        frame."""
        
        if (self.__partBundleDict.has_key(lodName)):
            partBundleDict = self.__partBundleDict[lodName]
        else:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None

        if (partBundleDict.has_key(partName)):
            bundle = partBundleDict[partName].node().getBundle()
        else:
            Actor.notify.warning("no part named %s!" % (partName))
            return None

        # Get a handle to the joint.
        joint = bundle.findChild(jointName)

        if (joint):
            joint.addNetTransform(node.node())
        else:
            Actor.notify.warning("no joint named %s!" % (jointName))

    def stopJoint(self, partName, jointName, lodName="lodRoot"):
        """stopJoint(self, string, string, key="lodRoot")
        Stops the joint from animating external nodes.  If the joint
        is animating a transform on a node, this will permanently stop
        it.  However, this does not affect vertex animations."""
        if (self.__partBundleDict.has_key(lodName)):
            partBundleDict = self.__partBundleDict[lodName]
        else:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None

        if (partBundleDict.has_key(partName)):
            bundle = partBundleDict[partName].node().getBundle()
        else:
            Actor.notify.warning("no part named %s!" % (partName))
            return None

        # Get a handle to the joint.
        joint = bundle.findChild(jointName)

        if (joint):
            joint.clearNetTransforms()
            joint.clearLocalTransforms()
        else:
            Actor.notify.warning("no joint named %s!" % (jointName))

    def controlJoint(self, node, partName, jointName, lodName="lodRoot"):
        """controlJoint(self, NodePath, string, string, key="lodRoot")

        The converse of exposeJoint: this associates the joint with
        the indicated node, so that the joint transform will be copied
        from the node to the joint each frame.  This can be used for
        programmer animation of a particular joint at runtime.

        This must be called before any animations are played.  Once an
        animation has been loaded and bound to the character, it will
        be too late to add a new control during that animation.
        """
        if (self.__partBundleDict.has_key(lodName)):
            partBundleDict = self.__partBundleDict[lodName]
        else:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None

        if (partBundleDict.has_key(partName)):
            bundle = partBundleDict[partName].node().getBundle()
        else:
            Actor.notify.warning("no part named %s!" % (partName))
            return None

        joint = bundle.findChild(jointName)
        if joint == None:
            Actor.notify.warning("no joint named %s!" % (jointName))
            return None

        if node == None:
            node = self.attachNewNode(jointName)

        # Store a dictionary of jointName : node to list the controls
        # requested for joints.  The controls will actually be applied
        # later, when we load up the animations in bindAnim().
        if self.__controlJoints.has_key(bundle):
            self.__controlJoints[bundle][jointName] = node
        else:
            self.__controlJoints[bundle] = { jointName : node }

        return node
            
    def instance(self, path, part, jointName, lodName="lodRoot"):
        """instance(self, NodePath, string, string, key="lodRoot")
        Instance a nodePath to an actor part at a joint called jointName"""
        if (self.__partBundleDict.has_key(lodName)):
            partBundleDict = self.__partBundleDict[lodName]
            if (partBundleDict.has_key(part)):
                joint = partBundleDict[part].find("**/" + jointName)
                if (joint.isEmpty()):
                    Actor.notify.warning("%s not found!" % (jointName))
                else:
                    return path.instanceTo(joint)
            else:
                Actor.notify.warning("no part named %s!" % (part))
        else:
            Actor.notify.warning("no lod named %s!" % (lodName))

    def attach(self, partName, anotherPart, jointName, lodName="lodRoot"):
        """attach(self, string, string, string, key="lodRoot")
        Attach one actor part to another at a joint called jointName"""
        if (self.__partBundleDict.has_key(lodName)):
            partBundleDict = self.__partBundleDict[lodName]
            if (partBundleDict.has_key(partName)):
                if (partBundleDict.has_key(anotherPart)):
                    joint = partBundleDict[anotherPart].find("**/" + jointName)
                    if (joint.isEmpty()):
                        Actor.notify.warning("%s not found!" % (jointName))
                    else:
                        partBundleDict[partName].reparentTo(joint)
                else:
                    Actor.notify.warning("no part named %s!" % (anotherPart))
            else:
                Actor.notify.warning("no part named %s!" % (partName))
        else:
            Actor.notify.warning("no lod named %s!" % (lodName))

                    
    def drawInFront(self, frontPartName, backPartName, mode,
                    root=None, lodName=None):
        """drawInFront(self, string, int, string=None, key=None)

        Arrange geometry so the frontPart(s) are drawn in front of
        backPart.

        If mode == -1, the geometry is simply arranged to be drawn in
        the correct order, assuming it is already under a
        direct-render scene graph (like the DirectGui system).  That
        is, frontPart is reparented to backPart, and backPart is
        reordered to appear first among its siblings.

        If mode == -2, the geometry is arranged to be drawn in the
        correct order, and depth test/write is turned off for
        frontPart.

        If mode == -3, frontPart is drawn as a decal onto backPart.
        This assumes that frontPart is mostly coplanar with and does
        not extend beyond backPart, and that backPart is mostly flat
        (not self-occluding).

        If mode >= 0, the frontPart geometry is placed in the 'fixed'
        bin, with the indicated drawing order.  This will cause it to
        be drawn after almost all other geometry.  In this case, the
        backPartName is actually unused.
        
        Takes an optional argument root as the start of the search for the
        given parts. Also takes optional lod name to refine search for the
        named parts. If root and lod are defined, we search for the given
        root under the given lod."""

        # check to see if we are working within an lod
        if (lodName != None):
            # find the named lod node
            lodRoot = self.find("**/" + str(lodName))
            if (root == None):
                # no need to look further
                root = lodRoot
            else:
                # look for root under lod
                root = lodRoot.find("**/" + root)
        else:
            # start search from self if no root and no lod given
            if (root == None):
                root = self

        frontParts = root.findAllMatches( "**/" + frontPartName)
                
        if mode > 0:
            # Use the 'fixed' bin instead of reordering the scene
            # graph.
            numFrontParts = frontParts.getNumPaths()
            for partNum in range(0, numFrontParts):
                frontParts[partNum].setBin('fixed', mode)
            return

        if mode == -2:
            # Turn off depth test/write on the frontParts.
            numFrontParts = frontParts.getNumPaths()
            for partNum in range(0, numFrontParts):
                frontParts[partNum].setDepthWrite(0)
                frontParts[partNum].setDepthTest(0)
 
        # Find the back part.
        backPart = root.find("**/" + backPartName)
        if (backPart.isEmpty()):
            Actor.notify.warning("no part named %s!" % (backPartName))
            return
                
        if mode == -3:
            # Draw as a decal.
            backPart.node().setEffect(DecalEffect.make())
        else:
            # Reorder the backPart to be the first of its siblings.
            backPart.reparentTo(backPart.getParent(), -1)

        #reparent all the front parts to the back part
        frontParts.reparentTo(backPart)


    def fixBounds(self, part=None):
        """fixBounds(self, nodePath=None)
        Force recomputation of bounding spheres for all geoms
        in a given part. If no part specified, fix all geoms
        in this actor"""
        
        # if no part name specified fix all parts
        if (part==None):
            part = self

        # update all characters first
        charNodes = part.findAllMatches("**/+Character")
        numCharNodes = charNodes.getNumPaths()
        for charNum in range(0, numCharNodes):
            (charNodes.getPath(charNum)).node().update()

        # for each geomNode, iterate through all geoms and force update
        # of bounding spheres by marking current bounds as stale
        geomNodes = part.findAllMatches("**/+GeomNode")
        numGeomNodes = geomNodes.getNumPaths()
        for nodeNum in range(0, numGeomNodes):
            thisGeomNode = geomNodes.getPath(nodeNum)
            numGeoms = thisGeomNode.node().getNumGeoms()
            for geomNum in range(0, numGeoms):
                thisGeom = thisGeomNode.node().getGeom(geomNum)
                thisGeom.markBoundStale()
                Actor.notify.debug("fixing bounds for node %s, geom %s" % \
                                  (nodeNum, geomNum))
            thisGeomNode.node().markBoundStale()

    def showAllBounds(self):
        """showAllBounds(self)
        Show the bounds of all actor geoms"""
        geomNodes = self.__geomNode.findAllMatches("**/+GeomNode")
        numGeomNodes = geomNodes.getNumPaths()

        for nodeNum in range(0, numGeomNodes):
            geomNodes.getPath(nodeNum).showBounds()

    def hideAllBounds(self):
        """hideAllBounds(self)
        Hide the bounds of all actor geoms"""
        geomNodes = self.__geomNode.findAllMatches("**/+GeomNode")
        numGeomNodes = geomNodes.getNumPaths()

        for nodeNum in range(0, numGeomNodes):
            geomNodes.getPath(nodeNum).hideBounds()
                    

    # actions
    def animPanel(self):
        import TkGlobal
        import AnimPanel
        return AnimPanel.AnimPanel(self)
    
    def stop(self, animName=None, partName=None):
        """stop(self, string=None, string=None)
        Stop named animation on the given part of the actor.
        If no name specified then stop all animations on the actor.
        NOTE: stops all LODs"""
        for control in self.getAnimControls(animName, partName):
            control.stop()
        
    def play(self, animName, partName=None, fromFrame=None, toFrame=None):
        """play(self, string, string=None)
        Play the given animation on the given part of the actor.
        If no part is specified, try to play on all parts. NOTE:
        plays over ALL LODs"""
        if fromFrame == None:
            for control in self.getAnimControls(animName, partName):
                control.play()
        else:
            for control in self.getAnimControls(animName, partName):
                control.play(fromFrame, toFrame)

    def loop(self, animName, restart=1, partName=None,
             fromFrame=None, toFrame=None):
        """loop(self, string, int=1, string=None)
        Loop the given animation on the given part of the actor,
        restarting at zero frame if requested. If no part name
        is given then try to loop on all parts. NOTE: loops on
        all LOD's"""
        if fromFrame == None:
            for control in self.getAnimControls(animName, partName):
                control.loop(restart)
        else:
            for control in self.getAnimControls(animName, partName):
                control.loop(restart, fromFrame, toFrame)

    def pingpong(self, animName, fromFrame, toFrame, restart=1, partName=None):
        """pingpong(self, string, fromFrame, toFrame, int=1, string=None)
        Loop the given animation on the given part of the actor,
        restarting at zero frame if requested. If no part name
        is given then try to loop on all parts. NOTE: loops on
        all LOD's"""
        for control in self.getAnimControls(animName, partName):
            control.pingpong(restart, fromFrame, toFrame)
        
    def pose(self, animName, frame, partName=None, lodName=None):
        """pose(self, string, int, string=None)
        Pose the actor in position found at given frame in the specified
        animation for the specified part. If no part is specified attempt
        to apply pose to all parts."""
        for control in self.getAnimControls(animName, partName, lodName):
            control.pose(frame)

    def enableBlend(self, blendType = PartBundle.BTNormalizedLinear, partName = None):
        """
        Enables blending of multiple animations simultaneously.
        After this is called, you may call play(), loop(), or pose()
        on multiple animations and have all of them contribute to the
        final pose each frame.

        With blending in effect, starting a particular animation with
        play(), loop(), or pose() does not implicitly make the
        animation visible; you must also call setControlEffect() for
        each animation you wish to use to indicate how much each
        animation contributes to the final pose.
        """
        for lodName, bundleDict in self.__partBundleDict.items():
            if partName == None:
                for partBundle in bundleDict.values():
                    partBundle.node().getBundle().setBlendType(blendType)
            else:
                partBundle = bundleDict.get(partName)
                if partBundle != None:
                    partBundle.node().getBundle().setBlendType(blendType)
                else:
                    Actor.notify.warning("Couldn't find part: %s" % (partName))

    def disableBlend(self, partName = None):
        """ Restores normal one-animation-at-a-time operation after a
        previous call to enableBlend().
        """
        self.enableBlend(PartBundle.BTSingle, partName)

    def setControlEffect(self, animName, effect,
                         partName = None, lodName = None):
        """
        Sets the amount by which the named animation contributes to
        the overall pose.  This controls blending of multiple
        animations; it only makes sense to call this after a previous
        call to enableBlend().
        """
        for control in self.getAnimControls(animName, partName, lodName):
            control.getPart().setControlEffect(control, effect)

    def getAnimControl(self, animName, partName, lodName="lodRoot"):
        """getAnimControl(self, string, string, string="lodRoot")
        Search the animControl dictionary indicated by lodName for
        a given anim and part. Return the animControl if present,
        or None otherwise
        """
        animControlDict = self.__animControlDict.get(lodName)
        # if this assertion fails, named lod was not present
        assert(animControlDict != None)

        animDict = animControlDict.get(partName)
        if animDict == None:
            # part was not present
            Actor.notify.warning("couldn't find part: %s" % (partName))
        else:
            anim = animDict.get(animName)
            if anim == None:
                # anim was not present
                Actor.notify.warning("couldn't find anim: %s" % (animName))
            else:
                # bind the animation first if we need to
                if not isinstance(anim[1], AnimControl):
                    self.__bindAnimToPart(animName, partName, lodName)
                return anim[1]
            
        return None
        
    def getAnimControls(self, animName=None, partName=None, lodName=None):
        """getAnimControls(self, string, string=None, string=None)

        Returns a list of the AnimControls that represent the given
        animation for the given part and the given lod.  If animName
        is omitted, the currently-playing animation (or all
        currently-playing animations) is returned.  If partName is
        omitted, all parts are returned.  If lodName is omitted, all
        LOD's are returned.
        """
        controls = []

        # build list of lodNames and corresponding animControlDicts
        # requested.
        if lodName == None:
            # Get all LOD's
            animControlDictItems = self.__animControlDict.items()
        else:
            animControlDict = self.__animControlDict.get(lodName)
            if animControlDict == None:
                Actor.notify.warning("couldn't find lod: %s" % (lodName))
                animControlDictItems = []
            else:
                animControlDictItems = [(lodName, animControlDict)]

        for lodName, animControlDict in animControlDictItems:
            # Now, build the list of partNames and the corresponding
            # animDicts.
            if partName == None:
                # Get all parts
                animDictItems = animControlDict.items()
            else:
                # Get a specific part
                animDict = animControlDict.get(partName)
                if animDict == None:
                    # part was not present
                    Actor.notify.warning("couldn't find part: %s" % (partName))
                    animDictItems = []
                else:
                    animDictItems = [(partName, animDict)]
                
            if animName == None:
                # get all playing animations
                for thisPart, animDict in animDictItems:
                    for anim in animDict.values():
                        if isinstance(anim[1], AnimControl) and anim[1].isPlaying():
                            controls.append(anim[1])
            else:
                # get the named animation only. 
                for thisPart, animDict in animDictItems:
                    anim = animDict.get(animName)
                    if anim == None:
                        # anim was not present
                        Actor.notify.warning("couldn't find anim: %s" % (animName))
                    else:
                        # bind the animation first if we need to
                        if not isinstance(anim[1], AnimControl):
                            if self.__bindAnimToPart(animName, thisPart, lodName):
                                controls.append(anim[1])
                        else:
                            controls.append(anim[1])

        return controls
            
    def loadModel(self, modelPath, partName="modelRoot", lodName="lodRoot", copy = 1):
        """loadModel(self, string, string="modelRoot", string="lodRoot",
        bool = 0)
        Actor model loader. Takes a model name (ie file path), a part
        name(defaults to "modelRoot") and an lod name(defaults to "lodRoot").
        If copy is set to 0, do a lodModelOnce instead of a loadModelCopy.
        """
        Actor.notify.debug("in loadModel: %s , part: %s, lod: %s, copy: %s" % \
            (modelPath, partName, lodName, copy))

        if isinstance(modelPath, NodePath):
            # If we got a NodePath instead of a string, use *that* as
            # the model directly.
            if (copy):
                model = modelPath.copyTo(hidden)
            else:
                model = modelPath
        else:
            # otherwise, we got the name of the model to load.
            if (copy):
                model = loader.loadModelCopy(modelPath)
            else:
                model = loader.loadModelOnce(modelPath)

        if (model == None):
            print "model = None!!!"
        bundle = model.find("**/+PartBundleNode")
        if (bundle.isEmpty()):
            Actor.notify.warning("%s is not a character!" % (modelPath))
            model.reparentTo(self.__geomNode)
        else:
            self.prepareBundle(bundle, partName, lodName)
            model.removeNode()        

    def prepareBundle(self, bundle, partName="modelRoot", lodName="lodRoot"):
        # Rename the node at the top of the hierarchy, if we
        # haven't already, to make it easier to identify this
        # actor in the scene graph.
        if not self.gotName:
            self.node().setName(bundle.node().getName())
            self.gotName = 1

        # we rename this node to make Actor copying easier
        bundle.node().setName(Actor.partPrefix + partName)

        if (self.__partBundleDict.has_key(lodName) == 0):
            # make a dictionary to store these parts in
            needsDict = 1
            bundleDict = {}
        else:
            needsDict = 0

        if (lodName!="lodRoot"):
            # instance to appropriate node under LOD switch
            #bundle = bundle.instanceTo(
            #    self.__LODNode.find("**/" + str(lodName)))
            bundle.reparentTo(self.__LODNode.find("**/" + str(lodName)))
        else:
            #bundle = bundle.instanceTo(self.__geomNode)
            bundle.reparentTo(self.__geomNode)

        if (needsDict):
            bundleDict[partName] = bundle
            self.__partBundleDict[lodName] = bundleDict
        else:
            self.__partBundleDict[lodName][partName] = bundle

    def loadAnims(self, anims, partName="modelRoot", lodName="lodRoot"):
        """loadAnims(self, string:string{}, string='modelRoot',
        string='lodRoot')
        Actor anim loader. Takes an optional partName (defaults to
        'modelRoot' for non-multipart actors) and lodName (defaults
        to 'lodRoot' for non-LOD actors) and dict of corresponding
        anims in the form animName:animPath{}"""
        
        Actor.notify.debug("in loadAnims: %s, part: %s, lod: %s" %
                          (anims, partName, lodName))

        for animName in anims.keys():
            # make sure this lod in in anim control dict
            if not (self.__animControlDict.has_key(lodName)):
                lodDict = {}
                self.__animControlDict[lodName] = lodDict

            # make sure this part dict exists
            if not (self.__animControlDict[lodName].has_key(partName)):
                partDict = {}
                self.__animControlDict[lodName][partName] = partDict

            # make sure this an anim dict exists
            if not (len(self.__animControlDict[lodName][partName].keys())):
                animDict = {}
                self.__animControlDict[lodName][partName] = animDict

            # store the file path and None in place of the animControl.
            # we will bind it only when played
            self.__animControlDict[lodName][partName][animName] = \
                                                                [anims[animName], None]


    def unloadAnims(self, anims, partName="modelRoot", lodName="lodRoot"):
        """unloadAnims(self, string:string{}, string='modelRoot',
        string='lodRoot')
        Actor anim unloader. Takes an optional partName (defaults to
        'modelRoot' for non-multipart actors) and lodName (defaults
        to 'lodRoot' for non-LOD actors) and dict of corresponding
        anims in the form animName:animPath{}. Deletes the anim control
        for the given animation and parts/lods."""
        
        Actor.notify.debug("in unloadAnims: %s, part: %s, lod: %s" %
                           (anims, partName, lodName))

        if (lodName == None):
            lodNames = self.__animControlDict.keys()
        else:
            lodNames = [lodName]

        if (partName == None):
            if len(lodNames) > 0:
                partNames = self.__animControlDict[lodNames[0]].keys()
            else:
                partNames = []
        else:
            partNames = [partName]

        if (anims==None):
            if len(lodNames) > 0 and len(partNames) > 0:
                anims = self.__animControlDict[lodNames[0]][partNames[0]].keys()
            else:
                anims = []

        for lodName in lodNames:
            for partName in partNames:
                for animName in anims:
                    # delete the anim control
                    animControlPair = self.__animControlDict[lodName][partName][animName]
                    if animControlPair[1] != None:
                        del(animControlPair[1])
                        animControlPair.append(None)
    
    def bindAnim(self, animName, partName="modelRoot", lodName="lodRoot"):
        """bindAnim(self, string, string='modelRoot', string='lodRoot')
        Bind the named animation to the named part and lod
        """

        if lodName == None:
            lodNames = self.__animControl.keys()
        else:
            lodNames = [lodName]
        
        # loop over all lods
        for thisLod in lodNames:
            if partName == None:
                partNames = animControlDict[lodName].keys()
            else:
                partNames = [partName]
            # loop over all parts
            for thisPart in partNames:
                ac = self.__bindAnimToPart(animName, thisPart, thisLod)
            
            
    def __bindAnimToPart(self, animName, partName, lodName):
        """for internal use only!"""
        # make sure this anim is in the dict
        if not self.__animControlDict[lodName][partName].has_key(animName):
            Actor.notify.debug("actor has no animation %s", animName)
            
        # only bind if not already bound!
        if isinstance(self.__animControlDict[lodName][partName][animName][1],
                      AnimControl):
            return None

        # fetch a copy from the modelPool, or if we weren't careful
        # enough to preload, fetch from disk :(
        animPath = self.__animControlDict[lodName][partName][animName][0]
        anim = loader.loadModelOnce(animPath)
        if anim == None:
            return None
        animBundle = \
                   (anim.find("**/+AnimBundleNode").node()).getBundle()

        bundle = self.__partBundleDict[lodName][partName].node().getBundle()

        # Are there any controls requested for joints in this bundle?
        # If so, apply them.
        controlDict = self.__controlJoints.get(bundle, None)
        if controlDict:
            for jointName, node in controlDict.items():
                if node:
                    joint = animBundle.makeChildDynamic(jointName)
                    if joint:
                        joint.setValueNode(node.node())

        # bind anim
        animControl = bundle.bindAnim(animBundle, -1)

        if (animControl == None):
            Actor.notify.error("Null AnimControl: %s" % (animName))
        else:
            # store the animControl
            self.__animControlDict[lodName][partName][animName][1] = \
                                                                   animControl
            Actor.notify.debug("binding anim: %s to part: %s, lod: %s" %
                           (animName, partName, lodName))
        return animControl

    def __copyPartBundles(self, other):
        """__copyPartBundles(self, Actor)
        Copy the part bundle dictionary from another actor as this
        instance's own. NOTE: this method does not actually copy geometry"""
        for lodName in other.__partBundleDict.keys():
            self.__partBundleDict[lodName] = {}            
            for partName in other.__partBundleDict[lodName].keys():
                # find the part in our tree
                partBundle = self.find("**/" + Actor.partPrefix + partName) 
                if (partBundle != None):
                    # store the part bundle
                    self.__partBundleDict[lodName][partName] = partBundle
                else:
                    Actor.notify.error("lod: %s has no matching part: %s" %
                                       (lodName, partName))


    def __copyAnimControls(self, other):
        """__copyAnimControls(self, Actor)
        Get the anims from the anim control's in the anim control
        dictionary of another actor. Bind these anim's to the part
        bundles in our part bundle dict that have matching names, and
        store the resulting anim controls in our own part bundle dict"""
        for lodName in other.__animControlDict.keys():
            self.__animControlDict[lodName] = {}        
            for partName in other.__animControlDict[lodName].keys():
                self.__animControlDict[lodName][partName] = {}
                for animName in other.__animControlDict[lodName][partName].keys():
                    # else just copy what's there
                    self.__animControlDict[lodName][partName][animName] = \
                        [other.__animControlDict[lodName][partName][animName][0], None]

    def actorInterval(self, *args, **kw):
        import ActorInterval
        return ActorInterval.ActorInterval(self, *args, **kw)

