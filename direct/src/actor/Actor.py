"""Actor module: contains the Actor class"""

from PandaObject import *
import LODNode

class Actor(PandaObject, NodePath):
    """Actor class: Contains methods for creating, manipulating
    and playing animations on characters"""

    #create the Actor class globals (ewww!)
    notify = directNotify.newCategory("Actor")
    partPrefix = "__Actor_"

    
    #special methods
    
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
            self.__initialized
            return
        
        except:
            self.__initialized = 1

        # initialize our NodePath essence
        NodePath.__init__(self)

        # create data structures
        self.__partBundleDict = {}
        self.__animControlDict = {}

        if (other == None):
            # act like a normal contructor

            # create base hierarchy
            self.assign(hidden.attachNewNode('actor'))
            self.setGeomNode(self.attachNewNode('actorGeom'))
            self.__LODNode = None
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
            # assign these elements to ourselve
            self.assign(otherCopy)
            self.setGeomNode(otherCopy.getChild(0))
            
            # copy the part dictionary from other
            self.__copyPartBundles(other)
            
            # copy the anim dictionary from other
            self.__copyAnimControls(other)
            
 
    def __str__(self):
        """__str__(self)
        Actor print function"""
        return "Actor: partBundleDict = %s,\n animControlDict = %s" % \
               (self.__partBundleDict, self.__animControlDict)
        

    # accessing
    
    def getLODNames(self):
        """getLODNames(self):
        Return list of Actor LOD names. If not an LOD actor,
        returns 'lodRoot'"""
        return self.__partBundleDict.keys()
    
    def getPartNames(self):
        """getPartNames(self):
        Return list of Actor part names. If not an multipart actor,
        returns 'modelRoot' NOTE: returns parts of first LOD"""
        return self.__partBundleDict.values()[0].keys()
    
    def getGeomNode(self):
        """getGeomNode(self)
        Return the node that contains all actor geometry"""
        return self.__geomNode

    def setGeomNode(self, node):
        """setGeomNode(self, node)
        Set the node that contains all actor geometry"""
        self.__geomNode = node

    def getLODNode(self):
        """getLODNode(self)
        Return the node that switches actor geometry in and out"""
        return self.__LODNode.node()

    def setLODNode(self, node=None):
        """setLODNode(self, LODNode=None)
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
        """resetLOD(self)
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
    
    def getFrameRate(self, animName=None, partName=None):
        """getFrameRate(self, string, string=None)
        Return duration of given anim name and given part.
        If no anim specified, use the currently playing anim.
        If no part specified, return anim durations of first part.
        NOTE: returns info only for the first LOD"""
        # use the first LOD
        lodName = self.__animControlDict.keys()[0]

        if (partName == None):
            partName = self.__animControlDict[lodName].keys()[0]
    
        if (animName==None):
            animName = self.getCurrentAnim(partName)

        # get duration for named part only
        if (self.__animControlDict[lodName].has_key(partName)):        
            animControl = self.getAnimControl(animName, partName, lodName)
            if (animControl != None):
                return animControl.getFrameRate()
        else:
            Actor.notify.warning("no part named %s" % (partName))
        return None

    def getPlayRate(self, animName=None, partName=None):
        """getPlayRate(self, string=None, string=None)
        Return the play rate of given anim for a given part.
        If no part is given, assume first part in dictionary.
        If no anim is given, find the current anim for the part.
        NOTE: Returns info only for the first LOD"""
        # use the first lod
        lodName = self.__animControlDict.keys()[0]
        
        if (partName==None):
            partName = self.__animControlDict[lodName].keys()[0]

        if (animName==None):
            animName = self.getCurrentAnim(partName)
            
        animControl = self.getAnimControl(animName, partName, lodName)
        if (animControl != None):
            return animControl.getPlayRate()
        else:
            return None
    
    def setPlayRate(self, rate, animName=None, partName=None):
        """getPlayRate(self, float, string=None, string=None)
        Set the play rate of given anim for a given part.
        If no part is given, set for all parts in dictionary.
        If no anim is given, find the current anim for the part.
        NOTE: sets play rate on all LODs"""
        # make a list of partNames for loop below
        for lodName in self.__animControlDict.keys():
            animControlDict = self.__animControlDict[lodName]
            if (partName==None):
                partNames = animControlDict.keys()
            else:
                partNames = []
                partNames.append(partName)

            # for each part in list, set play rate on given or current anim
            for partName in partNames:
                if (animName==None):
                    animName = self.getCurrentAnim(partName)          
                animControl = self.getAnimControl(animName, partName, lodName)
                if (animControl != None):
                    animControl.setPlayRate(rate)
            
    def getDuration(self, animName=None, partName=None):
        """getDuration(self, string, string=None)
        Return duration of given anim name and given part.
        If no anim specified, use the currently playing anim.
        If no part specified, return anim duration of first part.
        NOTE: returns info for first LOD only"""
        lodName = self.__animControlDict.keys()[0]
        if (partName == None):
            partName = self.__animControlDict[lodName].keys()[0]

        if (animName==None):
            animName = self.getCurrentAnim(partName)          

        # get duration for named part only
        if (self.__animControlDict[lodName].has_key(partName)):        
            animControl = self.getAnimControl(animName, partName, lodName)
            if (animControl != None):
                return (animControl.getNumFrames() / \
                        animControl.getFrameRate())
        else:
            Actor.notify.warning("no part named %s" % (partName))

        return None

    def getNumFrames(self, animName=None, partName=None):
	""" getNumFrames(animName, partName)
	"""
	lodName = self.__animControlDict.keys()[0]
	if (partName == None):
	    partName = self.__animControlDict[lodName].keys()[0]
	if (animName == None):
	    animName = self.getCurrentAnim(partName)
	if (self.__animControlDict[lodName].has_key(partName)):
	    animControl = self.getAnimControl(animName, partName, lodName)
	    if (animControl != None):
		return animControl.getNumFrames()
	    else:
		Actor.notify.error('no anim control!')
	else:
	    Actor.notify.warning('no part named: %s' % (partName))
        
    def getCurrentAnim(self, partName=None):
        """getCurrentAnim(self, string=None)
        Return the anim current playing on the actor. If part not
        specified return current anim of first part in dictionary.
        NOTE: only returns info for the first LOD"""
        lodName = self.__animControlDict.keys()[0]
        if (partName==None):
            partName = self.__animControlDict[lodName].keys()[0]

        # loop through all anims for named part and find if any are playing
        if (self.__animControlDict[lodName].has_key(partName)):
            for animName in self.__animControlDict[lodName][partName].keys():
                if (self.getAnimControl(animName, partName, lodName).isPlaying()):
                    return animName
        else:
            Actor.notify.warning("no part named %s" % (partName))

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
            joint.addNetTransform(node.arc())
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

        If mode == 0, the geometry is simply arranged to be drawn in
        the correct order.

        If mode < 0, one step further is taken: frontPart is drawn
        as a decal onto backPart.  This assumes that frontPart is
        mostly coplanar with and does not extend beyond backPart, and
        that backPart is mostly flat (not self-occluding).

        If mode > 0, the frontPart geometry is placed in the 'fixed'
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
                frontParts.getPath(partNum).setBin('fixed', mode)
            return

        # Find the back part.
        backPart = root.find("**/" + backPartName)
        if (backPart.isEmpty()):
            Actor.notify.warning("no part named %s!" % (backPartName))
            return
        
        if mode < 0:
            # Draw as a decal.
            dt = DecalTransition()
        else:
            dt = DirectRenderTransition()

        # make the back part have the proper transition
        backPart.arc().setTransition(dt)

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

    def showBounds(self):
        """showBounds(self)
        Show the bounds of all actor geoms"""
        geomNodes = self.findAllMatches("**/+GeomNode")
        numGeomNodes = geomNodes.getNumPaths()

        for nodeNum in range(0, numGeomNodes):
            geomNodes.getPath(nodeNum).showBounds()

    def hideBounds(self):
        """hideBounds(self)
        Hide the bounds of all actor geoms"""
        geomNodes = self.findAllMatches("**/+GeomNode")
        numGeomNodes = geomNodes.getNumPaths()

        for nodeNum in range(0, numGeomNodes):
            geomNodes.getPath(nodeNum).hideBounds()
                    

    # actions
    
    def stop(self, animName=None, partName=None):
        """stop(self, string=None, string=None)
        Stop named animation on the given part of the actor.
        If no name specified then stop all animations on the actor.
        NOTE: stops all LODs"""
        for lodName in self.__animControlDict.keys():
            animControlDict = self.__animControlDict[lodName]
            if (animName == None):
                # loop and stop all anims
                if (partName == None):
                    # loop over all parts
                    for thisPart in animControlDict.keys():
                        for thisAnim in animControlDict[thisPart].keys():
                            animControlDict[thisPart][thisAnim].stop()
                else:
                    # stop just this part
                    for thisAnim in animControlDict[partName].keys():
                        animControlDict[partName][thisAnim].stop()
            else:
                # stop the specified anim
                if (partName == None):
                    # loop over all parts
                    for thisPart in animControlDict.keys():
                        animControlDict[thisPart][animName].stop()
                else:
                    animControlDict[partName][animName].stop()
                    
        
    def play(self, animName, partName=None):
        """play(self, string, string=None)
        Play the given animation on the given part of the actor.
        If no part is specified, try to play on all parts. NOTE:
        plays over ALL LODs"""
        for lodName in self.__animControlDict.keys():
            animControlDict = self.__animControlDict[lodName]
            if (partName == None):
                # loop all parts
                for thisPart in animControlDict.keys():            
                    animControl = self.getAnimControl(animName, thisPart,
                                                        lodName)
                    if (animControl != None):
                        animControl.play()

            else:
                animControl = self.getAnimControl(animName, partName,
                                                    lodName)
                if (animControl != None):
                    animControl.play()


    def loop(self, animName, restart=1, partName=None):
        """loop(self, string, int=1, string=None)
        Loop the given animation on the given part of the actor,
        restarting at zero frame if requested. If no part name
        is given then try to loop on all parts. NOTE: loops on
        all LOD's"""
        for lodName in self.__animControlDict.keys():
            animControlDict = self.__animControlDict[lodName]
            if (partName == None):
                # loop all parts
                for thisPart in animControlDict.keys():
                    animControl = self.getAnimControl(animName, thisPart,
                                                        lodName)
                    if (animControl != None):
                        animControl.loop(restart)
            else:
                # loop a specific part
                animControl = self.getAnimControl(animName, partName,
                                                    lodName)
                if (animControl != None):
                    animControl.loop(restart)
        
    def pose(self, animName, frame, partName=None):
        """pose(self, string, int, string=None)
        Pose the actor in position found at given frame in the specified
        animation for the specified part. If no part is specified attempt
        to apply pose to all parts. NOTE: poses all LODs"""
        for lodName in self.__animControlDict.keys():
            animControlDict = self.__animControlDict[lodName]
            if (partName==None):
                # pose all parts
                for thisPart in animControlDict.keys():
                    animControl = self.getAnimControl(animName, thisPart,
                                                        lodName)
                    if (animControl != None):
                        animControl.pose(frame)
            else:
                # pose a specific part
                animControl = self.getAnimControl(animName, partName,
                                                    lodName)
                if (animControl != None):
                    animControl.pose(frame)
        
    def getAnimControl(self, animName, partName, lodName="lodRoot"):
        """getAnimControl(self, string, string, string="lodRoot")
        Search the animControl dictionary indicated by lodName for
        a given anim and part. Return the animControl if present,
        or None otherwise
        """
        if (self.__animControlDict.has_key(lodName)):
            animControlDict = self.__animControlDict[lodName]
            if (animControlDict.has_key(partName)):
                if (animControlDict[partName].has_key(animName)):
                    return animControlDict[partName][animName]
                else:
                    # anim was not present
                    Actor.notify.warning("couldn't find anim: %s" % (animName))
            else:
                # part was not present
                Actor.notify.warning("couldn't find part: %s" % (partName))
        else:
            # lod was not present
            Actor.notify.warning("couldn't find lod: %s" % (lodName))
            assert(0)
            
        return None

            
    def loadModel(self, modelPath, partName="modelRoot", lodName="lodRoot",
                  copy = 1):
        """loadModel(self, string, string="modelRoot", string="lodRoot",
        bool = 0)
        Actor model loader. Takes a model name (ie file path), a part
        name(defaults to "modelRoot") and an lod name(defaults to "lodRoot").
        If copy is set to 0, do a lodModelOnce instead of a loadModelCopy.
        """
        Actor.notify.debug("in loadModel: %s , part: %s, lod: %s, copy: %s" % \
            (modelPath, partName, lodName, copy))

        # load the model and extract its part bundle
        if (copy):
            model = loader.loadModelCopy(modelPath)
        else:
            model = loader.loadModelOnce(modelPath)

        if (model == None):
            print "model = None!!!"
            
        bundle = model.find("**/+PartBundleNode")
        if (bundle.isEmpty()):
            Actor.notify.warning("%s is not a character!" % (modelPath))
        else:
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
                bundle = bundle.instanceTo(
                    self.__LODNode.find("**/" + str(lodName)))
            else:
                bundle = bundle.instanceTo(self.__geomNode)

            if (needsDict):
                bundleDict[partName] = bundle
                self.__partBundleDict[lodName] = bundleDict
            else:
                self.__partBundleDict[lodName][partName] = bundle

            model.removeNode()        


    def loadAnims(self, anims, partName="modelRoot", lodName="lodRoot"):
        """loadAnims(self, string:string{}, string='modelRoot',
        string='lodRoot')
        Actor anim loader. Takes an optional partName (defaults to
        'modelRoot' for non-multipart actors) and lodName (defaults
        to 'lodRoot' for non-LOD actors) and dict of corresponding
        anims in the form animName:animPath{}"""
        
        Actor.notify.debug("in loadAnims: %s, part: %s, lod: %s" %
                           (anims, partName, lodName))

        animDict = {}

        for animName in anims.keys():
            
            # load the anim and get its anim bundle
            anim = loader.loadModelOnce(anims[animName])
            animBundle = \
                (anim.find("**/+AnimBundleNode").node()).getBundle()

            # bind anim
            bundleNode = (
                self.__partBundleDict[lodName][partName]).node()
                
            animControl = (bundleNode.getBundle()).bindAnim(animBundle, -1)
            if (animControl == None):
                Actor.notify.error("Null AnimControl: %s" % (animName))
            else:
                animDict[animName] = animControl
            
            # add this part's dictionary to animation dictionary
            if not (self.__animControlDict.has_key(lodName)):
                lodDict = {}
                self.__animControlDict[lodName] = lodDict
                
            self.__animControlDict[lodName][partName] = animDict
                

    def __copyPartBundles(self, other):
        """__copyPartBundles(self, Actor)
        Copy the part bundle dictionary from another actor as this
        instance's own. NOTE: this method does not actually copy geometry"""
        for partName in other.__partBundleDict.keys():
            print("copyPart: copying part named = %s" % (partName))
            # find the part in our tree
            partBundle = self.find("**/" + Actor.partPrefix + partName) 
            if (partBundle != None):
                # store the part bundle
                self.__partBundleDict[partName] = partBundle
            else:
                Actor.notify.error("couldn't find matching part:  %s" % \
                                   partName)


    def __copyAnimControls(self, other):
        """__copyAnimControls(self, Actor)
        Get the anims from the anim control's in the anim control
        dictionary of another actor. Bind these anim's to the part
        bundles in our part bundle dict that have matching names, and
        store the resulting anim controls in our own part bundle dict"""
        for partName in other.__animControlDict.keys():
            print("copyAnim: partName = %s" % (partName))
            self.__animControlDict[partName] = {}
            for animName in other.__animControlDict[partName].keys():
                print("    anim: %s" % (animName))
                # get the anim
                animBundle = \
                    other.__animControlDict[partName][animName].getAnim()
                # get the part
                partBundleNode = (self.__partBundleDict[partName].node())
                # bind the anim
                animControl = \
                    (partBundleNode.getBundle().bindAnim(animBundle, -1))
                if (animControl == None):
                    Actor.notify.error("Null animControl: %s" % (animName))
                else:
                    # store the anim control
                    self.__animControlDict[partName][animName] = animControl
                
