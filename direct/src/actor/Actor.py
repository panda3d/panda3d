"""Actor module: contains the Actor class"""

__all__ = ['Actor']

from panda3d.core import *
from panda3d.core import Loader as PandaLoader
from direct.showbase.DirectObject import DirectObject
from direct.showbase.Loader import Loader
from direct.directnotify import DirectNotifyGlobal


class Actor(DirectObject, NodePath):
    """
    Actor class: Contains methods for creating, manipulating
    and playing animations on characters
    """
    notify = DirectNotifyGlobal.directNotify.newCategory("Actor")
    partPrefix = "__Actor_"

    modelLoaderOptions = LoaderOptions(LoaderOptions.LFSearch |
                                       LoaderOptions.LFReportErrors |
                                       LoaderOptions.LFConvertSkeleton)
    animLoaderOptions =  LoaderOptions(LoaderOptions.LFSearch |
                                       LoaderOptions.LFReportErrors |
                                       LoaderOptions.LFConvertAnim)

    validateSubparts = ConfigVariableBool('validate-subparts', True)
    mergeLODBundles = ConfigVariableBool('merge-lod-bundles', True)
    allowAsyncBind = ConfigVariableBool('allow-async-bind', True)

    class PartDef:

        """Instances of this class are stored within the
        PartBundleDict to track all of the individual PartBundles
        associated with the Actor.  In general, each separately loaded
        model file is a different PartBundle.  This can include the
        multiple different LOD's, as well as the multiple different
        pieces of a multipart Actor. """

        def __init__(self, partBundleNP, partBundleHandle, partModel):
            # We also save the ModelRoot node along with the
            # PartBundle, so that the reference count in the ModelPool
            # will be accurate.
            self.partBundleNP = partBundleNP
            self.partBundleHandle = partBundleHandle
            self.partModel = partModel

        def getBundle(self):
            return self.partBundleHandle.getBundle()

        def __repr__(self):
            return 'Actor.PartDef(%s, %s)' % (repr(self.partBundleNP), repr(self.partModel))


        #snake_case alias:
        get_bundle = getBundle

    class AnimDef:

        """Instances of this class are stored within the
        AnimControlDict to track all of the animations associated with
        the Actor.  This includes animations that have already been
        bound (these have a valid AnimControl) as well as those that
        have not yet been bound (for these, self.animControl is None).

        There is a different AnimDef for each different part or
        sub-part, times each different animation in the AnimDict. """

        def __init__(self, filename = None, animBundle = None):
            self.filename = filename
            self.animBundle = animBundle
            self.animControl = None

        def makeCopy(self):
            return Actor.AnimDef(self.filename, self.animBundle)

        def __repr__(self):
            return 'Actor.AnimDef(%s)' % (repr(self.filename))


        #snake_case alias:
        make_copy = makeCopy

    class SubpartDef:

        """Instances of this class are stored within the SubpartDict
        to track the existance of arbitrary sub-parts.  These are
        designed to appear to the user to be identical to true "part"
        of a multi-part Actor, but in fact each subpart represents a
        subset of the joints of an existing part (which is accessible
        via a different name). """

        def __init__(self, truePartName, subset = PartSubset()):
            self.truePartName = truePartName
            self.subset = subset

        def makeCopy(self):
            return Actor.SubpartDef(self.truePartName, PartSubset(self.subset))


        def __repr__(self):
            return 'Actor.SubpartDef(%s, %s)' % (repr(self.truePartName), repr(self.subset))

    def __init__(self, models=None, anims=None, other=None, copy=True,
                 lodNode = None, flattenable = True, setFinal = False,
                 mergeLODBundles = None, allowAsyncBind = None,
                 okMissing = None):
        """Actor constructor: can be used to create single or multipart
        actors. If another Actor is supplied as an argument this
        method acts like a copy constructor. Single part actors are
        created by calling with a model and animation dictionary
        ``(animName:animPath{})`` as follows::

           a = Actor("panda-3k.egg", {"walk":"panda-walk.egg",
                                      "run":"panda-run.egg"})

        This could be displayed and animated as such::

           a.reparentTo(render)
           a.loop("walk")
           a.stop()

        Multipart actors expect a dictionary of parts and a dictionary
        of animation dictionaries ``(partName:(animName:animPath{}){})``
        as below::

            a = Actor(

                # part dictionary
                {"head": "char/dogMM/dogMM_Shorts-head-mod",
                 "torso": "char/dogMM/dogMM_Shorts-torso-mod",
                 "legs": "char/dogMM/dogMM_Shorts-legs-mod"},

                # dictionary of anim dictionaries
                {"head":{"walk": "char/dogMM/dogMM_Shorts-head-walk",
                         "run": "char/dogMM/dogMM_Shorts-head-run"},
                 "torso":{"walk": "char/dogMM/dogMM_Shorts-torso-walk",
                          "run": "char/dogMM/dogMM_Shorts-torso-run"},
                 "legs":{"walk": "char/dogMM/dogMM_Shorts-legs-walk",
                         "run": "char/dogMM/dogMM_Shorts-legs-run"}
                 })

        In addition multipart actor parts need to be connected together
        in a meaningful fashion::

            a.attach("head", "torso", "joint-head")
            a.attach("torso", "legs", "joint-hips")

        #
        # ADD LOD COMMENT HERE!
        #

        Other useful Actor class functions::

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

        self.loader = PandaLoader.getGlobalPtr()

        # Set the mergeLODBundles flag.  If this is true, all
        # different LOD's will be merged into a single common bundle
        # (joint hierarchy).  All LOD's will thereafter share the same
        # skeleton, even though they may have been loaded from
        # different egg files.  If this is false, LOD's will be kept
        # completely isolated, and each LOD will have its own
        # skeleton.

        # When this flag is true, __animControlDict has only one key,
        # ['common']; when it is false, __animControlDict has one key
        # per each LOD name.

        if mergeLODBundles is None:
            # If this isn't specified, it comes from the Config.prc
            # file.
            self.mergeLODBundles = Actor.mergeLODBundles.getValue()
        else:
            self.mergeLODBundles = mergeLODBundles

        # Set the allowAsyncBind flag.  If this is true, it enables
        # asynchronous animation binding.  This requires that you have
        # run "egg-optchar -preload" on your animation and models to
        # generate the appropriate AnimPreloadTable.
        if allowAsyncBind is None:
            self.allowAsyncBind = Actor.allowAsyncBind.getValue()
        else:
            self.allowAsyncBind = allowAsyncBind

        # create data structures
        self.__commonBundleHandles = {}
        self.__partBundleDict = {}
        self.__subpartDict = {}
        self.__sortedLODNames = []
        self.__animControlDict = {}

        self.__subpartsComplete = False

        self.__LODNode = None
        self.__LODAnimation = None
        self.__LODCenter = Point3(0, 0, 0)
        self.switches = None

        if (other == None):
            # act like a normal constructor

            # create base hierarchy
            self.gotName = 0

            if flattenable:
                # If we want a flattenable Actor, don't create all
                # those ModelNodes, and the GeomNode is the same as
                # the root.
                root = PandaNode('actor')
                self.assign(NodePath(root))
                self.setGeomNode(NodePath(self))

            else:
                # A standard Actor has a ModelNode at the root, and
                # another ModelNode to protect the GeomNode.
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
            if models:
                # do we have a dictionary of models?
                if type(models) == dict:
                    # if this is a dictionary of dictionaries
                    if type(models[next(iter(models))]) == dict:
                        # then it must be a multipart actor w/LOD
                        self.setLODNode(node = lodNode)
                        # preserve numerical order for lod's
                        # this will make it easier to set ranges
                        sortedKeys = list(models.keys())
                        sortedKeys.sort()
                        for lodName in sortedKeys:
                            # make a node under the LOD switch
                            # for each lod (just because!)
                            self.addLOD(str(lodName))
                            # iterate over both dicts
                            for modelName in models[lodName]:
                                self.loadModel(models[lodName][modelName],
                                               modelName, lodName, copy = copy,
                                               okMissing = okMissing)
                    # then if there is a dictionary of dictionaries of anims
                    elif type(anims[next(iter(anims))]) == dict:
                        # then this is a multipart actor w/o LOD
                        for partName in models:
                            # pass in each part
                            self.loadModel(models[partName], partName,
                                           copy = copy, okMissing = okMissing)
                    else:
                        # it is a single part actor w/LOD
                        self.setLODNode(node = lodNode)
                        # preserve order of LOD's
                        sortedKeys = list(models.keys())
                        sortedKeys.sort()
                        for lodName in sortedKeys:
                            self.addLOD(str(lodName))
                            # pass in dictionary of parts
                            self.loadModel(models[lodName], lodName=lodName,
                                           copy = copy, okMissing = okMissing)
                else:
                    # else it is a single part actor
                    self.loadModel(models, copy = copy, okMissing = okMissing)

            # load anims
            # make sure the actor has animations
            if anims:
                if len(anims) >= 1:
                    # if so, does it have a dictionary of dictionaries?
                    if type(anims[next(iter(anims))]) == dict:
                        # are the models a dict of dicts too?
                        if type(models) == dict:
                            if type(models[next(iter(models))]) == dict:
                                # then we have a multi-part w/ LOD
                                sortedKeys = list(models.keys())
                                sortedKeys.sort()
                                for lodName in sortedKeys:
                                    # iterate over both dicts
                                    for partName in anims:
                                        self.loadAnims(
                                            anims[partName], partName, lodName)
                            else:
                                # then it must be multi-part w/o LOD
                                for partName in anims:
                                    self.loadAnims(anims[partName], partName)
                    elif type(models) == dict:
                        # then we have single-part w/ LOD
                        sortedKeys = list(models.keys())
                        sortedKeys.sort()
                        for lodName in sortedKeys:
                            self.loadAnims(anims, lodName=lodName)
                    else:
                        # else it is single-part w/o LOD
                        self.loadAnims(anims)

        else:
            self.copyActor(other, True) # overwrite everything

        if setFinal:
            # If setFinal is true, the Actor will set its top bounding
            # volume to be the "final" bounding volume: the bounding
            # volumes below the top volume will not be tested.  If a
            # cull test passes the top bounding volume, the whole
            # Actor is rendered.

            # We do this partly because an Actor is likely to be a
            # fairly small object relative to the scene, and is pretty
            # much going to be all onscreen or all offscreen anyway;
            # and partly because of the Character bug that doesn't
            # update the bounding volume for pieces that animate away
            # from their original position.  It's disturbing to see
            # someone's hands disappear; better to cull the whole
            # object or none of it.
            self.__geomNode.node().setFinal(1)

    def delete(self):
        try:
            self.Actor_deleted
            return
        except:
            self.Actor_deleted = 1
            self.cleanup()

    def copyActor(self, other, overwrite=False):
            # act like a copy constructor
            self.gotName = other.gotName

            # copy the scene graph elements of other
            if (overwrite):
                otherCopy = other.copyTo(NodePath())
                otherCopy.detachNode()
                # assign these elements to ourselve (overwrite)
                self.assign(otherCopy)
            else:
                # just copy these to ourselves
                otherCopy = other.copyTo(self)
            # masad: check if otherCopy has a geomNode as its first child
            # if actor is initialized with flattenable, then otherCopy, not
            # its first child, is the geom node; check __init__, for reference
            if other.getGeomNode().getName() == other.getName():
                self.setGeomNode(otherCopy)
            else:
                self.setGeomNode(otherCopy.getChild(0))

            # copy the switches for lods
            self.switches = other.switches
            self.__LODNode = self.find('**/+LODNode')
            self.__hasLOD = 0
            if (not self.__LODNode.isEmpty()):
                self.__hasLOD = 1


            # copy the part dictionary from other
            self.__copyPartBundles(other)
            self.__copySubpartDict(other)
            self.__subpartsComplete = other.__subpartsComplete

            # copy the anim dictionary from other
            self.__copyAnimControls(other)


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
        return "Actor %s, parts = %s, LODs = %s, anims = %s" % \
               (self.getName(), self.getPartNames(), self.getLODNames(), self.getAnimNames())

    def listJoints(self, partName="modelRoot", lodName="lodRoot"):
        """Handy utility function to list the joint hierarchy of the
        actor. """

        if self.mergeLODBundles:
            partBundleDict = self.__commonBundleHandles
        else:
            partBundleDict = self.__partBundleDict.get(lodName)
            if not partBundleDict:
                Actor.notify.error("no lod named: %s" % (lodName))

        subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))

        partDef = partBundleDict.get(subpartDef.truePartName)
        if partDef == None:
            Actor.notify.error("no part named: %s" % (partName))

        self.__doListJoints(0, partDef.getBundle(),
                            subpartDef.subset.isIncludeEmpty(), subpartDef.subset)

    def __doListJoints(self, indentLevel, part, isIncluded, subset):
        name = part.getName()
        if subset.matchesInclude(name):
            isIncluded = True
        elif subset.matchesExclude(name):
            isIncluded = False

        if isIncluded:
            value = ''
            if hasattr(part, 'outputValue'):
                lineStream = LineStream()
                part.outputValue(lineStream)
                value = lineStream.getLine()

            print(' '.join((' ' * indentLevel, part.getName(), value)))

        for child in part.getChildren():
            self.__doListJoints(indentLevel + 2, child, isIncluded, subset)


    def getActorInfo(self):
        """
        Utility function to create a list of information about an actor.
        Useful for iterating over details of an actor.
        """
        lodInfo = []
        for lodName, partDict in self.__animControlDict.items():
            if self.mergeLODBundles:
                lodName = self.__sortedLODNames[0]

            partInfo = []
            for partName in partDict:
                subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))
                partBundleDict = self.__partBundleDict.get(lodName)
                partDef = partBundleDict.get(subpartDef.truePartName)
                partBundle = partDef.getBundle()
                animDict = partDict[partName]
                animInfo = []
                for animName in animDict:
                    file = animDict[animName].filename
                    animControl = animDict[animName].animControl
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
            print('LOD: %s' % lodName)
            for partName, bundle, animInfo in lodInfo:
                print('  Part: %s' % partName)
                print('  Bundle: %r' % bundle)
                for animName, file, animControl in animInfo:
                    print('    Anim: %s' % animName)
                    print('      File: %s' % file)
                    if animControl == None:
                        print(' (not loaded)')
                    else:
                        print('      NumFrames: %d PlayRate: %0.2f' %
                               (animControl.getNumFrames(),
                                animControl.getPlayRate()))

    def cleanup(self):
        """
        Actor cleanup function
        """
        self.stop(None)
        self.clearPythonData()
        self.flush()
        if(self.__geomNode):
            self.__geomNode.removeNode()
            self.__geomNode = None
        if not self.isEmpty():
            self.removeNode()

    def removeNode(self):
        if self.__geomNode and (self.__geomNode.getNumChildren() > 0):
            assert self.notify.warning("called actor.removeNode() on %s without calling cleanup()" % self.getName())
        NodePath.removeNode(self)

    def clearPythonData(self):
        self.__commonBundleHandles = {}
        self.__partBundleDict = {}
        self.__subpartDict = {}
        self.__sortedLODNames = []
        self.__animControlDict = {}

    def flush(self):
        """
        Actor flush function
        """
        self.clearPythonData()

        if self.__LODNode and (not self.__LODNode.isEmpty()):
            self.__LODNode.removeNode()
            self.__LODNode = None

        # remove all its children
        if self.__geomNode:
            self.__geomNode.getChildren().detach()

        self.__hasLOD = 0

    # accessing

    def getAnimControlDict(self):
        return self.__animControlDict

    def removeAnimControlDict(self):
        self.__animControlDict = {}

    def getPartBundleDict(self):
        return self.__partBundleDict

    def getPartBundles(self, partName = None):
        """ Returns a list of PartBundle objects for the entire Actor,
        or for the indicated part only. """

        bundles = []

        for lodName, partBundleDict in self.__partBundleDict.items():
            if partName == None:
                for partDef in partBundleDict.values():
                    bundles.append(partDef.getBundle())

            else:
                subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))
                partDef = partBundleDict.get(subpartDef.truePartName)
                if partDef != None:
                    bundles.append(partDef.getBundle())
                else:
                    Actor.notify.warning("Couldn't find part: %s" % (partName))

        return bundles

    def __updateSortedLODNames(self):
        # Cache the sorted LOD names so we don't have to grab them
        # and sort them every time somebody asks for the list
        self.__sortedLODNames = list(self.__partBundleDict.keys())
        # Reverse sort the doing a string->int
        def sortKey(x):
            if not str(x).isdigit():
                smap = {'h':3,
                        'm':2,
                        'l':1,
                        'f':0}

                """
                sx = smap.get(x[0], None)

                if sx is None:
                    self.notify.error('Invalid lodName: %s' % x)
                """
                return smap[x[0]]
            else:
                return int(x)

        self.__sortedLODNames.sort(key=sortKey, reverse=True)

    def getLODNames(self):
        """
        Return list of Actor LOD names. If not an LOD actor,
        returns 'lodRoot'
        Caution - this returns a reference to the list - not your own copy
        """
        return self.__sortedLODNames

    def getPartNames(self):
        """
        Return list of Actor part names. If not an multipart actor,
        returns 'modelRoot' NOTE: returns parts of arbitrary LOD
        """
        partNames = []
        if self.__partBundleDict:
            partNames = list(next(iter(self.__partBundleDict.values())).keys())
        return partNames + list(self.__subpartDict.keys())

    def getGeomNode(self):
        """
        Return the node that contains all actor geometry
        """
        return self.__geomNode

    def setGeomNode(self, node):
        """
        Set the node that contains all actor geometry
        """
        self.__geomNode = node

    def getLODNode(self):
        """
        Return the node that switches actor geometry in and out"""
        return self.__LODNode.node()

    def setLODNode(self, node=None):
        """
        Set the node that switches actor geometry in and out.
        If one is not supplied as an argument, make one
        """
        if (node == None):
            node = LODNode.makeDefaultLod("lod")

        if self.__LODNode:
            self.__LODNode = node
        else:
            self.__LODNode = self.__geomNode.attachNewNode(node)
            self.__hasLOD = 1
            self.switches = {}


    def useLOD(self, lodName):
        """
        Make the Actor ONLY display the given LOD
        """
        # make sure we don't call this twice in a row
        # and pollute the the switches dictionary
        child = self.__LODNode.find(str(lodName))
        index = self.__LODNode.node().findChild(child.node())
        self.__LODNode.node().forceSwitch(index)

    def printLOD(self):
        sortedKeys = self.__sortedLODNames
        for eachLod in sortedKeys:
            print("python switches for %s: in: %d, out %d" % (eachLod,
                                              self.switches[eachLod][0],
                                              self.switches[eachLod][1]))

        switchNum = self.__LODNode.node().getNumSwitches()
        for eachSwitch in range(0, switchNum):
            print("c++ switches for %d: in: %d, out: %d" % (eachSwitch,
                   self.__LODNode.node().getIn(eachSwitch),
                   self.__LODNode.node().getOut(eachSwitch)))


    def resetLOD(self):
        """
        Restore all switch distance info (usually after a useLOD call)"""
        self.__LODNode.node().clearForceSwitch()

    def addLOD(self, lodName, inDist=0, outDist=0, center=None):
        """addLOD(self, string)
        Add a named node under the LODNode to parent all geometry
        of a specific LOD under.
        """
        self.__LODNode.attachNewNode(str(lodName))
        # save the switch distance info
        self.switches[lodName] = [inDist, outDist]
        # add the switch distance info
        self.__LODNode.node().addSwitch(inDist, outDist)
        if center != None:
            self.setCenter(center)

    def setLOD(self, lodName, inDist=0, outDist=0):
        """setLOD(self, string)
        Set the switch distance for given LOD
        """
        # save the switch distance info
        self.switches[lodName] = [inDist, outDist]
        # add the switch distance info
        self.__LODNode.node().setSwitch(self.getLODIndex(lodName), inDist, outDist)

    def getLODIndex(self, lodName):
        """getLODIndex(self)
        safe method (but expensive) for retrieving the child index
        """
        return list(self.__LODNode.getChildren()).index(self.getLOD(lodName))

    def getLOD(self, lodName):
        """getLOD(self, string)
        Get the named node under the LOD to which we parent all LOD
        specific geometry to. Returns 'None' if not found
        """
        if self.__LODNode:
            lod = self.__LODNode.find(str(lodName))
            if lod.isEmpty():
                return None
            else:
                return lod
        else:
            return None

    def hasLOD(self):
        """
        Return 1 if the actor has LODs, 0 otherwise
        """
        return self.__hasLOD

    def setCenter(self, center):
        if center == None:
            center = Point3(0, 0, 0)
        self.__LODCenter = center
        if self.__LODNode:
            self.__LODNode.node().setCenter(self.__LODCenter)
        if self.__LODAnimation:
            self.setLODAnimation(*self.__LODAnimation)

    def setLODAnimation(self, farDistance, nearDistance, delayFactor):
        """ Activates a special mode in which the Actor animates less
        frequently as it gets further from the camera.  This is
        intended as a simple optimization to minimize the effort of
        computing animation for lots of characters that may not
        necessarily be very important to animate every frame.

        If the character is closer to the camera than near_distance,
        then it is animated its normal rate, every frame.  If the
        character is exactly far_distance away, it is animated only
        every delay_factor seconds (which should be a number greater
        than 0).  If the character is between near_distance and
        far_distance, its animation rate is linearly interpolated
        according to its distance between the two.  The interpolation
        function continues beyond far_distance, so that the character
        is animated increasingly less frequently as it gets farther
        away. """

        self.__LODAnimation = (farDistance, nearDistance, delayFactor)

        for lodData in self.__partBundleDict.values():
            for partData in lodData.values():
                char = partData.partBundleNP
                char.node().setLodAnimation(self.__LODCenter, farDistance, nearDistance, delayFactor)

    def clearLODAnimation(self):
        """ Description: Undoes the effect of a recent call to
        set_lod_animation().  Henceforth, the character will animate
        every frame, regardless of its distance from the camera.
        """

        self.__LODAnimation = None

        for lodData in self.__partBundleDict.values():
            for partData in lodData.values():
                char = partData.partBundleNP
                char.node().clearLodAnimation()


    def update(self, lod=0, partName=None, lodName=None, force=False):
        """ Updates all of the Actor's joints in the indicated LOD.
        The LOD may be specified by name, or by number, where 0 is the
        highest level of detail, 1 is the next highest, and so on.

        If force is True, this will update every joint, even if we
        don't believe it's necessary.

        Returns True if any joint has changed as a result of this,
        False otherwise. """

        if lodName == None:
            lodNames = self.getLODNames()
        else:
            lodNames = [lodName]

        anyChanged = False
        if lod < len(lodNames):
            lodName = lodNames[lod]
            if partName == None:
                partBundleDict = self.__partBundleDict[lodName]
                partNames = list(partBundleDict.keys())
            else:
                partNames = [partName]

            for partName in partNames:
                partBundle = self.getPartBundle(partName, lodNames[lod])
                if force:
                    if partBundle.forceUpdate():
                        anyChanged = True
                else:
                    if partBundle.update():
                        anyChanged = True
        else:
            self.notify.warning('update() - no lod: %d' % lod)

        return anyChanged

    def getFrameRate(self, animName=None, partName=None):
        """getFrameRate(self, string, string=None)
        Return actual frame rate of given anim name and given part.
        If no anim specified, use the currently playing anim.
        If no part specified, return anim durations of first part.
        NOTE: returns info only for an arbitrary LOD
        """
        lodName = next(iter(self.__animControlDict))
        controls = self.getAnimControls(animName, partName)
        if len(controls) == 0:
            return None

        return controls[0].getFrameRate()

    def getBaseFrameRate(self, animName=None, partName=None):
        """getBaseFrameRate(self, string, string=None)
        Return frame rate of given anim name and given part, unmodified
        by any play rate in effect.
        """
        lodName = next(iter(self.__animControlDict))
        controls = self.getAnimControls(animName, partName)
        if len(controls) == 0:
            return None

        return controls[0].getAnim().getBaseFrameRate()

    def getPlayRate(self, animName=None, partName=None):
        """
        Return the play rate of given anim for a given part.
        If no part is given, assume first part in dictionary.
        If no anim is given, find the current anim for the part.
        NOTE: Returns info only for an arbitrary LOD
        """
        if self.__animControlDict:
            # use the first lod
            lodName = next(iter(self.__animControlDict))
            controls = self.getAnimControls(animName, partName)
            if controls:
                return controls[0].getPlayRate()
        return None

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

    def getDuration(self, animName=None, partName=None,
                    fromFrame=None, toFrame=None):
        """
        Return duration of given anim name and given part.
        If no anim specified, use the currently playing anim.
        If no part specified, return anim duration of first part.
        NOTE: returns info for arbitrary LOD
        """
        lodName = next(iter(self.__animControlDict))
        controls = self.getAnimControls(animName, partName)
        if len(controls) == 0:
            return None

        animControl = controls[0]
        if fromFrame is None:
            fromFrame = 0
        if toFrame is None:
            toFrame = animControl.getNumFrames()-1
        return ((toFrame+1)-fromFrame) / animControl.getFrameRate()

    def getNumFrames(self, animName=None, partName=None):
        #lodName = next(iter(self.__animControlDict))
        controls = self.getAnimControls(animName, partName)
        if len(controls) == 0:
            return None
        return controls[0].getNumFrames()

    def getFrameTime(self, anim, frame, partName=None):
        numFrames = self.getNumFrames(anim,partName)
        animTime = self.getDuration(anim,partName)
        frameTime = animTime * float(frame) / numFrames
        return frameTime

    def getCurrentAnim(self, partName=None):
        """
        Return the anim currently playing on the actor. If part not
        specified return current anim of an arbitrary part in dictionary.
        NOTE: only returns info for an arbitrary LOD
        """
        if len(self.__animControlDict) == 0:
            return

        lodName, animControlDict = next(iter(self.__animControlDict.items()))
        if partName == None:
            partName, animDict = next(iter(animControlDict.items()))
        else:
            animDict = animControlDict.get(partName)
            if animDict == None:
                # part was not present
                Actor.notify.warning("couldn't find part: %s" % (partName))
                return None

        # loop through all anims for named part and find if any are playing
        for animName, anim in animDict.items():
            if anim.animControl and anim.animControl.isPlaying():
                return animName

        # we must have found none, or gotten an error
        return None

    def getCurrentFrame(self, animName=None, partName=None):
        """
        Return the current frame number of the named anim, or if no
        anim is specified, then the anim current playing on the
        actor. If part not specified return current anim of first part
        in dictionary.  NOTE: only returns info for an arbitrary LOD
        """
        lodName, animControlDict = next(iter(self.__animControlDict.items()))
        if partName == None:
            partName, animDict = next(iter(animControlDict.items()))
        else:
            animDict = animControlDict.get(partName)
            if animDict == None:
                # part was not present
                Actor.notify.warning("couldn't find part: %s" % (partName))
                return None

        if animName:
            anim = animDict.get(animName)
            if not anim:
                Actor.notify.warning("couldn't find anim: %s" % (animName))
            elif anim.animControl:
                return anim.animControl.getFrame()
        else:
            # loop through all anims for named part and find if any are playing
            for animName, anim in animDict.items():
                if anim.animControl and anim.animControl.isPlaying():
                    return anim.animControl.getFrame()

        # we must have found none, or gotten an error
        return None


    # arranging

    def getPart(self, partName, lodName="lodRoot"):
        """
        Find the named part in the optional named lod and return it, or
        return None if not present
        """
        partBundleDict = self.__partBundleDict.get(lodName)
        if not partBundleDict:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None
        subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))
        partDef = partBundleDict.get(subpartDef.truePartName)
        if partDef != None:
            return partDef.partBundleNP
        return None

    def getPartBundle(self, partName, lodName="lodRoot"):
        """
        Find the named part in the optional named lod and return its
        associated PartBundle, or return None if not present
        """
        partBundleDict = self.__partBundleDict.get(lodName)
        if not partBundleDict:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None
        subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))
        partDef = partBundleDict.get(subpartDef.truePartName)
        if partDef != None:
            return partDef.getBundle()
        return None

    def removePart(self, partName, lodName="lodRoot"):
        """
        Remove the geometry and animations of the named part of the
        optional named lod if present.
        NOTE: this will remove child geometry also!
        """
        # find the corresponding part bundle dict
        partBundleDict = self.__partBundleDict.get(lodName)
        if not partBundleDict:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return

        # remove the part
        if (partName in partBundleDict):
            partBundleDict[partName].partBundleNP.removeNode()
            del(partBundleDict[partName])

        # find the corresponding anim control dict
        if self.mergeLODBundles:
            lodName = 'common'
        partDict = self.__animControlDict.get(lodName)
        if not partDict:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return

        # remove the animations
        if (partName in partDict):
            del(partDict[partName])

    def hidePart(self, partName, lodName="lodRoot"):
        """
        Make the given part of the optionally given lod not render,
        even though still in the tree.
        NOTE: this will affect child geometry
        """
        partBundleDict = self.__partBundleDict.get(lodName)
        if not partBundleDict:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return
        partDef = partBundleDict.get(partName)
        if partDef:
            partDef.partBundleNP.hide()
        else:
            Actor.notify.warning("no part named %s!" % (partName))

    def showPart(self, partName, lodName="lodRoot"):
        """
        Make the given part render while in the tree.
        NOTE: this will affect child geometry
        """
        partBundleDict = self.__partBundleDict.get(lodName)
        if not partBundleDict:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return
        partDef = partBundleDict.get(partName)
        if partDef:
            partDef.partBundleNP.show()
        else:
            Actor.notify.warning("no part named %s!" % (partName))

    def showAllParts(self, partName, lodName="lodRoot"):
        """
        Make the given part and all its children render while in the tree.
        NOTE: this will affect child geometry
        """
        partBundleDict = self.__partBundleDict.get(lodName)
        if not partBundleDict:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return
        partDef = partBundleDict.get(partName)
        if partDef:
            partDef.partBundleNP.show()
            partDef.partBundleNP.getChildren().show()
        else:
            Actor.notify.warning("no part named %s!" % (partName))

    def exposeJoint(self, node, partName, jointName, lodName="lodRoot",
                    localTransform = 0):
        """exposeJoint(self, NodePath, string, string, key="lodRoot")
        Starts the joint animating the indicated node.  As the joint
        animates, it will transform the node by the corresponding
        amount.  This will replace whatever matrix is on the node each
        frame.  The default is to expose the net transform from the root,
        but if localTransform is true, only the node's local transform
        from its parent is exposed."""
        partBundleDict = self.__partBundleDict.get(lodName)
        if not partBundleDict:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None

        subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))

        partDef = partBundleDict.get(subpartDef.truePartName)
        if partDef:
            bundle = partDef.getBundle()
        else:
            Actor.notify.warning("no part named %s!" % (partName))
            return None

        # Get a handle to the joint.
        joint = bundle.findChild(jointName)

        if node is None:
            node = partDef.partBundleNP.attachNewNode(jointName)

        if (joint):
            if localTransform:
                joint.addLocalTransform(node.node())
            else:
                joint.addNetTransform(node.node())
        else:
            Actor.notify.warning("no joint named %s!" % (jointName))

        return node

    def stopJoint(self, partName, jointName, lodName="lodRoot"):
        """stopJoint(self, string, string, key="lodRoot")
        Stops the joint from animating external nodes.  If the joint
        is animating a transform on a node, this will permanently stop
        it.  However, this does not affect vertex animations."""
        partBundleDict = self.__partBundleDict.get(lodName)
        if not partBundleDict:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None

        subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))

        partDef = partBundleDict.get(subpartDef.truePartName)
        if partDef:
            bundle = partDef.getBundle()
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

    def getJoints(self, partName = None, jointName = '*', lodName = None):
        """ Returns the list of all joints, from the named part or
        from all parts, that match the indicated jointName.  The
        jointName may include pattern characters like \\*. """

        joints=[]
        pattern = GlobPattern(jointName)

        if lodName == None and self.mergeLODBundles:
            # Get the common bundle.
            partBundleDicts = [self.__commonBundleHandles]

        elif lodName == None:
            # Get all LOD's.
            partBundleDicts = self.__partBundleDict.values()
        else:
            # Get one LOD.
            partBundleDict = self.__partBundleDict.get(lodName)
            if not partBundleDict:
                Actor.notify.warning("couldn't find lod: %s" % (lodName))
                return []
            partBundleDicts = [partBundleDict]

        for partBundleDict in partBundleDicts:
            parts = []
            if partName:
                subpartDef = self.__subpartDict.get(partName, None)
                if not subpartDef:
                    # Whole part
                    subset = None
                    partDef = partBundleDict.get(partName)
                else:
                    # Sub-part
                    subset = subpartDef.subset
                    partDef = partBundleDict.get(subpartDef.truePartName)
                if not partDef:
                    Actor.notify.warning("no part named %s!" % (partName))
                    return []
                parts = [partDef]
            else:
                subset = None
                parts = partBundleDict.values()

            for partData in parts:
                partBundle = partData.getBundle()

                if not pattern.hasGlobCharacters() and not subset:
                    # The simple case.
                    joint = partBundle.findChild(jointName)
                    if joint:
                        joints.append(joint)
                else:
                    # The more complex case.
                    isIncluded = True
                    if subset:
                        isIncluded = subset.isIncludeEmpty()
                    self.__getPartJoints(joints, pattern, partBundle, subset, isIncluded)

        return joints

    def getOverlappingJoints(self, partNameA, partNameB, jointName = '*', lodName = None):
        """ Returns the set of joints, matching jointName, that are
        shared between partNameA and partNameB. """
        jointsA = set(self.getJoints(partName = partNameA, jointName = jointName, lodName = lodName))
        jointsB = set(self.getJoints(partName = partNameB, jointName = jointName, lodName = lodName))

        return jointsA & jointsB

    def __getPartJoints(self, joints, pattern, partNode, subset, isIncluded):
        """ Recursively walks the joint hierarchy to look for matching
        joint names, implementing getJoints(). """

        name = partNode.getName()
        if subset:
            # Constrain the traversal just to the named subset.
            if subset.matchesInclude(name):
                isIncluded = True
            elif subset.matchesExclude(name):
                isIncluded = False

        if isIncluded and pattern.matches(name) and isinstance(partNode, MovingPartBase):
            joints.append(partNode)

        for child in partNode.getChildren():
            self.__getPartJoints(joints, pattern, child, subset, isIncluded)

    def getJointTransform(self, partName, jointName, lodName='lodRoot'):
        partBundleDict=self.__partBundleDict.get(lodName)
        if not partBundleDict:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None

        subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))
        partDef = partBundleDict.get(subpartDef.truePartName)
        if partDef:
            bundle = partDef.getBundle()
        else:
            Actor.notify.warning("no part named %s!" % (partName))
            return None

        joint = bundle.findChild(jointName)
        if joint == None:
            Actor.notify.warning("no joint named %s!" % (jointName))
            return None
        return joint.getDefaultValue()

    def getJointTransformState(self, partName, jointName, lodName='lodRoot'):
        partBundleDict=self.__partBundleDict.get(lodName)
        if not partBundleDict:
            Actor.notify.warning("no lod named: %s" % (lodName))
            return None

        subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))
        partDef = partBundleDict.get(subpartDef.truePartName)
        if partDef:
            bundle = partDef.getBundle()
        else:
            Actor.notify.warning("no part named %s!" % (partName))
            return None

        joint = bundle.findChild(jointName)
        if joint == None:
            Actor.notify.warning("no joint named %s!" % (jointName))
            return None
        return joint.getTransformState()

    def controlJoint(self, node, partName, jointName, lodName="lodRoot"):
        """The converse of exposeJoint: this associates the joint with
        the indicated node, so that the joint transform will be copied
        from the node to the joint each frame.  This can be used for
        programmer animation of a particular joint at runtime.

        The parameter node should be the NodePath for the node whose
        transform will animate the joint.  If node is None, a new node
        will automatically be created and loaded with the joint's
        initial transform.  In either case, the node used will be
        returned.

        It used to be necessary to call this before any animations
        have been loaded and bound, but that is no longer so.
        """
        subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))
        trueName = subpartDef.truePartName
        anyGood = False
        for bundleDict in self.__partBundleDict.values():
            bundle = bundleDict[trueName].getBundle()
            if node == None:
                node = self.attachNewNode(ModelNode(jointName))
                joint = bundle.findChild(jointName)
                if joint and isinstance(joint, MovingPartMatrix):
                    node.setMat(joint.getDefaultValue())

            if bundle.controlJoint(jointName, node.node()):
                anyGood = True

        if not anyGood:
            self.notify.warning("Cannot control joint %s" % (jointName))

        return node

    def freezeJoint(self, partName, jointName, transform = None,
                    pos=Vec3(0,0,0), hpr=Vec3(0,0,0), scale=Vec3(1,1,1)):
        """Similar to controlJoint, but the transform assigned is
        static, and may not be animated at runtime (without another
        subsequent call to freezeJoint).  This is slightly more
        optimal than controlJoint() for cases in which the transform
        is not intended to be animated during the lifetime of the
        Actor. """
        if transform == None:
            transform = TransformState.makePosHprScale(pos, hpr, scale)

        subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))
        trueName = subpartDef.truePartName
        anyGood = False
        for bundleDict in self.__partBundleDict.values():
            if bundleDict[trueName].getBundle().freezeJoint(jointName, transform):
                anyGood = True

        if not anyGood:
            self.notify.warning("Cannot freeze joint %s" % (jointName))

    def releaseJoint(self, partName, jointName):
        """Undoes a previous call to controlJoint() or freezeJoint()
        and restores the named joint to its normal animation. """

        subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))
        trueName = subpartDef.truePartName
        for bundleDict in self.__partBundleDict.values():
            bundleDict[trueName].getBundle().releaseJoint(jointName)

    def instance(self, path, partName, jointName, lodName="lodRoot"):
        """instance(self, NodePath, string, string, key="lodRoot")
        Instance a nodePath to an actor part at a joint called jointName"""
        partBundleDict = self.__partBundleDict.get(lodName)
        if partBundleDict:
            subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))
            partDef = partBundleDict.get(subpartDef.truePartName)
            if partDef:
                joint = partDef.partBundleNP.find("**/" + jointName)
                if (joint.isEmpty()):
                    Actor.notify.warning("%s not found!" % (jointName))
                else:
                    return path.instanceTo(joint)
            else:
                Actor.notify.warning("no part named %s!" % (partName))
        else:
            Actor.notify.warning("no lod named %s!" % (lodName))

    def attach(self, partName, anotherPartName, jointName, lodName="lodRoot"):
        """attach(self, string, string, string, key="lodRoot")
        Attach one actor part to another at a joint called jointName"""
        partBundleDict = self.__partBundleDict.get(lodName)
        if partBundleDict:
            subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))
            partDef = partBundleDict.get(subpartDef.truePartName)
            if partDef:
                anotherPartDef = partBundleDict.get(anotherPartName)
                if anotherPartDef:
                    joint = anotherPartDef.partBundleNP.find("**/" + jointName)
                    if (joint.isEmpty()):
                        Actor.notify.warning("%s not found!" % (jointName))
                    else:
                        partDef.partBundleNP.reparentTo(joint)
                else:
                    Actor.notify.warning("no part named %s!" % (anotherPartName))
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

        If mode > 0, the frontPart geometry is placed in the 'fixed'
        bin, with the indicated drawing order.  This will cause it to
        be drawn after almost all other geometry.  In this case, the
        backPartName is actually unused.

        Takes an optional argument root as the start of the search for the
        given parts. Also takes optional lod name to refine search for the
        named parts. If root and lod are defined, we search for the given
        root under the given lod.
        """
        # check to see if we are working within an lod
        if lodName != None:
            # find the named lod node
            lodRoot = self.__LODNode.find(str(lodName))
            if root == None:
                # no need to look further
                root = lodRoot
            else:
                # look for root under lod
                root = lodRoot.find("**/" + root)
        else:
            # start search from self if no root and no lod given
            if root == None:
                root = self

        frontParts = root.findAllMatches("**/" + frontPartName)

        if mode > 0:
            # Use the 'fixed' bin instead of reordering the scene
            # graph.
            for part in frontParts:
                part.setBin('fixed', mode)
            return

        if mode == -2:
            # Turn off depth test/write on the frontParts.
            for part in frontParts:
                part.setDepthWrite(0)
                part.setDepthTest(0)

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


    def fixBounds(self, partName = None):
        if(partName == None):
            #iterate through everything
            for lodData in self.__partBundleDict.values():
                for partData in lodData.values():
                    char = partData.partBundleNP
                    char.node().update()
                    geomNodes = char.findAllMatches("**/+GeomNode")
                    for thisGeomNode in geomNodes:
                        for thisGeom in thisGeomNode.node().getGeoms():
                            thisGeom.markBoundsStale()
                        thisGeomNode.node().markInternalBoundsStale()
        else:
            #iterate through for a specific part
            for lodData in self.__partBundleDict.values():
                partData = lodData.get(partName)
                if(partData):
                    char = partData.partBundleNP
                    char.node().update()
                    geomNodes = char.findAllMatches("**/+GeomNode")
                    for thisGeomNode in geomNodes:
                        for thisGeom in thisGeomNode.node().getGeoms():
                            thisGeom.markBoundsStale()
                        thisGeomNode.node().markInternalBoundsStale()

    def fixBounds_old(self, part=None):
        """fixBounds(self, nodePath=None)
        Force recomputation of bounding spheres for all geoms
        in a given part. If no part specified, fix all geoms
        in this actor
        """
        # if no part name specified fix all parts
        if (part==None):
            part = self

        # update all characters first
        charNodes = part.findAllMatches("**/+Character")
        for charNode in charNodes:
            charNode.node().update()

        # for each geomNode, iterate through all geoms and force update
        # of bounding spheres by marking current bounds as stale
        geomNodes = part.findAllMatches("**/+GeomNode")
        for nodeNum, thisGeomNode in enumerate(geomNodes):
            for geomNum, thisGeom in enumerate(thisGeomNode.node().getGeoms()):
                thisGeom.markBoundsStale()
                assert Actor.notify.debug("fixing bounds for node %s, geom %s" % \
                                          (nodeNum, geomNum))
            thisGeomNode.node().markInternalBoundsStale()

    def showAllBounds(self):
        """
        Show the bounds of all actor geoms
        """
        geomNodes = self.__geomNode.findAllMatches("**/+GeomNode")

        for node in geomNodes:
            node.showBounds()

    def hideAllBounds(self):
        """
        Hide the bounds of all actor geoms
        """
        geomNodes = self.__geomNode.findAllMatches("**/+GeomNode")

        for node in geomNodes:
            node.hideBounds()


    # actions
    def animPanel(self):
        # Don't use a regular import, to prevent ModuleFinder from picking
        # it up as a dependency when building a .p3d package.
        import importlib
        AnimPanel = importlib.import_module('direct.tkpanels.AnimPanel')
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
                if toFrame == None:
                    control.play(fromFrame, control.getNumFrames() - 1)
                else:
                    control.play(fromFrame, toFrame)

    def loop(self, animName, restart=1, partName=None,
             fromFrame=None, toFrame=None):
        """loop(self, string, int=1, string=None)
        Loop the given animation on the given part of the actor,
        restarting at zero frame if requested. If no part name
        is given then try to loop on all parts. NOTE: loops on
        all LOD's
        """

        if fromFrame == None:
            for control in self.getAnimControls(animName, partName):
                control.loop(restart)
        else:
            for control in self.getAnimControls(animName, partName):
                if toFrame == None:
                    control.loop(restart, fromFrame, control.getNumFrames() - 1)
                else:
                    control.loop(restart, fromFrame, toFrame)

    def pingpong(self, animName, restart=1, partName=None,
                 fromFrame=None, toFrame=None):
        """pingpong(self, string, int=1, string=None)
        Loop the given animation on the given part of the actor,
        restarting at zero frame if requested. If no part name
        is given then try to loop on all parts. NOTE: loops on
        all LOD's"""
        if fromFrame == None:
            fromFrame = 0

        for control in self.getAnimControls(animName, partName):
            if toFrame == None:
                control.pingpong(restart, fromFrame, control.getNumFrames() - 1)
            else:
                control.pingpong(restart, fromFrame, toFrame)

    def pose(self, animName, frame, partName=None, lodName=None):
        """pose(self, string, int, string=None)
        Pose the actor in position found at given frame in the specified
        animation for the specified part. If no part is specified attempt
        to apply pose to all parts."""
        for control in self.getAnimControls(animName, partName, lodName):
            control.pose(frame)

    def setBlend(self, animBlend = None, frameBlend = None,
                 blendType = None, partName = None):
        """
        Changes the way the Actor handles blending of multiple
        different animations, and/or interpolation between consecutive
        frames.

        The animBlend and frameBlend parameters are boolean flags.
        You may set either or both to True or False.  If you do not
        specify them, they do not change from the previous value.

        When animBlend is True, multiple different animations may
        simultaneously be playing on the Actor.  This means you may
        call play(), loop(), or pose() on multiple animations and have
        all of them contribute to the final pose each frame.

        In this mode (that is, when animBlend is True), starting a
        particular animation with play(), loop(), or pose() does not
        implicitly make the animation visible; you must also call
        setControlEffect() for each animation you wish to use to
        indicate how much each animation contributes to the final
        pose.

        The frameBlend flag is unrelated to playing multiple
        animations.  It controls whether the Actor smoothly
        interpolates between consecutive frames of its animation (when
        the flag is True) or holds each frame until the next one is
        ready (when the flag is False).  The default value of
        frameBlend is controlled by the interpolate-frames Config.prc
        variable.

        In either case, you may also specify blendType, which controls
        the precise algorithm used to blend two or more different
        matrix values into a final result.  Different skeleton
        hierarchies may benefit from different algorithms.  The
        default blendType is controlled by the anim-blend-type
        Config.prc variable.
        """
        for bundle in self.getPartBundles(partName = partName):
            if blendType != None:
                bundle.setBlendType(blendType)
            if animBlend != None:
                bundle.setAnimBlendFlag(animBlend)
            if frameBlend != None:
                bundle.setFrameBlendFlag(frameBlend)

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

        This method is deprecated.  You should use setBlend() instead.
        """
        self.setBlend(animBlend = True, blendType = blendType, partName = partName)

    def disableBlend(self, partName = None):
        """
        Restores normal one-animation-at-a-time operation after a
        previous call to enableBlend().

        This method is deprecated.  You should use setBlend() instead.
        """
        self.setBlend(animBlend = False, partName = partName)

    def setControlEffect(self, animName, effect,
                         partName = None, lodName = None):
        """
        Sets the amount by which the named animation contributes to
        the overall pose.  This controls blending of multiple
        animations; it only makes sense to call this after a previous
        call to setBlend(animBlend = True).
        """
        for control in self.getAnimControls(animName, partName, lodName):
            control.getPart().setControlEffect(control, effect)

    def getAnimFilename(self, animName, partName='modelRoot'):
        """
        getAnimFilename(self, animName)
        return the animFilename given the animName
        """
        if self.mergeLODBundles:
            lodName = 'common'
        elif self.switches:
            lodName = str(next(iter(self.switches)))
        else:
            lodName = 'lodRoot'

        try:
            return self.__animControlDict[lodName][partName][animName].filename
        except:
            return None

    def getAnimControl(self, animName, partName=None, lodName=None,
                       allowAsyncBind = True):
        """
        getAnimControl(self, string, string, string="lodRoot")
        Search the animControl dictionary indicated by lodName for
        a given anim and part. If none specified, try the first part and lod.
        Return the animControl if present, or None otherwise.
        """

        if not partName:
            partName = 'modelRoot'

        if self.mergeLODBundles:
            lodName = 'common'
        elif not lodName:
            if self.switches:
                lodName = str(next(iter(self.switches)))
            else:
                lodName = 'lodRoot'

        partDict = self.__animControlDict.get(lodName)
        # if this assertion fails, named lod was not present
        assert partDict != None

        animDict = partDict.get(partName)
        if animDict == None:
            # part was not present
            Actor.notify.warning("couldn't find part: %s" % (partName))
        else:
            anim = animDict.get(animName)
            if anim == None:
                # anim was not present
                assert Actor.notify.debug("couldn't find anim: %s" % (animName))
                pass
            else:
                # bind the animation first if we need to
                if not anim.animControl:
                    self.__bindAnimToPart(animName, partName, lodName,
                                          allowAsyncBind = allowAsyncBind)
                elif not allowAsyncBind:
                    anim.animControl.waitPending()
                return anim.animControl

        return None

    def getAnimControls(self, animName=None, partName=None, lodName=None,
                        allowAsyncBind = True):
        """getAnimControls(self, string, string=None, string=None)

        Returns a list of the AnimControls that represent the given
        animation for the given part and the given lod.

        If animName is None or omitted, the currently-playing
        animation (or all currently-playing animations) is returned.
        If animName is True, all animations are returned.  If animName
        is a single string name, that particular animation is
        returned.  If animName is a list of string names, all of the
        names animations are returned.

        If partName is None or omitted, all parts are returned (or
        possibly the one overall Actor part, according to the
        subpartsComplete flag).

        If lodName is None or omitted, all LOD's are returned.
        """

        if partName == None and self.__subpartsComplete:
            # If we have the __subpartsComplete flag, and no partName
            # is specified, it really means to play the animation on
            # all subparts, not on the overall Actor.
            partName = list(self.__subpartDict.keys())

        controls = []
        # build list of lodNames and corresponding animControlDicts
        # requested.
        if lodName == None or self.mergeLODBundles:
            # Get all LOD's
            animControlDictItems = self.__animControlDict.items()
        else:
            partDict = self.__animControlDict.get(lodName)
            if partDict == None:
                Actor.notify.warning("couldn't find lod: %s" % (lodName))
                animControlDictItems = []
            else:
                animControlDictItems = [(lodName, partDict)]

        for lodName, partDict in animControlDictItems:
            # Now, build the list of partNames and the corresponding
            # animDicts.
            if partName == None:
                # Get all main parts, but not sub-parts.
                animDictItems = []
                for thisPart, animDict in partDict.items():
                    if thisPart not in self.__subpartDict:
                        animDictItems.append((thisPart, animDict))

            else:
                # Get exactly the named part or parts.
                if isinstance(partName, str):
                    partNameList = [partName]
                else:
                    partNameList = partName

                animDictItems = []

                for pName in partNameList:
                    animDict = partDict.get(pName)
                    if animDict == None:
                        # Maybe it's a subpart that hasn't been bound yet.
                        subpartDef = self.__subpartDict.get(pName)
                        if subpartDef:
                            animDict = {}
                            partDict[pName] = animDict

                    if animDict == None:
                        # part was not present
                        Actor.notify.warning("couldn't find part: %s" % (pName))
                    else:
                        animDictItems.append((pName, animDict))

            if animName is None:
                # get all playing animations
                for thisPart, animDict in animDictItems:
                    for anim in animDict.values():
                        if anim.animControl and anim.animControl.isPlaying():
                            controls.append(anim.animControl)
            else:
                # get the named animation(s) only.
                if isinstance(animName, str):
                    # A single animName
                    animNameList = [animName]
                else:
                    # A list of animNames, or True to indicate all anims.
                    animNameList = animName
                for thisPart, animDict in animDictItems:
                    names = animNameList
                    if animNameList is True:
                        names = animDict.keys()
                    for animName in names:
                        anim = animDict.get(animName)
                        if anim == None and partName != None:
                            for pName in partNameList:
                                # Maybe it's a subpart that hasn't been bound yet.
                                subpartDef = self.__subpartDict.get(pName)
                                if subpartDef:
                                    truePartName = subpartDef.truePartName
                                    anim = partDict[truePartName].get(animName)
                                    if anim:
                                        anim = anim.makeCopy()
                                        animDict[animName] = anim

                        if anim == None:
                            # anim was not present
                            assert Actor.notify.debug("couldn't find anim: %s" % (animName))
                            pass
                        else:
                            # bind the animation first if we need to
                            animControl = anim.animControl
                            if animControl == None:
                                animControl = self.__bindAnimToPart(
                                    animName, thisPart, lodName,
                                    allowAsyncBind = allowAsyncBind)
                            elif not allowAsyncBind:
                                # Force the animation to load if it's
                                # not already loaded.
                                animControl.waitPending()

                            if animControl:
                                controls.append(animControl)

        return controls

    def loadModel(self, modelPath, partName="modelRoot", lodName="lodRoot",
                  copy = True, okMissing = None, autoBindAnims = True):
        """Actor model loader. Takes a model name (ie file path), a part
        name(defaults to "modelRoot") and an lod name(defaults to "lodRoot").
        """
        assert partName not in self.__subpartDict

        assert Actor.notify.debug("in loadModel: %s, part: %s, lod: %s, copy: %s" % \
                                  (modelPath, partName, lodName, copy))

        if isinstance(modelPath, NodePath):
            # If we got a NodePath instead of a string, use *that* as
            # the model directly.
            if (copy):
                model = modelPath.copyTo(NodePath())
            else:
                model = modelPath
        else:
            # otherwise, we got the name of the model to load.
            loaderOptions = self.modelLoaderOptions
            if not copy:
                # If copy = 0, then we should always hit the disk.
                loaderOptions = LoaderOptions(loaderOptions)
                loaderOptions.setFlags(loaderOptions.getFlags() & ~LoaderOptions.LFNoRamCache)

            if okMissing is not None:
                if okMissing:
                    loaderOptions.setFlags(loaderOptions.getFlags() & ~LoaderOptions.LFReportErrors)
                else:
                    loaderOptions.setFlags(loaderOptions.getFlags() | LoaderOptions.LFReportErrors)

            # Ensure that custom Python loader hooks are initialized.
            Loader._loadPythonFileTypes()

            # Pass loaderOptions to specify that we want to
            # get the skeleton model.  This only matters to model
            # files (like .mb) for which we can choose to extract
            # either the skeleton or animation, or neither.
            model = self.loader.loadSync(Filename(modelPath), loaderOptions)
            if model is not None:
                model = NodePath(model)

        if (model == None):
            raise IOError("Could not load Actor model %s" % (modelPath))

        if (model.node().isOfType(Character.getClassType())):
            bundleNP = model
        else:
            bundleNP = model.find("**/+Character")

        if (bundleNP.isEmpty()):
            Actor.notify.warning("%s is not a character!" % (modelPath))
            model.reparentTo(self.__geomNode)
        else:
            # Maybe the model file also included some animations.  If
            # so, try to bind them immediately and put them into the
            # animControlDict.
            if autoBindAnims:
                acc = AnimControlCollection()
                autoBind(model.node(), acc, ~0)
                numAnims = acc.getNumAnims()
            else:
                numAnims = 0

            # Now extract out the Character and integrate it with
            # the Actor.

            if (lodName!="lodRoot"):
                # parent to appropriate node under LOD switch
                bundleNP.reparentTo(self.__LODNode.find(str(lodName)))
            else:
                bundleNP.reparentTo(self.__geomNode)
            self.__prepareBundle(bundleNP, model.node(), partName, lodName)

            # we rename this node to make Actor copying easier
            bundleNP.node().setName("%s%s"%(Actor.partPrefix,partName))

            if numAnims != 0:
                # If the model had some animations, store them in the
                # dict so they can be played.
                Actor.notify.info("model contains %s animations." % (numAnims))

                # make sure this lod is in anim control dict
                if self.mergeLODBundles:
                    lodName = 'common'
                self.__animControlDict.setdefault(lodName, {})
                self.__animControlDict[lodName].setdefault(partName, {})

                for i in range(numAnims):
                    animControl = acc.getAnim(i)
                    animName = acc.getAnimName(i)

                    animDef = Actor.AnimDef()
                    animDef.animBundle = animControl.getAnim()
                    animDef.animControl = animControl
                    self.__animControlDict[lodName][partName][animName] = animDef

    def __prepareBundle(self, bundleNP, partModel,
                        partName="modelRoot", lodName="lodRoot"):
        assert partName not in self.__subpartDict

        # Rename the node at the top of the hierarchy, if we
        # haven't already, to make it easier to identify this
        # actor in the scene graph.
        if not self.gotName:
            self.node().setName(bundleNP.node().getName())
            self.gotName = 1

        bundleDict = self.__partBundleDict.get(lodName, None)
        if bundleDict == None:
            # make a dictionary to store these parts in
            bundleDict = {}
            self.__partBundleDict[lodName] = bundleDict
            self.__updateSortedLODNames()

        node = bundleNP.node()
        # A model loaded from disk will always have just one bundle.
        assert(node.getNumBundles() == 1)
        bundleHandle = node.getBundleHandle(0)

        if self.mergeLODBundles:
            loadedBundleHandle = self.__commonBundleHandles.get(partName, None)
            if loadedBundleHandle:
                # We've already got a bundle for this part; merge it.
                node.mergeBundles(bundleHandle, loadedBundleHandle)
                bundleHandle = loadedBundleHandle
            else:
                # We haven't already got a bundle for this part; store it.
                self.__commonBundleHandles[partName] = bundleHandle

        bundleDict[partName] = Actor.PartDef(bundleNP, bundleHandle, partModel)


    def makeSubpart(self, partName, includeJoints, excludeJoints = [],
                    parent="modelRoot", overlapping = False):

        """Defines a new "part" of the Actor that corresponds to the
        same geometry as the named parent part, but animates only a
        certain subset of the joints.  This can be used for
        partial-body animations, for instance to animate a hand waving
        while the rest of the body continues to play its walking
        animation.

        includeJoints is a list of joint names that are to be animated
        by the subpart.  Each name can include globbing characters
        like '?' or '*', which will match one or any number of
        characters, respectively.  Including a joint by naming it in
        includeJoints implicitly includes all of the descendents of
        that joint as well, except for excludeJoints, below.

        excludeJoints is a list of joint names that are *not* to be
        animated by the subpart.  As in includeJoints, each name can
        include globbing characters.  If a joint is named by
        excludeJoints, it will not be included (and neither will any
        of its descendents), even if a parent joint was named by
        includeJoints.

        if overlapping is False, an error is raised (in the dev build)
        if this subpart shares joints with any other subparts.  If
        overlapping is True, no such error is raised.

        parent is the actual partName that this subpart is based
        on."""

        assert partName not in self.__subpartDict

        subpartDef = self.__subpartDict.get(parent, Actor.SubpartDef(''))

        subset = PartSubset(subpartDef.subset)
        for name in includeJoints:
            subset.addIncludeJoint(GlobPattern(name))
        for name in excludeJoints:
            subset.addExcludeJoint(GlobPattern(name))

        self.__subpartDict[partName] = Actor.SubpartDef(parent, subset)

        if __dev__ and not overlapping and self.validateSubparts.getValue():
            # Without the overlapping flag True, we're not allowed to
            # define overlapping sub-parts.  Verify that we haven't.
            for otherPartName, otherPartDef in self.__subpartDict.items():
                if otherPartName != partName and otherPartDef.truePartName == parent:
                    joints = self.getOverlappingJoints(partName, otherPartName)
                    if joints:
                        raise Exception('Overlapping joints: %s and %s' % (partName, otherPartName))

    def setSubpartsComplete(self, flag):

        """Sets the subpartsComplete flag.  This affects the behavior
        of play(), loop(), stop(), etc., when no explicit parts are
        specified.

        When this flag is False (the default), play() with no parts
        means to play the animation on the overall Actor, which is a
        separate part that overlaps each of the subparts.  If you then
        play a different animation on a subpart, it may stop the
        overall animation (in non-blend mode) or blend with it (in
        blend mode).

        When this flag is True, play() with no parts means to play the
        animation on each of the subparts--instead of on the overall
        Actor.  In this case, you may then play a different animation
        on a subpart, which replaces only that subpart's animation.

        It makes sense to set this True when the union of all of your
        subparts completely defines the entire Actor.
        """

        self.__subpartsComplete = flag

        if __dev__ and self.__subpartsComplete and self.validateSubparts.getValue():
            # If we've specified any parts at all so far, make sure we've
            # specified all of them.
            if self.__subpartDict:
                self.verifySubpartsComplete()


    def getSubpartsComplete(self):
        """See setSubpartsComplete()."""

        return self.__subpartsComplete

    def verifySubpartsComplete(self, partName = None, lodName = None):
        """ Ensures that each joint is defined by at least one
        subPart.  Prints a warning if this is not the case. """

        if partName:
            assert partName not in self.__subpartDict
            partNames = [partName]
        else:
            if lodName:
                partNames = self.__partBundleDict[lodName].keys()
            else:
                partNames = next(iter(self.__partBundleDict.values())).keys()

        for partName in partNames:
            subJoints = set()
            for subPartName, subPartDef in self.__subpartDict.items():
                if subPartName != partName and subPartDef.truePartName == partName:
                    subJoints |= set(self.getJoints(partName = subPartName, lodName = lodName))

            allJoints = set(self.getJoints(partName = partName, lodName = lodName))
            diff = allJoints.difference(subJoints)
            if diff:
                self.notify.warning('Uncovered joints: %s' % (list(diff)))

    def loadAnims(self, anims, partName="modelRoot", lodName="lodRoot"):
        """loadAnims(self, string:string{}, string='modelRoot',
        string='lodRoot')
        Actor anim loader. Takes an optional partName (defaults to
        'modelRoot' for non-multipart actors) and lodName (defaults
        to 'lodRoot' for non-LOD actors) and dict of corresponding
        anims in the form animName:animPath{}
        """
        reload = True
        if self.mergeLODBundles:
            lodNames = ['common']
        elif lodName == 'all':
            reload = False
            lodNames = list(self.switches.keys())
            lodNames.sort()
            for i in range(0, len(lodNames)):
                lodNames[i] = str(lodNames[i])
        else:
            lodNames = [lodName]

        assert Actor.notify.debug("in loadAnims: %s, part: %s, lod: %s" %
                                  (anims, partName, lodNames[0]))

        firstLoad = True
        if not reload:
            try:
                self.__animControlDict[lodNames[0]][partName]
                firstLoad = False
            except:
                pass
        for lName in lodNames:
            if firstLoad:
                self.__animControlDict.setdefault(lName, {})
                self.__animControlDict[lName].setdefault(partName, {})

        for animName, filename in anims.items():
            # make sure this lod is in anim control dict
            for lName in lodNames:
                if firstLoad:
                    self.__animControlDict[lName][partName][animName] = Actor.AnimDef()

                if isinstance(filename, NodePath):
                    # We were given a pre-load anim bundle, not a filename.
                    assert not filename.isEmpty()
                    if filename.node().isOfType(AnimBundleNode.getClassType()):
                        animBundleNP = filename
                    else:
                        animBundleNP = filename.find('**/+AnimBundleNode')
                    assert not animBundleNP.isEmpty()
                    self.__animControlDict[lName][partName][animName].animBundle = animBundleNP.node().getBundle()

                else:
                    # We were given a filename that must be loaded.
                    # Store the filename only; we will load and bind
                    # it (and produce an AnimControl) when it is
                    # played.
                    self.__animControlDict[lName][partName][animName].filename = filename

    def initAnimsOnAllLODs(self,partNames):
        if self.mergeLODBundles:
            lodNames = ['common']
        else:
            lodNames = self.__partBundleDict.keys()

        for lod in lodNames:
            for part in partNames:
                self.__animControlDict.setdefault(lod,{})
                self.__animControlDict[lod].setdefault(part, {})

        #for animName, filename in anims.items():
        #    # make sure this lod is in anim control dict
        #    for lod in self.__partBundleDict.keys():
        #        # store the file path only; we will bind it (and produce
        #        # an AnimControl) when it is played
        #
        #        self.__animControlDict[lod][partName][animName] = Actor.AnimDef(filename)

    def loadAnimsOnAllLODs(self, anims,partName="modelRoot"):
        """loadAnims(self, string:string{}, string='modelRoot',
        string='lodRoot')
        Actor anim loader. Takes an optional partName (defaults to
        'modelRoot' for non-multipart actors) and lodName (defaults
        to 'lodRoot' for non-LOD actors) and dict of corresponding
        anims in the form animName:animPath{}
        """
        if self.mergeLODBundles:
            lodNames = ['common']
        else:
            lodNames = self.__partBundleDict.keys()

        for animName, filename in anims.items():
            # make sure this lod is in anim control dict
            for lod in lodNames:
                # store the file path only; we will bind it (and produce
                # an AnimControl) when it is played

                self.__animControlDict[lod][partName][animName]= Actor.AnimDef(filename)

    def postFlatten(self):
        """Call this after performing an aggressive flatten operation,
        such as flattenStrong(), that involves the Actor.  This is
        especially necessary when mergeLODBundles is true, since this
        kind of actor may be broken after a flatten operation; this
        method should restore proper Actor functionality. """

        if self.mergeLODBundles:
            # Re-merge all bundles, and restore the common bundle map.
            self.__commonBundleHandles = {}
            for lodName, bundleDict in self.__partBundleDict.items():
                for partName, partDef in bundleDict.items():
                    loadedBundleHandle = self.__commonBundleHandles.get(partName, None)
                    node = partDef.partBundleNP.node()
                    if loadedBundleHandle:
                        node.mergeBundles(partDef.partBundleHandle, loadedBundleHandle)
                        partDef.partBundleHandle = loadedBundleHandle
                    else:
                        self.__commonBundleHandles[partName] = partDef.partBundleHandle

        # Since we may have merged together some bundles, all of
        # our anims are now suspect.  Force them to reload.
        self.unloadAnims()

    def unloadAnims(self, anims=None, partName=None, lodName=None):
        """unloadAnims(self, string:string{}, string='modelRoot',
        string='lodRoot')
        Actor anim unloader. Takes an optional partName (defaults to
        'modelRoot' for non-multipart actors) and lodName (defaults to
        'lodRoot' for non-LOD actors) and list of animation
        names. Deletes the anim control for the given animation and
        parts/lods.

        If any parameter is None or omitted, it means all of them.
        """
        assert Actor.notify.debug("in unloadAnims: %s, part: %s, lod: %s" %
                                  (anims, partName, lodName))

        if lodName is None or self.mergeLODBundles:
            lodNames = self.__animControlDict.keys()
        else:
            lodNames = [lodName]

        if partName is None:
            if len(lodNames) > 0:
                partNames = self.__animControlDict[next(iter(lodNames))].keys()
            else:
                partNames = []
        else:
            partNames = [partName]

        if anims is None:
            for lodName in lodNames:
                for partName in partNames:
                    for animDef in self.__animControlDict[lodName][partName].values():
                        if animDef.animControl != None:
                            # Try to clear any control effects before we let
                            # our handle on them go. This is especially
                            # important if the anim control was blending
                            # animations.
                            animDef.animControl.getPart().clearControlEffects()
                            animDef.animControl = None
        else:
            for lodName in lodNames:
                for partName in partNames:
                    for anim in anims:
                        animDef = self.__animControlDict[lodName][partName].get(anim)
                        if animDef and animDef.animControl != None:
                            # Try to clear any control effects before we let
                            # our handle on them go. This is especially
                            # important if the anim control was blending
                            # animations.
                            animDef.animControl.getPart().clearControlEffects()
                            animDef.animControl = None


    def bindAnim(self, animName, partName = None, lodName = None,
                 allowAsyncBind = False):
        """
        Binds the named animation to the named part and/or lod.  If
        allowAsyncBind is False, this guarantees that the animation is
        bound immediately--the animation is never bound in a
        sub-thread; it will be loaded and bound in the main thread, so
        it will be available by the time this method returns.

        The parameters are the same as that for getAnimControls().  In
        fact, this method is a thin wrapper around that other method.

        Use this method if you need to ensure that an animation is
        available before you start to play it, and you don't mind
        holding up the render for a frame or two until the animation
        is available.
        """
        self.getAnimControls(animName = animName, partName = partName,
                             lodName = lodName,
                             allowAsyncBind = allowAsyncBind)

    def bindAllAnims(self, allowAsyncBind = False):
        """Loads and binds all animations that have been defined for
        the Actor. """
        self.getAnimControls(animName = True, allowAsyncBind = allowAsyncBind)

    def waitPending(self, partName = None):
        """Blocks until all asynchronously pending animations (that
        are currently playing) have been loaded and bound the the
        Actor.  Call this after calling play() if you are using
        asynchronous binds, but you need this particular animation
        to be loaded immediately. """

        for bundle in self.getPartBundles(partName = partName):
            bundle.waitPending()

    def __bindAnimToPart(self, animName, partName, lodName,
                         allowAsyncBind = True):
        """
        Binds the named animation to the named part/lod and returns
        the associated animControl.  The animation is loaded and bound
        in a sub-thread, if allowAsyncBind is True,
        self.allowAsyncBind is True, threading is enabled, and the
        animation has a preload table generated for it (e.g. via
        "egg-optchar -preload").  Even though the animation may or may
        not be yet bound at the time this function returns, a usable
        animControl is returned, or None if the animation could not be
        bound.
        """
        # make sure this anim is in the dict
        subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))

        partDict = self.__animControlDict[lodName]
        animDict = partDict.get(partName)
        if animDict == None:
            # It must be a subpart that hasn't been bound yet.
            animDict = {}
            partDict[partName] = animDict

        anim = animDict.get(animName)
        if anim == None:
            # It must be a subpart that hasn't been bound yet.
            anim = partDict[subpartDef.truePartName].get(animName)
            anim = anim.makeCopy()
            animDict[animName] = anim

        if anim == None:
            Actor.notify.error("actor has no animation %s", animName)

        # only bind if not already bound!
        if anim.animControl:
            return anim.animControl

        if self.mergeLODBundles:
            bundle = self.__commonBundleHandles[subpartDef.truePartName].getBundle()
        else:
            bundle = self.__partBundleDict[lodName][subpartDef.truePartName].getBundle()

        if anim.animBundle:
            # We already have a bundle; just bind it.
            animControl = bundle.bindAnim(anim.animBundle, -1, subpartDef.subset)

        else:
            # Load and bind the anim.  This might be an asynchronous
            # operation that will complete in the background, but if so it
            # will still return a usable AnimControl.
            animControl = bundle.loadBindAnim(
                self.loader, Filename(anim.filename), -1,
                subpartDef.subset, allowAsyncBind and self.allowAsyncBind)

        if not animControl:
            # Couldn't bind.  (This implies the binding operation was
            # not attempted asynchronously.)
            return None

        # store the animControl
        anim.animControl = animControl
        assert Actor.notify.debug("binding anim: %s to part: %s, lod: %s" %
                                  (animName, partName, lodName))
        return animControl

    def __copyPartBundles(self, other):
        """__copyPartBundles(self, Actor)
        Copy the part bundle dictionary from another actor as this
        instance's own. NOTE: this method does not actually copy geometry
        """
        for lodName in other.__partBundleDict:
            # find the lod Asad
            if lodName == 'lodRoot':
                partLod = self
            else:
                partLod = self.__LODNode.find(str(lodName))
            if partLod.isEmpty():
                Actor.notify.warning("no lod named: %s" % (lodName))
                return None
            for partName, partDef in other.__partBundleDict[lodName].items():
                # We can really only copy from a non-flattened avatar.
                assert partDef.partBundleNP.node().getNumBundles() == 1

                # find the part in our tree
                bundleNP = partLod.find("**/%s%s"%(Actor.partPrefix,partName))
                if (bundleNP != None):
                    # store the part bundle
                    self.__prepareBundle(bundleNP, partDef.partModel,
                                         partName, lodName)
                else:
                    Actor.notify.error("lod: %s has no matching part: %s" %
                                       (lodName, partName))

    def __copySubpartDict(self, other):
        """Copies the subpartDict from another as this instance's own.
        This makes a deep copy of the map and all of the names and
        PartSubset objects within it.  We can't use copy.deepcopy()
        because of the included C++ PartSubset objects."""

        self.__subpartDict = {}
        for partName, subpartDef in other.__subpartDict.items():
            subpartDefCopy = subpartDef
            if subpartDef:
                subpartDef = subpartDef.makeCopy()
            self.__subpartDict[partName] = subpartDef

    def __copyAnimControls(self, other):
        """__copyAnimControls(self, Actor)
        Get the anims from the anim control's in the anim control
        dictionary of another actor. Bind these anim's to the part
        bundles in our part bundle dict that have matching names, and
        store the resulting anim controls in our own part bundle dict"""

        assert(other.mergeLODBundles == self.mergeLODBundles)

        for lodName in other.__animControlDict:
            self.__animControlDict[lodName] = {}
            for partName in other.__animControlDict[lodName]:
                self.__animControlDict[lodName][partName] = {}
                for animName in other.__animControlDict[lodName][partName]:
                    anim = other.__animControlDict[lodName][partName][animName]
                    anim = anim.makeCopy()
                    self.__animControlDict[lodName][partName][animName] = anim


    def actorInterval(self, *args, **kw):
        from direct.interval import ActorInterval
        return ActorInterval.ActorInterval(self, *args, **kw)

    def getAnimBlends(self, animName=None, partName=None, lodName=None):
        """Returns a list of the form::

           [ (lodName, [(animName, [(partName, effect), (partName, effect), ...]),
                        (animName, [(partName, effect), (partName, effect), ...]),
                        ...]),
             (lodName, [(animName, [(partName, effect), (partName, effect), ...]),
                        (animName, [(partName, effect), (partName, effect), ...]),
                        ...]),
              ... ]

        This list reports the non-zero control effects for each
        partName within a particular animation and LOD. """

        result = []

        if animName is None:
            animNames = self.getAnimNames()
        else:
            animNames = [animName]

        if lodName is None:
            lodNames = self.getLODNames()
            if self.mergeLODBundles:
                lodNames = lodNames[:1]
        else:
            lodNames = [lodName]

        if partName == None and self.__subpartsComplete:
            partNames = self.__subpartDict.keys()
        else:
            partNames = [partName]

        for lodName in lodNames:
            animList = []
            for animName in animNames:
                blendList = []
                for partName in partNames:
                    control = self.getAnimControl(animName, partName, lodName)
                    if control:
                        part = control.getPart()
                        effect = part.getControlEffect(control)
                        if effect > 0.:
                            blendList.append((partName, effect))
                if blendList:
                    animList.append((animName, blendList))
            if animList:
                result.append((lodName, animList))

        return result

    def printAnimBlends(self, animName=None, partName=None, lodName=None):
        for lodName, animList in self.getAnimBlends(animName, partName, lodName):
            print('LOD %s:' % (lodName))
            for animName, blendList in animList:

                list = []
                for partName, effect in blendList:
                    list.append('%s:%.3f' % (partName, effect))
                print('  %s: %s' % (animName, ', '.join(list)))

    def osdAnimBlends(self, animName=None, partName=None, lodName=None):
        if not onScreenDebug.enabled:
            return
        # puts anim blending info into the on-screen debug panel
        if animName is None:
            animNames = self.getAnimNames()
        else:
            animNames = [animName]
        for animName in animNames:
            if animName == 'nothing':
                continue
            thisAnim = ''
            totalEffect = 0.
            controls = self.getAnimControls(animName, partName, lodName)
            for control in controls:
                part = control.getPart()
                name = part.getName()
                effect = part.getControlEffect(control)
                if effect > 0.:
                    totalEffect += effect
                    thisAnim += ('%s:%.3f, ' % (name, effect))
            thisAnim += "\n"
            for control in controls:
                part = control.getPart()
                name = part.getName()
                rate = control.getPlayRate()
                thisAnim += ('%s:%.1f, ' % (name, rate))
            # don't display anything if this animation is not being played
            itemName = 'anim %s' % animName
            if totalEffect > 0.:
                onScreenDebug.add(itemName, thisAnim)
            else:
                if onScreenDebug.has(itemName):
                    onScreenDebug.remove(itemName)

    # these functions compensate for actors that are modeled facing the viewer but need
    # to face away from the camera in the game
    def faceAwayFromViewer(self):
        self.getGeomNode().setH(180)
    def faceTowardsViewer(self):
        self.getGeomNode().setH(0)

    def renamePartBundles(self, partName, newBundleName):
        subpartDef = self.__subpartDict.get(partName, Actor.SubpartDef(partName))
        for partBundleDict in self.__partBundleDict.values():
            partDef = partBundleDict.get(subpartDef.truePartName)
            partDef.getBundle().setName(newBundleName)

    #snake_case alias:
    control_joint = controlJoint
    set_lod_animation = setLODAnimation
    get_anim_control_dict = getAnimControlDict
    get_actor_info = getActorInfo
    clear_lod_animation = clearLODAnimation
    reset_lod = resetLOD
    fix_bounds = fixBounds
    get_anim_filename = getAnimFilename
    get_subparts_complete = getSubpartsComplete
    verify_subparts_complete = verifySubpartsComplete
    get_play_rate = getPlayRate
    clear_python_data = clearPythonData
    load_anims = loadAnims
    set_subparts_complete = setSubpartsComplete
    draw_in_front = drawInFront
    get_lod_node = getLODNode
    hide_part = hidePart
    get_joint_transform_state = getJointTransformState
    set_control_effect = setControlEffect
    get_anim_controls = getAnimControls
    release_joint = releaseJoint
    print_anim_blends = printAnimBlends
    get_lod = getLOD
    disable_blend = disableBlend
    show_part = showPart
    get_joint_transform = getJointTransform
    face_away_from_viewer = faceAwayFromViewer
    set_lod = setLOD
    osd_anim_blends = osdAnimBlends
    get_current_frame = getCurrentFrame
    set_play_rate = setPlayRate
    bind_all_anims = bindAllAnims
    unload_anims = unloadAnims
    remove_part = removePart
    use_lod = useLOD
    get_anim_blends = getAnimBlends
    get_lod_index = getLODIndex
    get_num_frames = getNumFrames
    post_flatten = postFlatten
    get_lod_names = getLODNames
    list_joints = listJoints
    make_subpart = makeSubpart
    get_anim_control = getAnimControl
    get_part_bundle = getPartBundle
    get_part_bundle_dict = getPartBundleDict
    get_duration = getDuration
    has_lod = hasLOD
    print_lod = printLOD
    fix_bounds_old = fixBounds_old
    get_anim_names = getAnimNames
    get_part_bundles = getPartBundles
    anim_panel = animPanel
    stop_joint = stopJoint
    actor_interval = actorInterval
    hide_all_bounds = hideAllBounds
    show_all_bounds = showAllBounds
    init_anims_on_all_lods = initAnimsOnAllLODs
    get_part = getPart
    add_lod = addLOD
    show_all_parts = showAllParts
    get_joints = getJoints
    get_overlapping_joints = getOverlappingJoints
    enable_blend = enableBlend
    face_towards_viewer = faceTowardsViewer
    bind_anim = bindAnim
    set_blend = setBlend
    get_frame_time = getFrameTime
    remove_node = removeNode
    wait_pending = waitPending
    expose_joint = exposeJoint
    set_lod_node = setLODNode
    get_frame_rate = getFrameRate
    get_current_anim = getCurrentAnim
    get_part_names = getPartNames
    freeze_joint = freezeJoint
    set_center = setCenter
    rename_part_bundles = renamePartBundles
    get_geom_node = getGeomNode
    set_geom_node = setGeomNode
    load_model = loadModel
    copy_actor = copyActor
    get_base_frame_rate = getBaseFrameRate
    remove_anim_control_dict = removeAnimControlDict
    load_anims_on_all_lods = loadAnimsOnAllLODs
