"""Actor module: contains the Actor class"""

from PandaObject import *

class Actor(PandaObject, ShowBase.NodePath):
    """Actor class: Contains methods for creating, manipulating
    and playing animations on characters"""

    #create the Actor class DirectNotify category
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


        Other useful Acotr class functions:

            #fix actor eye rendering
            a.drawInFront("joint-pupil?", "eyes*")

            #fix bounding volumes - this must be done after drawing
            #the actor for a few frames, otherwise it has no effect
            a.fixBounds()
            
        """

        # initial our NodePath essence
        NodePath.__init__(self)

        # create data structures
        self.__partBundleDict = {}
        self.__animControlDict = {}

        if (other == None):
            # act like a normal contructor

            # create base hierarchy
            self.assign(hidden.attachNewNode('actor'))
            self.setGeomNode(self.attachNewNode('actorGeom'))
        
            # load models
            # make sure we have models
            if (models):
                # if this is a dictionary
                if (type(models)==type({})):
                    # then it must be multipart actor
                    for partName in models.keys():
                        self.loadModel(models[partName], partName)
                else:
                    # else it is a single part actor
                    self.loadModel(models)

            # load anims
            # make sure the actor has animations
            if (anims):
                if (len(anims) >= 1):
                    # if so, does it have a dictionary of dictionaries
                    if (type(anims[anims.keys()[0]])==type({})):
                        # then it must be multipart
                        for partName in anims.keys():
                            self.loadAnims(anims[partName], partName)
                    else:
                        # else it is not multipart
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
        return "Actor: partBundleDict = %s, animControlDict = %s" % \
               (self.__partBundleDict, self.__animControlDict)
        

    # accessing
    
    def getPartNames(self):
        """getPartNames(self):
        Return list of Actor part names. If not multipart,
        returns modelRoot"""
        return self.__partBundleDict.keys()
    
    def getGeomNode(self):
        """getGeomNode(self)
        Return the node that contains all actor geometry"""
        return self.__geomNode

    def setGeomNode(self, node):
        """setGeomNode(self, node)
        Set the node that contains all actor geometry"""
        self.__geomNode = node

    def getFrameRate(self, animName=None, partName=None):
        """getFrameRate(self, string, string=None)
        Return duration of given anim name and given part.
        If no anim specified, use the currently playing anim.
        If no part specified, return anim durations of first part"""
        if (partName == None):
            partName = self.__animControlDict.keys()[0]
    
        if (animName==None):
            animName = self.getCurrentAnim(partName)

        # get duration for named part only
        if (self.__animControlDict.has_key(partName)):        
            animControl = self.__getAnimControl(animName, partName)
            if (animControl != None):
                return animControl.getFrameRate()
        else:
            Actor.notify.warning("no part named %s" % (partName))

        return None

    def getPlayRate(self, animName=None, partName=None):
        """getPlayRate(self, string=None, string=None)
        Return the play rate of given anim for a given part.
        If no part is given, assume first part in dictionary.
        If no anim is given, find the current anim for the part"""
        if (partName==None):
            partName = self.__animControlDict.keys()[0]

        if (animName==None):
            animName = self.getCurrentAnim(partName)
            
        animControl = self.__getAnimControl(animName, partName)
        if (animControl != None):
            return animControl.getPlayRate()
        else:
            return None
    
    def setPlayRate(self, rate, animName=None, partName=None):
        """getPlayRate(self, float, string=None, string=None)
        Set the play rate of given anim for a given part.
        If no part is given, set for all parts in dictionary.
        If no anim is given, find the current anim for the part"""
        # make a list of partNames for loop below
        if (partName==None):
            partNames = self.__animControlDict.keys()
        else:
            partNames = []
            partNames.append(partName)

        # for each part in list, set play rate on given or current anim
        for partName in partNames:
            if (animName==None):
                animName = self.getCurrentAnim(partName)          
            animControl = self.__getAnimControl(animName, partName)
            if (animControl != None):
                animControl.setPlayRate(rate)
            
    def getDuration(self, animName=None, partName=None):
        """getDuration(self, string, string=None)
        Return duration of given anim name and given part.
        If no anim specified, use the currently playing anim.
        If no part specified, return anim duration of first part"""
        if (partName == None):
            partName = self.__animControlDict.keys()[0]

        if (animName==None):
            animName = self.getCurrentAnim(partName)          

        # get duration for named part only
        if (self.__animControlDict.has_key(partName)):        
            animControl = self.__getAnimControl(animName, partName)
            if (animControl != None):
                return (animControl.getNumFrames() / \
                        animControl.getFrameRate())
        else:
            Actor.notify.warning("no part named %s" % (partName))

        return None
        
    def getCurrentAnim(self, partName=None):
        """getCurrentAnim(self, string=None)
        Return the anim current playing on the actor. If part not
        specified return current anim of first part in dictionary"""
        if (partName==None):
            partName = self.__animControlDict.keys()[0]

        # loop through all anims for named part and find if any are playing
        if (self.__animControlDict.has_key(partName)):
            for animName in self.__animControlDict[partName].keys():
                if (self.__getAnimControl(animName, partName).isPlaying()):
                    return animName
        else:
            Actor.notify.warning("no part named %s" % (partName))

        # we must have found none, or gotten an error
        return None



    # arranging

    def getPart(self, partName):
        """getPart(self, string)
        Find the named part in the partBundleDict and return it, or
        return None if not present"""
        if (self.__partBundleDict.has_key(partName)):
            return self.__partBundleDict[partName]
        else:
            return None

    def removePart(self, partName):
        """removePart(Self, string)
        Remove the geometry and animations of the named part if present
        NOTE: this will remove parented geometry also!"""
        # remove the geometry
        if (self.__partBundleDict.has_key(partName)):
            self.__partBundleDict[partName].removeNode()
            del(self.__partBundleDict[partName])
        # remove the animations
        if (self.__animControlDict.has_key(partName)):
            del(self.__animControlDict[partName])

            
    def hidePart(self, partName):
        """hidePart(self, string)
        Make the given part not render, even though still in the tree.
        NOTE: this functionality will be effected by the 'attach' method"""
        if (self.__partBundleDict.has_key(partName)):
            self.__partBundleDict[partName].hide()
        else:
            Actor.notify.warning("no part named %s!" % (partName))

    def showPart(self, partName):
        """showPart(self, string)
        Make the given part render while in the tree.
        NOTE: this functionality will be effected by the 'attach' method"""
        if (self.__partBundleDict.has_key(partName)):
            self.__partBundleDict[partName].show()
        else:
            Actor.notify.warning("no part named %s!" % (partName))
            
    def instance(self, partName, anotherPart, jointName):
        """instance(self, string, string, string)
        Instance one actor part to another at a joint called jointName"""
        if (self.__partBundleDict.has_key(partName)):
            if (self.__partBundleDict.has_key(anotherPart)):
                joint = NodePath(self.__partBundleDict[anotherPart], \
                                 "**/" + jointName)
                if (joint.isEmpty()):
                    Actor.notify.warning("%s not found!" % (jointName))
                else:
                    return self.__partBundleDict[partName].instanceTo(joint)
            else:
                Actor.notify.warning("no part named %s!" % (anotherPart))
        else:
            Actor.notify.warning("no part named %s!" % (partName))


    def attach(self, partName, anotherPart, jointName):
        """attach(self, string, string, string)
        Attach one actor part to another at a joint called jointName"""
        if (self.__partBundleDict.has_key(partName)):
            if (self.__partBundleDict.has_key(anotherPart)):
                joint = NodePath(self.__partBundleDict[anotherPart], \
                                 "**/" + jointName)
                if (joint.isEmpty()):
                    Actor.notify.warning("%s not found!" % (jointName))
                else:
                    self.__partBundleDict[partName].reparentTo(joint)
            else:
                Actor.notify.warning("no part named %s!" % (anotherPart))
        else:
            Actor.notify.warning("no part named %s!" % (partName))

                    
    def drawInFront(self, frontPartName, backPartName, root=None):
        """drawInFront(self, string, string=None)
        Arrange geometry so the frontPart is drawn properly wrt backPart.
        Takes an optional argument root as the start of the search for the
        given parts"""

        # start search from self if no root given
        if (root==None):
            root = self
            
        # make the back part have the proper transition
        backPart = NodePath(root, "**/"+backPartName)
        if (backPart.isEmpty()):
            Actor.notify.warning("no part named %s!" % (backPartName))
        else:
            (backPart.getBottomArc()).setTransition(DirectRenderTransition())

        #reparent the front parts to the back parts
        frontParts = self.findAllMatches( "**/"+frontPartName)
        numFrontParts = frontParts.getNumPaths()
        for partNum in range(0, numFrontParts):
            (frontParts.getPath(partNum)).reparentTo(backPart)


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
                Actor.notify.info("fixing bounds for node %s, geom %s" % \
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
        If no name specified then stop all animations on the actor"""
        if (animName == None):
            #loop and stop ALL anims
            for animControl in self.__animControlDict[partName].keys():
                self.__animControlDict[partName][animControl].stop()
        else:
            #stop the specified anim
            animControl = self.__getAnimControl(animName, partName)
            if (animControl != None):
                animControl.stop()
        
    def play(self, animName, partName=None):
        """play(self, string, string=None)
        Play the given animation on the given part of the actor.
        If no part is specified, try to play on all parts"""
        if (partName == None):
            # loop all parts
            for partName in self.__animControlDict.keys():            
                animControl = self.__getAnimControl(animName, partName)
                if (animControl != None):
                    animControl.play()
        else:
            animControl = self.__getAnimControl(animName, partName)
            if (animControl != None):
                animControl.play()


    def loop(self, animName, restart=1, partName=None):
        """loop(self, string, int=1, string=None)
        Loop the given animation on the given part of the actor,
        restarting at zero frame if requested. If no part name
        is given then try to loop on all parts"""
        if (partName == None):
            # loop all parts
            for partName in self.__animControlDict.keys():
                animControl = self.__getAnimControl(animName, partName)
                if (animControl != None):
                    animControl.loop(restart)
        else:
            # loop a specific part
            animControl = self.__getAnimControl(animName, partName)
            if (animControl != None):
                animControl.loop(restart)
        
    def pose(self, animName, frame, partName=None):
        """pose(self, string, int, string=None)
        Pose the actor in position found at given frame in the specified
        animation for the specified part. If no part is specified attempt
        to apply pose to all parts"""
        if (partName==None):
            # pose all parts
            for partName in self.__animControlDict.keys():
                animControl = self.__getAnimControl(animName, partName)
                if (animControl != None):
                    animControl.pose(frame)
        else:
            # pose a specific part
            animControl = self.__getAnimControl(animName, partName)
            if (animControl != None):
                animControl.pose(frame)
        

    #private
    
    def __getAnimControl(self, animName, partName):
        """__getAnimControl(self, string, string)
        Search the animControl dictionary for given anim and part.
        Return the animControl if present, or None otherwise"""
        if (self.__animControlDict.has_key(partName)):
            if (self.__animControlDict[partName].has_key(animName)):
                return self.__animControlDict[partName][animName]
            else:
                # anim was not present
                Actor.notify.warning("couldn't find anim: %s" % (animName))
        else:
            # part was not present
            Actor.notify.warning("couldn't find part: %s" % (partName))
                
        return None

            
    def loadModel(self, modelPath, partName="modelRoot"):
        """loadModel(self, string, string="modelRoot")
        Actor model loader. Takes a model name (ie file path) and
        a partName (defaults to "modelRoot")"""
        
        Actor.notify.info("in loadModel: %s , part: %s" % \
            (modelPath, partName))

        # load the model and extract its part bundle
        model = loader.loadModelCopy(modelPath)


        bundle = NodePath(model, "**/+PartBundleNode")
        if (bundle.isEmpty()):
            Actor.notify.warning("%s is not a character!" % (modelPath))
        else:
            # we rename this node to make Actor copying easier
            bundle.node().setName(Actor.partPrefix + partName)
            bundle.reparentTo(self.__geomNode)
            model.removeNode()        

        #make this mimic mutli-part by giving it a default part anme
        self.__partBundleDict[partName] = bundle
        

    def loadAnims(self, anims, partName="modelRoot"):
        """loadAnims(self, string:string{}, string="modelRoot")
        Actor anim loader. Takes an optional partName (defaults to
        'modelRoot' for non-multipart actors) and dict of corresponding
        anims in the form animName:animPath{}"""
        
        Actor.notify.info("in loadAnims: %s, part: %s" % (anims, partName))

        animDict = {}

        for animName in anims.keys():
            
            #load the anim and get its anim bundle
            anim = loader.loadModelCopy(anims[animName])
            animBundle = \
                (NodePath(anim, "**/+AnimBundleNode").node()).getBundle()

            #bind anim
            bundleNode = (self.__partBundleDict[partName]).node()
            animControl = (bundleNode.getBundle()).bindAnim(animBundle, -1)
            if (animControl == None):
                Actor.notify.error("Null AnimControl: %s" % (animName))
            else:
                animDict[animName] = animControl
            
        # add this part's dictionary to animation dictionary
        self.__animControlDict[partName] = animDict
    

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
                
