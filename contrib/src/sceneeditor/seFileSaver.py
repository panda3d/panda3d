####################################################################################################################################################
# File Saving
# This code saves the scene out as python code... the scene is stored in the various dictionaries in "dataHolder.py" ...the class "AllScene"
#
####################################################################################################################################################
from panda3d.core import *

from direct.showbase.ShowBaseGlobal import *
import os
import shutil
import string

####################################################################################################################################################
#### These modules are modified versions of Disney's equivalent modules
#### We need to figure out a way to inherit their modules and overwrite what we need changed
import seParticlePanel
import seParticles
import seParticleEffect
import seForceGroup
####################################################################################################################################################

class FileSaver:

    ####################################################################################################################################################
    # This class saves out the scene built with the scene editor as python code
    # There are dictionaries saved out to save the state of the scene for reloading it with the editor
    # Currently saving is supported for Models, Animations, Lights, Dummy Nodes
    # Attributes like parenting are also saved out
    # This class is actually instantiated in sceneEditor.py in the saveScene() method
    ####################################################################################################################################################

    def __init(self):
        pass

    def SaveFile(self,AllScene,filename,dirname,reSaveFlag=0):

      ################################################################################################################################################
        # This function takes the "dataHolder" instance "AllScene" which has dictionaries containing scene information
        # The filename is where the scene will be written to
        ################################################################################################################################################

        i1="    " # indentation
        i2=i1+i1  # double indentation
        out_file = open(filename,"w")
        print("dirname:" + dirname)
        if( not os.path.isdir(dirname)):
            os.mkdir(dirname)
        savepathname=Filename(filename)
        self.savepath=savepathname.getBasenameWoExtension()
        out_file.write("##########################################################################################################\n")
        out_file.write("# Auto Generated Code by Scene Editor\n")
        out_file.write("# Edit with caution\n")
        out_file.write("# Using this file in your code:\n")
        out_file.write("# For example, if you have named this file as \"myscene.py\"\n")
        out_file.write("# Do the following:\n")
        out_file.write("# from myscene import * \n")
        out_file.write("# theScene=SavedScene() #instantiate the class\n")
        out_file.write("# IMPORTANT: All the documentation below refers to \"theScene\" as the instance of SavedScene()\n")
        out_file.write("##########################################################################################################\n\n")

        out_file.write("##########################################################################################################\n")
        out_file.write("# Import Panda Modules\n")
        out_file.write("##########################################################################################################\n")
        out_file.write("from direct.directbase.DirectStart import * # Core functionality for running the \"show\"\n")
        out_file.write("from direct.actor import Actor # Importing models with animations\n")
        out_file.write("from direct.directutil import Mopath # Motion Paths\n")
        out_file.write("from direct.interval import MopathInterval # Motion Paths\n")
        out_file.write("from direct.interval.IntervalGlobal import * # Intervals for interpolation, sequencing and parallelization\n")
        out_file.write("from direct.particles import ParticleEffect # Particle Systems\n")
        out_file.write("from direct.particles import ForceGroup # Forces acting on Particles\n")
        out_file.write("from direct.particles import Particles\n\n")
        out_file.write("##########################################################################################################\n")
        out_file.write("# This class stores the entire scene\n")
        out_file.write("##########################################################################################################\n\n")
        out_file.write("class SavedScene(DirectObject): # We inherit from DirectObject so that we can use self.accept method to catch messages\n")
        out_file.write("\n")
        out_file.write(i1+"# These dictionaries are required for re-loading a scene in the editor\n")
        out_file.write(i1+"# They can be used to access the objects as well\n\n")
        out_file.write(i1+"ModelDic={}# Stores all the models and static geometry\n")
        out_file.write(i1+"ModelRefDic={}# Stores the paths to the models\n")
        out_file.write("\n")
        out_file.write(i1+"ActorDic={}# Stores all the actors\n")
        out_file.write(i1+"ActorRefDic={}# Stores the paths to the actors\n")
        out_file.write(i1+"ActorAnimsDic={}# Stores the animations for each actor\n")
        out_file.write(i1+"blendAnimDict={}# Stores all the blended animations\n")
        out_file.write("\n")
        out_file.write(i1+"LightDict={}# Stores all the lights\n")
        out_file.write(i1+"LightTypes={}# Stores types for the lights\n")
        out_file.write(i1+"LightNodes={}# Stores the actual nodes for the lights\n")
        out_file.write("\n")
        out_file.write(i1+"dummyDict={}# Stores dummies\n")
        out_file.write("\n")
        out_file.write(i1+"collisionDict={}# Stores Collision information\n")
        out_file.write("\n")
        out_file.write(i1+"curveDict={}# Stores Mopath information\n")
        out_file.write(i1+"curveIntervals=[]# Stores list of mopath intervals\n")
        out_file.write(i1+"curveRefColl=[]# Stores paths to mopaths\n")
        out_file.write(i1+"curveIntervalsDict={}# Stores mopath intervals\n")
        out_file.write("\n")
        out_file.write(i1+"particleDict={}# Stores particles\n")
        out_file.write(i1+"particleNodes={}# Stores particle nodes\n")
        out_file.write("\n")
        out_file.write(i1+"#Light Count\n")
        out_file.write(i1+"ambientCount=0\n")
        out_file.write(i1+"directionalCount=0\n")
        out_file.write(i1+"pointCount=0\n")
        out_file.write(i1+"spotCount=0\n")
        out_file.write("\n")
        out_file.write(i1+"#Lighting Attribute\n")
        out_file.write(i1+"lightAttrib = LightAttrib.makeAllOff()# Initialize lighting\n")
        out_file.write("\n")
        out_file.write(i1+"CollisionHandler=CollisionHandlerEvent()# Setup a Collision Handler\n")

        out_file.write(i1+"##########################################################################################################\n")
        out_file.write(i1+"# Constructor: this is run first when you instantiate the SavedScene class\n")
        out_file.write(i1+"##########################################################################################################\n")
        out_file.write(i1+"def __init__(self,loadmode=1,seParticleEffect=None,seParticles=None,executionpath=None):# loadmode 0 specifies that this file is being loaded by the scene editor and it passes its own versions of the particle fx modules\n")
        out_file.write("\n")
        out_file.write(i2+"self.loadmode=loadmode\n")
        out_file.write(i2+"self.seParticleEffect=seParticleEffect\n")
        out_file.write(i2+"self.seParticles=seParticles\n")
        out_file.write(i2+"self.executionpath=executionpath\n")
        out_file.write("\n")
        out_file.write(i2+"base.enableParticles()# Enable Particle effects\n")
        out_file.write("\n")
        out_file.write(i2+"self.cTrav = CollisionTraverser() # Setup a traverser for collisions\n")
        out_file.write(i2+"base.cTrav = self.cTrav\n")
        out_file.write(i2+"self.CollisionHandler.setInPattern(\"enter%in\")# The message to be raised when something enters a collision node\n")
        out_file.write(i2+"self.CollisionHandler.setOutPattern(\"exit%in\")# The message to be raised when something exits a collision node\n")
        out_file.write("\n")
        ####################################################################################################################################################
        # Save Models
        ####################################################################################################################################################

        out_file.write(i2+"##########################################################################################################\n")
        out_file.write(i2+"# Code for all the models\n")
        out_file.write(i2+"# To access these models:\n")
        out_file.write(i2+"# theScene.ModelDic[\"'Model_Name\"']\n")
        out_file.write(i2+"# where theScene is the SavedScene class instance\n")
        out_file.write(i2+"# Properties saved include:\n")
        out_file.write(i2+"# Transformations\n")
        out_file.write(i2+"# Alpha and color\n")
        out_file.write(i2+"# Parent and child information\n")
        out_file.write(i2+"##########################################################################################################\n")

        for model in AllScene.ModelDic:
            out_file.write("\n")
            modelS=str(model)

            if(1): # This is kept for now... perhaps later some sort of check might have to be enforced based on loadMode
                #Loading Code
                out_file.write(i2+"# Loading model's egg file\n")
                #out_file.write(i2+ "self."+ modelS + "=loader.loadModel(\'" + AllScene.ModelRefDic[model].getFullpath() + "\')\n")#Absolute Paths

                newpath = dirname + "/" + AllScene.ModelRefDic[model].getBasename()
                newpathF=Filename(newpath)
                newpathSpecific=newpathF.toOsSpecific()

                # Copy all the textures referenced by this file over to the relative directory
                fnamelist=[]
                modelData=EggData()
                modelData.read(AllScene.ModelRefDic[model])
                textures=EggTextureCollection()
                textures.findUsedTextures(modelData)
                for index in range(textures.getNumTextures()):
                    texture=textures.getTexture(index)
                    texfilename=texture.getFilename()
                    fnamelist.append(texfilename.getFullpath())
                    oldFilename=Filename(Filename(AllScene.ModelRefDic[model].getDirname()),texfilename)
                    if(not oldFilename.isRegularFile()):
                        if(texfilename.resolveFilename(getTexturePath(),"")):
                            oldFilename=texfilename
                    oldtexpath=oldFilename.toOsSpecific()

                    newtexpath=dirname + "/" + texfilename.getBasename()
                    newtexpathF=Filename(newtexpath)
                    newtexpathSpecific=newtexpathF.toOsSpecific()

                    print("TEXTURE SAVER:: copying" + oldtexpath + " to " + newtexpathSpecific)
                    if(oldtexpath != newtexpathSpecific):
                        shutil.copyfile(oldtexpath,newtexpathSpecific)






                # Copy the file over to the relative directory
                oldModelpath=AllScene.ModelRefDic[model].toOsSpecific()
                print("FILESAVER:: copying from " + AllScene.ModelRefDic[model].toOsSpecific() + "to" + newpathSpecific)
                if(oldModelpath!=newpathSpecific):
                    shutil.copyfile(oldModelpath,newpathSpecific)


                e=EggData()
                e.read(AllScene.ModelRefDic[model])
                etc=EggTextureCollection()
                etc.extractTextures(e)
                for index in range(len(fnamelist)):
                    print(fnamelist[index])
                    tex=etc.findFilename(Filename(fnamelist[index]))
                    fn=Filename(tex.getFilename())
                    fn.setDirname("")
                    tex.setFilename(fn)
                    e.writeEgg(Filename.fromOsSpecific(newpathSpecific))



                out_file.write(i2+"if(self.loadmode==1):\n")
                out_file.write(i2+i1+ "self."+ modelS + "=loader.loadModel(\'" + self.savepath + "/" +  AllScene.ModelRefDic[model].getBasename() + "')\n")#Relative Path
                out_file.write(i2+"else:\n")
                out_file.write(i2+i1+ "self."+ modelS + "=loader.loadModel(self.executionpath + \'/" +  AllScene.ModelRefDic[model].getBasename() + "')\n")#Relative Path with execution point specified by the invoking-level-editor

                #Transformation Code
                out_file.write("\n")
                out_file.write(i2+"# Transforming the model\n")
                out_file.write(i2+ "self."+ modelS + ".setPosHprScale(%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f)\n"% (AllScene.ModelDic[model].getX(),AllScene.ModelDic[model].getY(),AllScene.ModelDic[model].getZ(),AllScene.ModelDic[model].getH(),AllScene.ModelDic[model].getP(),AllScene.ModelDic[model].getR(),AllScene.ModelDic[model].getSx(),AllScene.ModelDic[model].getSy(),AllScene.ModelDic[model].getSz()))

                if(AllScene.ModelDic[model].hasTransparency()):
                    out_file.write("\n")
                    out_file.write(i2+"# Alpha\n")
                    out_file.write(i2+ "self."+ modelS + ".setTransparency(1)\n")
                    clr=AllScene.ModelDic[model].getColor()
                    out_file.write(i2+ "self."+ modelS + ".setColor(%.4f,%.4f,%.4f,%.4f)\n"%(clr.getX(),clr.getY(),clr.getZ(),clr.getW()))

                out_file.write("\n")
                out_file.write(i2+ "# Reparent To Render for now and later we update all the parentings\n")
                out_file.write(i2+ "self."+ modelS + ".reparentTo(render)\n")
                out_file.write("\n")
                out_file.write(i2+ "# Save Metadata...can be retrieved by doing theScene.ModelDic[\"Model_Name\"].getTag(\"Metadata\")\n")
                out_file.write(i2+ "self."+ modelS + ".setTag(\"Metadata\",\"" + AllScene.ModelDic[model].getTag("Metadata") + "\")\n")
                out_file.write("\n")
                out_file.write(i2+ "# Fill in the dictionaries which are used by level Ed to reload state\n")
                out_file.write(i2+ "self.ModelDic[\'" + modelS + "\']=self." + AllScene.ModelDic[model].getName()+"\n")
                #out_file.write(i2+ "self.ModelRefDic[\'" + modelS + "\']=Filename(\'"+ AllScene.ModelRefDic[model].getFullpath() +"\')\n")# The old Absolute Path way
                out_file.write(i2+ "self.ModelRefDic[\'" + modelS + "\']=\'"+ AllScene.ModelRefDic[model].getBasename() +"\'\n")# Relative paths
                out_file.write(i2+ "self.ModelDic[\'"+ modelS + "\'].setName(\'"+ modelS +"\')\n")
                out_file.write("\n")

        ####################################################################################################################################################
        # Save Dummies
        ####################################################################################################################################################
        out_file.write(i2+"##########################################################################################################\n")
        out_file.write(i2+"# Code for all the Dummy Objects\n")
        out_file.write(i2+"# To access the dummies\n")
        out_file.write(i2+"# theScene.dummyDict['Dummy_Name']\n")
        out_file.write(i2+"##########################################################################################################\n")
        for dummy in AllScene.dummyDict:
            out_file.write("\n")
            dummyS=str(dummy)

            if(1): # This is kept for now... perhaps later some sort of check might have to be enforced based on loadMode
                out_file.write(i2+ "self."+ dummyS + "=loader.loadModel(\"models/misc/sphere\")\n")
                #Transformation Code
                out_file.write(i2+"# Transforming the Dummy\n")
                out_file.write(i2+ "self."+ dummyS + ".setPosHprScale(%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f)\n"% (AllScene.dummyDict[dummy].getX(),AllScene.dummyDict[dummy].getY(),AllScene.dummyDict[dummy].getZ(),AllScene.dummyDict[dummy].getH(),AllScene.dummyDict[dummy].getP(),AllScene.dummyDict[dummy].getR(),AllScene.dummyDict[dummy].getSx(),AllScene.dummyDict[dummy].getSy(),AllScene.dummyDict[dummy].getSz()))
                out_file.write("\n")
                out_file.write(i2+ "# Fill in the dictionaries which are used by level Ed to reload state\n")
                out_file.write(i2+ "self.dummyDict[\'" + dummyS + "\']=self." + AllScene.dummyDict[dummy].getName()+"\n")
                out_file.write(i2+ "self.dummyDict[\'"+ dummyS + "\'].setName(\'"+ dummyS +"\')\n")
                out_file.write("\n")
                out_file.write(i2+ "# Save Metadata...can be retrieved by doing theScene.dummyDict[\"Dummy_Name\"].getTag(\"Metadata\")\n")
                out_file.write(i2+ "self."+ dummyS + ".setTag(\"Metadata\",\"" + AllScene.dummyDict[dummy].getTag("Metadata") + "\")\n")
                out_file.write("\n")

        ####################################################################################################################################################
        # Saving Actors and their animations
        ####################################################################################################################################################
        out_file.write(i2+"##########################################################################################################\n")
        out_file.write(i2+"# Code for all the Actors and animations\n")
        out_file.write(i2+"# To access the Actors\n")
        out_file.write(i2+"# theScene.ActorDic[\'Actor_Name\']\n")
        out_file.write(i2+"# theScene.ActorDic[\'Actor_Name\'].play(\'Animation_Name\')\n")
        out_file.write(i2+"##########################################################################################################\n")
        for actor in AllScene.ActorDic:
            out_file.write("\n")
            actorS=str(actor)

            if(1): # This is kept for now... perhaps later some sort of check might have to be enforced based on loadMode
                #out_file.write(i2+ "self."+ actorS + "=Actor.Actor(\'"+ AllScene.ActorRefDic[actor].getFullpath() + "\')\n")# The old way with absolute paths


                newpath = dirname + "/" + AllScene.ActorRefDic[actor].getBasename()
                newpathF=Filename(newpath)
                newpathSpecific=newpathF.toOsSpecific()

                # Copy all the textures referenced by this file over to the relative directory
                actorfnamelist=[]
                actorData=EggData()
                actorData.read(AllScene.ActorRefDic[actor])
                textures=EggTextureCollection()
                textures.findUsedTextures(actorData)
                for index in range(textures.getNumTextures()):
                    texture=textures.getTexture(index)
                    texfilename=texture.getFilename()
                    actorfnamelist.append(texfilename.getFullpath())

                    oldFilename=Filename(Filename(AllScene.ActorRefDic[actor].getDirname()),texfilename)
                    if(not oldFilename.isRegularFile()):
                        if(texfilename.resolveFilename(getTexturePath(),"")):
                            oldFilename=texfilename
                    oldtexpath=oldFilename.toOsSpecific()


                    newtexpath=dirname + "/" + texfilename.getBasename()
                    newtexpathF=Filename(newtexpath)
                    newtexpathSpecific=newtexpathF.toOsSpecific()
                    print("TEXTURE SAVER:: copying" + oldtexpath + " to " + newtexpathSpecific)
                    if(oldtexpath != newtexpathSpecific):
                        shutil.copyfile(oldtexpath,newtexpathSpecific)


                # Copy the file over to the relative directory
                oldActorpath=AllScene.ActorRefDic[actor].toOsSpecific()
                print("FILESAVER:: copying from " + AllScene.ActorRefDic[actor].toOsSpecific() + "to" + newpathSpecific)
                if(oldActorpath!=newpathSpecific):
                    shutil.copyfile(oldActorpath,newpathSpecific)


                e=EggData()
                e.read(AllScene.ActorRefDic[actor])
                etc=EggTextureCollection()
                etc.extractTextures(e)
                for index in range(len(actorfnamelist)):
                    print(actorfnamelist[index])
                    tex=etc.findFilename(Filename(actorfnamelist[index]))
                    fn=Filename(tex.getFilename())
                    fn.setDirname("")
                    tex.setFilename(fn)
                    e.writeEgg(Filename.fromOsSpecific(newpathSpecific))



                out_file.write(i2+"if(self.loadmode==1):\n")
                out_file.write(i2+i1+ "self."+ actorS + "=Actor.Actor(\'" + self.savepath + "/" +  AllScene.ActorRefDic[actor].getBasename() + "')\n")#Relative Path
                out_file.write(i2+"else:\n")
                out_file.write(i2+i1+ "self."+ actorS + "=Actor.Actor(self.executionpath + \'/" +  AllScene.ActorRefDic[actor].getBasename() + "')\n")#Relative Path with execution point specified by the invoking-level-editor

                #Transformation Code
                out_file.write(i2+"# Transforming the Actor\n")
                out_file.write(i2+ "self."+ actorS + ".setPosHprScale(%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f)\n"% (AllScene.ActorDic[actor].getX(),AllScene.ActorDic[actor].getY(),AllScene.ActorDic[actor].getZ(),AllScene.ActorDic[actor].getH(),AllScene.ActorDic[actor].getP(),AllScene.ActorDic[actor].getR(),AllScene.ActorDic[actor].getSx(),AllScene.ActorDic[actor].getSy(),AllScene.ActorDic[actor].getSz()))

                if(AllScene.ActorDic[actor].hasTransparency()):
                    out_file.write(i2+"# Alpha\n")
                    out_file.write(i2+ "self."+ actorS + ".setTransparency(1)\n")
                    clr=AllScene.ActorDic[actor].getColor()
                    out_file.write(i2+ "self."+ actorS + ".setColor(%.4f,%.4f,%.4f,%.4f)\n"%(clr.getX(),clr.getY(),clr.getZ(),clr.getW()))

                out_file.write(i2+ "self."+ actorS + ".reparentTo(render)\n")

                out_file.write("\n")
                out_file.write(i2+ "# Save Metadata...can be retrieved by doing theScene.ActorDic[\"Actor_Name\"].getTag(\"Metadata\")\n")
                out_file.write(i2+ "self."+ actorS + ".setTag(\"Metadata\",\"" + AllScene.ActorDic[actor].getTag("Metadata") + "\")\n")

                out_file.write("\n")
                out_file.write(i2+ "# Fill in the dictionaries which are used by level Ed to reload state\n")
                ActorAnimations=AllScene.getAnimationDictFromActor(actor)
                ActorAnimationsInvoke={}

                if(ActorAnimations!={}):  #Check if a dictionary of animations exists for this actor
                    for animation in ActorAnimations:
                        #out_file.write(i2+ "self."+ actorS + ".loadAnims(" + str(ActorAnimations) +")\n") # Old way with absolute paths
                        #Manakel 2/12/2004: solve the not empty but not defined animation case
                        if not animation is None:
                            print("ACTOR ANIMATIONS:" + ActorAnimations[animation])
                            oldAnimPath=Filename(ActorAnimations[animation])
                            oldAnim=oldAnimPath.toOsSpecific()
                            dirOS=Filename(dirname)
                            newAnim=dirOS.toOsSpecific() + "\\" + oldAnimPath.getBasename()
                            print("ACTOR ANIM SAVER:: Comparing" + oldAnim +"and" + newAnim)
                            if(oldAnim!=newAnim):
                                shutil.copyfile(oldAnim,newAnim)
                            newAnimF=Filename.fromOsSpecific(newAnim)
                            ActorAnimationsInvoke[animation]="self.executionpath +" + "/" +newAnimF.getBasename()
                            ActorAnimations[animation]= self.savepath + "/" + newAnimF.getBasename()


                out_file.write(i2+"if(self.loadmode==1):\n")
                out_file.write(i2+ i1+"self."+ actorS + ".loadAnims(" + str(ActorAnimations) +")\n") # Now with new relative paths
                out_file.write(i2+"else:\n")
                theloadAnimString=str(ActorAnimationsInvoke)# We hack the "self.executionpath" part into the dictionary as a variable using string replace
                print("LOAD ANIM STRING BEFORE" + theloadAnimString)
                theloadAnimString=theloadAnimString.replace('\'self.executionpath +','self.executionpath + \'')
                print("LOAD ANIM STRING AFTER" + theloadAnimString)
                out_file.write(i2+ i1+"self."+ actorS + ".loadAnims(" + theloadAnimString +")\n") # Now with new relative paths based on editor invocation

                out_file.write(i2+ "self.ActorDic[\'" + actorS + "\']=self." + AllScene.ActorDic[actor].getName()+"\n")
                #out_file.write(i2+ "self.ActorRefDic[\'" + actorS + "\']=Filename(\'"+AllScene.ActorRefDic[actor].getFullpath() +"\')\n") # Old way with absolute paths
                out_file.write(i2+ "self.ActorRefDic[\'" + actorS + "\']=\'"+ AllScene.ActorRefDic[actor].getBasename() +"\'\n")# Relative paths
                out_file.write(i2+ "self.ActorDic[\'"+ actorS + "\'].setName(\'"+ actorS +"\')\n")
                if(actor in AllScene.blendAnimDict): # Check if a dictionary of blended animations exists
                    out_file.write(i2+ "self.blendAnimDict[\"" + actorS +"\"]=" + str(AllScene.blendAnimDict[actor]) + "\n")


                out_file.write("\n")

        ####################################################################################################################################################
        # Collsion Node Saving
        ####################################################################################################################################################

        out_file.write(i2+"##########################################################################################################\n")
        out_file.write(i2+"# Code for setting up Collision Nodes\n")
        out_file.write(i2+"# To use collision detection:\n")
        out_file.write(i2+"# You must set up your own bitmasking and event handlers, the traverser \"cTrav\" is created for you at the top\n")
        out_file.write(i2+"# The collision nodes are stored in collisionDict\n")
        out_file.write(i2+"##########################################################################################################\n\n")
        for collnode in AllScene.collisionDict:
            collnodeS=str(collnode)
            solid=AllScene.collisionDict[collnode].node().getSolid(0)
            nodetype=solid.getType().getName()

            if(nodetype=="CollisionSphere"): #Save Collison Sphere
                out_file.write(i2+"collSolid=CollisionSphere(%.3f,%.3f,%.3f,%.3f)\n"%(solid.getCenter().getX(),solid.getCenter().getY(),solid.getCenter().getZ(),solid.getRadius()))
                pass
            elif(nodetype=="CollisionPolygon"): #Save Collison Polygon

                ax=AllScene.collisionDict[collnode].getTag("A_X")
                ay=AllScene.collisionDict[collnode].getTag("A_Y")
                az=AllScene.collisionDict[collnode].getTag("A_Z")

                bx=AllScene.collisionDict[collnode].getTag("B_X")
                by=AllScene.collisionDict[collnode].getTag("B_Y")
                bz=AllScene.collisionDict[collnode].getTag("B_Z")

                cx=AllScene.collisionDict[collnode].getTag("C_X")
                cy=AllScene.collisionDict[collnode].getTag("C_Y")
                cz=AllScene.collisionDict[collnode].getTag("C_Z")

                out_file.write(i2+"pointA =  Point3(" + ax + "," + ay + "," + az + ")\n")
                out_file.write(i2+"pointB =  Point3(" + bx + "," + by + "," + bz + ")\n")
                out_file.write(i2+"pointC =  Point3(" + cx + "," + cy + "," + cz + ")\n")
                out_file.write(i2+"collSolid=CollisionPolygon(pointA, pointB, pointC)\n")

                pass

            elif(nodetype=="CollisionSegment"): #Save Collison Segment
                A=AllScene.collisionDict[collnode].node().getSolid(0).getPointA()
                B=AllScene.collisionDict[collnode].node().getSolid(0).getPointB()

                out_file.write(i2+"pointA =  Point3(%.3f,%.3f,%.3f)\n"%(A.getX(),A.getY(),A.getZ()))
                out_file.write(i2+"pointB =  Point3(%.3f,%.3f,%.3f)\n"%(B.getX(),B.getY(),B.getZ()))
                out_file.write(i2+"collSolid=CollisionSegment()\n")
                out_file.write(i2+"collSolid.setPointA(pointA)\n")
                out_file.write(i2+"collSolid.setFromLens(base.cam.node(),Point2(-1,1))\n")
                out_file.write(i2+"collSolid.setPointB(pointB)\n")

                pass

            elif(nodetype=="CollisionRay"): #Save Collison Ray
                P =  AllScene.collisionDict[collnode].node().getSolid(0).getOrigin()
                V =  AllScene.collisionDict[collnode].node().getSolid(0).getDirection()

                out_file.write(i2+"point=Point3(%.3f,%.3f,%.3f)\n"%(P.getX(),P.getY(),P.getZ()))
                out_file.write(i2+"vector=Vec3(%.3f,%.3f,%.3f)\n"%(V.getX(),V.getY(),V.getZ()))
                out_file.write(i2+"collSolid=CollisionRay()\n")
                out_file.write(i2+"collSolid.setOrigin(point)\n")
                out_file.write(i2+"collSolid.setDirection(vector)\n")

                pass
            else:
                 print("Invalid Collision Node: " + nodetype)
            out_file.write("\n")


            out_file.write(i2+"self." + collnodeS + "_Node" + "=CollisionNode(\""+collnodeS+"\")\n")
            out_file.write(i2+"self." + collnodeS + "_Node" + ".addSolid(collSolid)\n")
            out_file.write(i2+"base.cTrav.addCollider(self." + collnodeS + "_Node,self.CollisionHandler)\n")
            out_file.write("\n")




        ####################################################################################################################################################
        # Light Saving
        ####################################################################################################################################################
        out_file.write(i2+"##########################################################################################################\n")
        out_file.write(i2+"# Code for Lighting\n")
        out_file.write(i2+"# To manipulated lights:\n")
        out_file.write(i2+"# Manipulate the light node in theScene.LightNodes[\'Light_Name\']\n")
        out_file.write(i2+"##########################################################################################################\n\n")

        LightList=AllScene.lightManager.getLightNodeList()
        for light in LightList:
                 type = light.getType()
                 if type == 'ambient':
                     out_file.write(i2+"# Ambient Light\n")
                     out_file.write (i2+ "self.ambientCount += 1\n")
                     out_file.write (i2+ "alight = AmbientLight(\'"+ light.getName() +"\')\n")
                     out_file.write (i2+ "alight.setColor(VBase4("+ str(light.getLightColor().getX())+ "," + str(light.getLightColor().getY())+ "," + str(light.getLightColor().getZ()) + "," + str(light.getLightColor().getW()) + "))\n")
                     out_file.write (i2+ "self.lightAttrib=self.lightAttrib.addLight(alight)\n")
                     out_file.write (i2+ "self."+light.getName()+"= render.attachNewNode(alight)\n")
                     out_file.write (i2+ "self."+light.getName()+".setTag(\"Metadata\",\"" + light.getTag("Metadata") + "\")\n")
                     out_file.write (i2+ "self.LightDict[\'" + light.getName() + "\']=alight\n")
                     out_file.write (i2+ "self.LightTypes[\'" + light.getName() + "\']=\'" + type + "\'\n")
                     out_file.write (i2+ "self.LightNodes[\'" + light.getName() + "\']=self." + light.getName() + "\n")
                     out_file.write ("\n")
                 elif type == 'directional':
                     out_file.write(i2+"# Directional Light\n")
                     out_file.write (i2+ "self.directionalCount += 1\n")
                     out_file.write (i2+ "alight = DirectionalLight(\'"+ light.getName() + "\')\n")
                     out_file.write (i2+ "alight.setColor(VBase4("+ str(light.getLightColor().getX())+ "," + str(light.getLightColor().getY())+ "," + str(light.getLightColor().getZ()) + "," + str(light.getLightColor().getW()) + "))\n")
                     #out_file.write (i2+ "alight.setDirection(Vec3("+ str(light.getH())+ "," + str(light.getP())+ "," + str(light.getR()) + "))\n")
                     #out_file.write (i2+ "alight.setPoint(Point3(" + str(light.getX()) + "," + str(light.getY()) + "," + str(light.getZ()) + "))\n")
                     out_file.write (i2+ "alight.setSpecularColor(Vec4(" + str(light.getSpecColor().getX()) + "," + str(light.getSpecColor().getY()) + "," + str(light.getSpecColor().getZ()) + "," + str(light.getSpecColor().getW()) + "))\n")
                     out_file.write (i2+ "self.lightAttrib=self.lightAttrib.addLight(alight)\n")
                     out_file.write (i2+ "self."+light.getName()+ "= render.attachNewNode(alight)\n")
                     out_file.write (i2+ "self."+light.getName()+ ".setPos(Point3(" + str(light.getX()) + "," + str(light.getY()) + "," + str(light.getZ()) + "))\n")
                     out_file.write (i2+ "self."+light.getName()+ ".setHpr(Vec3("+ str(light.getH())+ "," + str(light.getP())+ "," + str(light.getR()) + "))\n")
                     out_file.write (i2+ "self."+light.getName()+ ".setTag(\"Metadata\",\"" + light.getTag("Metadata") + "\")\n")
                     #out_file.write (i2+ "alight.setPos
                     out_file.write (i2+ "self.LightDict[\'" + light.getName() + "\']=alight\n")
                     out_file.write (i2+ "self.LightTypes[\'" + light.getName() + "\']=\'" + type + "\'\n")
                     out_file.write (i2+ "self.LightNodes[\'" + light.getName() + "\']=self." + light.getName()  + "\n")
                     out_file.write ("\n")
                 elif type == 'point':
                     out_file.write(i2+"# Point Light\n")
                     out_file.write (i2+ "self.pointCount += 1\n")
                     out_file.write (i2+ "alight = PointLight(\'"+ light.getName() +"\')\n")
                     out_file.write (i2+ "alight.setColor(VBase4("+ str(light.getLightColor().getX())+ "," + str(light.getLightColor().getY())+ "," + str(light.getLightColor().getZ()) + "," + str(light.getLightColor().getW()) + "))\n")
                     #out_file.write (i2+ "alight.setPoint(Point3(" + str(light.getX()) + "," + str(light.getY()) + "," + str(light.getZ()) + "))\n")
                     out_file.write (i2+ "alight.setSpecularColor(Vec4(" + str(light.getSpecColor().getX()) + "," + str(light.getSpecColor().getY()) + "," + str(light.getSpecColor().getZ()) + "," + str(light.getSpecColor().getW()) + "))\n")
                     out_file.write (i2+ "alight.setAttenuation(Vec3("+ str(light.getAttenuation().getX()) + "," + str(light.getAttenuation().getY()) + "," + str(light.getAttenuation().getZ()) + "))\n")
                     out_file.write (i2+ "self.lightAttrib=self.lightAttrib.addLight(alight)\n")
                     out_file.write (i2+ "self."+light.getName()+ "= render.attachNewNode(alight)\n")
                     out_file.write (i2+ "self."+light.getName()+ ".setTag(\"Metadata\",\"" + light.getTag("Metadata") + "\")\n")
                     out_file.write (i2+ "self."+light.getName()+ ".setPos(Point3(" + str(light.getX()) + "," + str(light.getY()) + "," + str(light.getZ()) + "))\n")
                     out_file.write (i2+ "self.LightDict[\'" + light.getName() + "\']=alight\n")
                     out_file.write (i2+ "self.LightTypes[\'" + light.getName() + "\']=\'" + type + "\'\n")
                     out_file.write (i2+ "self.LightNodes[\'" + light.getName() + "\']=self." + light.getName() +  "\n")
                     out_file.write ("\n")
                 elif type == 'spot':
                     out_file.write(i2+"# Spot Light\n")
                     out_file.write (i2+ "self.spotCount += 1\n")
                     out_file.write (i2+ "alight = Spotlight(\'"+ light.getName() + "\')\n")
                     out_file.write (i2+ "alight.setColor(VBase4("+ str(light.getLightColor().getX())+ "," + str(light.getLightColor().getY())+ "," + str(light.getLightColor().getZ()) + "," + str(light.getLightColor().getW()) + "))\n")
                     out_file.write (i2+ "alens = PerspectiveLens()\n")
                     out_file.write (i2+ "alight.setLens(alens)\n")
                     out_file.write (i2+ "alight.setSpecularColor(Vec4(" + str(light.getSpecColor().getX()) + "," + str(light.getSpecColor().getY()) + "," + str(light.getSpecColor().getZ()) + "," + str(light.getSpecColor().getW()) + "))\n")
                     out_file.write (i2+ "alight.setAttenuation(Vec3("+ str(light.getAttenuation().getX()) + "," + str(light.getAttenuation().getY()) + "," + str(light.getAttenuation().getZ()) + "))\n")
                     out_file.write (i2+ "alight.setExponent(" +str(light.getExponent()) +")\n")
                     out_file.write (i2+ "self.lightAttrib=self.lightAttrib.addLight(alight)\n")
                     out_file.write (i2+ "self."+light.getName()+ "= render.attachNewNode(alight)\n")
                     out_file.write (i2+ "self."+light.getName()+ ".setTag(\"Metadata\",\"" + light.getTag("Metadata") + "\")\n")
                     out_file.write (i2+ "self."+light.getName()+ ".setPos(Point3(" + str(light.getX()) + "," + str(light.getY()) + "," + str(light.getZ()) + "))\n")
                     out_file.write (i2+ "self."+light.getName()+ ".setHpr(Vec3("+ str(light.getH())+ "," + str(light.getP())+ "," + str(light.getR()) + "))\n")
                     out_file.write (i2+ "self.LightDict[\'" + light.getName() + "\']=alight\n")
                     out_file.write (i2+ "self.LightTypes[\'" + light.getName() + "\']=\'" + type + "\'\n")
                     out_file.write (i2+ "self.LightNodes[\'" + light.getName() + "\']=self." + light.getName() + "\n")
                     out_file.write ("\n")
                 else:
                     out_file.write (i2+ "print \'Invalid light type\'")
                     out_file.write (i2+ "return None")
                 out_file.write("\n")



        ####################################################################################################################################################
        # Enable Lighting
        ####################################################################################################################################################

        out_file.write(i2+ "# Enable Ligthing\n")
        out_file.write(i2+ "render.node().setAttrib(self.lightAttrib)\n")
        out_file.write("\n")



        ####################################################################################################################################################
        # Initialize Particles for non scene editor mode
        ####################################################################################################################################################

        out_file.write(i2+"# Load Particle Effects. The parameters to this function are to allow us to use our modified versions of the Particle Effects modules when loading this file with the level editor\n")
        out_file.write(i2+"self.starteffects(self.loadmode,self.seParticleEffect,self.seParticles)\n")
        out_file.write("\n")

        ####################################################################################################################################################
        # Save Camera Settings
        ####################################################################################################################################################

        out_file.write("\n")
        out_file.write(i2+ "# Save Camera Settings\n")
        out_file.write(i2+ "camera.setX(" + str(camera.getX()) + ")\n")
        out_file.write(i2+ "camera.setY(" + str(camera.getY()) + ")\n")
        out_file.write(i2+ "camera.setZ(" + str(camera.getZ()) + ")\n")
        out_file.write(i2+ "camera.setH(" + str(camera.getH()) + ")\n")
        out_file.write(i2+ "camera.setP(" + str(camera.getP()) + ")\n")
        out_file.write(i2+ "camera.setR(" + str(camera.getR()) + ")\n")
        out_file.write(i2+ "camera.getChild(0).node().getLens().setNear(" + str(camera.getChild(0).node().getLens().getNear()) + ")\n")
        out_file.write(i2+ "camera.getChild(0).node().getLens().setFar(" + str(camera.getChild(0).node().getLens().getFar()) + ")\n")
        out_file.write(i2+ "camera.getChild(0).node().getLens().setFov(VBase2(%.5f,%.5f))\n"% (camera.getChild(0).node().getLens().getHfov(),camera.getChild(0).node().getLens().getVfov()))
        FilmSize=camera.getChild(0).node().getLens().getFilmSize()
        out_file.write(i2+ "camera.getChild(0).node().getLens().setFilmSize(%.3f,%.3f)\n"%(FilmSize.getX(),FilmSize.getY()))
        out_file.write(i2+ "camera.getChild(0).node().getLens().setFocalLength(" + str(camera.getChild(0).node().getLens().getFocalLength()) + ")\n")
        out_file.write(i2+ "camera.setTag(\"Metadata\",\"" + camera.getTag("Metadata") + "\")\n")
        out_file.write(i2+ "camera.reparentTo(render)\n")
        out_file.write(i2+ "base.disableMouse()\n")
        self.bgColor=base.getBackgroundColor()
        out_file.write(i2+ "base.setBackgroundColor(%.3f,%.3f,%.3f)\n"%(self.bgColor.getX(),self.bgColor.getY(),self.bgColor.getZ()))
        out_file.write("\n")


        ####################################################################################################################################################
        # Mopath Saving
        ####################################################################################################################################################

        out_file.write(i2+"##########################################################################################################\n")
        out_file.write(i2+"# Motion Paths\n")
        out_file.write(i2+"# Using Mopaths:\n")
        out_file.write(i2+"# theScene.curveIntervals[0].start() or .loop() will play curve with index 0\n")
        out_file.write(i2+"##########################################################################################################\n\n")

        for node in AllScene.curveDict:
            curveCollection=AllScene.curveDict[node]
            curvenumber=0
            for curve in curveCollection:
                filestring=dirname+ "\\" + str(node)+"_curve_"+str(curvenumber)+".egg"
                f=Filename.fromOsSpecific(filestring)
                #filestring=f.getFullpath()# The old absolute path way
                filestring=f.getBasename() # The new relative path way
                curve.writeEgg(f)
                out_file.write(i2+"m=Mopath.Mopath()\n")

                out_file.write(i2+"if(self.loadmode==1):\n")
                out_file.write(i2+i1+"m.loadFile(\"" + self.savepath +"/"+ filestring + "\")\n") # If just normally executed
                out_file.write(i2+"else:\n")
                out_file.write(i2+i1+"m.loadFile(self.executionpath + \"/"+ filestring + "\")\n") # If being invoked by level Ed

                out_file.write(i2+"mp=MopathInterval(m,self." + str(node) + ")\n")
                out_file.write(i2+"self.curveIntervals.append(mp)\n")

                out_file.write(i2+"if(self.loadmode==1):\n")
                out_file.write(i2+i1+"self.curveRefColl.append(\"" + self.savepath +"/"+ filestring +"\")\n")
                out_file.write(i2+"else:\n")
                out_file.write(i2+i1+"self.curveRefColl.append(self.executionpath + \"/"+ filestring +"\")\n")

                curvenumber=curvenumber+1
            out_file.write(i2+"self.curveIntervalsDict[\"" + str(node) + "\"]=self.curveIntervals\n")
            out_file.write(i2+"self.curveDict[\"" + str(node) + "\"]=self.curveRefColl\n")


        ####################################################################################################################################################
        # Lets do all the reparenting here so as to make sure everything that needed to load was loaded
        ####################################################################################################################################################


        out_file.write(i2+"##########################################################################################################\n")
        out_file.write(i2+"# Reparenting\n")
        out_file.write(i2+"# A final pass is done on setting all the scenegraph hierarchy after all objects are laoded\n")
        out_file.write(i2+"##########################################################################################################\n\n")

        for model in AllScene.ModelDic:
            modelS=str(model)
            parent=AllScene.ModelDic[model].getParent().getName()
            if(parent=="render" or parent=="camera"):
                out_file.write(i2+ "self."+ modelS + ".reparentTo(" + parent + ")\n")
            else:
                if(parent in AllScene.particleDict):
                    out_file.write(i2+ "self."+ modelS + ".reparentTo(self." + parent + ".getEffect())\n")
                else:
                    out_file.write(i2+ "self."+ modelS + ".reparentTo(self." + parent + ")\n")
            out_file.write(i2+ "self.ModelDic[\'" + modelS + "\']=self." + AllScene.ModelDic[model].getName()+"\n")
            out_file.write(i2+"\n")

        for dummy in AllScene.dummyDict:
            dummyS=str(dummy)
            parent=AllScene.dummyDict[dummy].getParent().getName()
            if(parent=="render" or parent=="camera"):
                out_file.write(i2+ "self."+ dummyS + ".reparentTo(" + parent + ")\n")
            else:
                if(parent in AllScene.particleDict):
                    out_file.write(i2+ "self."+ dummyS + ".reparentTo(self." + parent + ".getEffect())\n")
                else:
                    out_file.write(i2+ "self."+ dummyS + ".reparentTo(self." + parent + ")\n")

            out_file.write(i2+ "self.dummyDict[\'" + dummyS + "\']=self." + AllScene.dummyDict[dummy].getName()+"\n")
            out_file.write(i2+"\n")

        for actor in AllScene.ActorDic:
            actorS=str(actor)
            parent=AllScene.ActorDic[actor].getParent().getName()
            if(parent=="render" or parent=="camera"):
                out_file.write(i2+ "self."+ actorS + ".reparentTo(" + parent + ")\n")
            else:
                if(parent in AllScene.particleDict):
                    out_file.write(i2+ "self."+ actorS + ".reparentTo(self." + parent + ".getEffect())\n")
                else:
                    out_file.write(i2+ "self."+ actorS + ".reparentTo(self." + parent + ")\n")

            out_file.write(i2+ "self.ActorDic[\'" + actorS + "\']=self." + AllScene.ActorDic[actor].getName()+"\n")
            out_file.write(i2+"\n")


        for collnode in AllScene.collisionDict:
            collnodeS=str(collnode)
            solid=AllScene.collisionDict[collnode].node().getSolid(0)
            nodetype=solid.getType().getName()
            parentname=AllScene.collisionDict[collnode].getParent().getName()
            if(parentname=="render" or parentname =="camera"):
                out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"]="+ parentname + ".attachNewNode(self." + collnodeS + "_Node)\n")
            else:
                #Manakel 2/12/2005: parent replaced by parent Name but why Parent name in partice and parent for other objects?
                if(parentname in AllScene.particleDict):
                    out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"]=self."+ parentname + "getEffect().attachNewNode(self." + collnodeS + "_Node)\n")
                else:
                    out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"]=self."+ parentname + ".attachNewNode(self." + collnodeS + "_Node)\n")
            dictelem=AllScene.collisionDict[collnode]
            out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"].setPosHprScale(%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f)\n"%(dictelem.getX(),dictelem.getY(),dictelem.getZ(),dictelem.getH(),dictelem.getP(),dictelem.getR(),dictelem.getSx(),dictelem.getSy(),dictelem.getSz()))
            out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"].setTag(\"Metadata\",\"" + AllScene.collisionDict[collnode].getTag("Metadata") + "\")\n")
            out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"].show()\n")
            if(nodetype=="CollisionPolygon"): #Save Collison Polygon... the reason we need to use setTag here is because there is no inbuilt way of saving transforms for collision polys

                ax=float(AllScene.collisionDict[collnode].getTag("A_X"))
                ay=float(AllScene.collisionDict[collnode].getTag("A_Y"))
                az=float(AllScene.collisionDict[collnode].getTag("A_Z"))

                bx=float(AllScene.collisionDict[collnode].getTag("B_X"))
                by=float(AllScene.collisionDict[collnode].getTag("B_Y"))
                bz=float(AllScene.collisionDict[collnode].getTag("B_Z"))

                cx=float(AllScene.collisionDict[collnode].getTag("C_X"))
                cy=float(AllScene.collisionDict[collnode].getTag("C_Y"))
                cz=float(AllScene.collisionDict[collnode].getTag("C_Z"))

                out_file.write(i2+"pointA=Point3(%.3f,%.3f,%.3f)\n"%(ax,ay,az))
                out_file.write(i2+"pointB=Point3(%.3f,%.3f,%.3f)\n"%(bx,by,bz))
                out_file.write(i2+"pointC=Point3(%.3f,%.3f,%.3f)\n"%(cx,cy,cz))

                out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"].setTag('A_X','%f'%pointA.getX())\n")
                out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"].setTag('A_Y','%f'%pointA.getY())\n")
                out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"].setTag('A_Z','%f'%pointA.getZ())\n")

                out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"].setTag('B_X','%f'%pointB.getX())\n")
                out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"].setTag('B_Y','%f'%pointB.getY())\n")
                out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"].setTag('B_Z','%f'%pointB.getZ())\n")

                out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"].setTag('C_X','%f'%pointC.getX())\n")
                out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"].setTag('C_Y','%f'%pointC.getY())\n")
                out_file.write(i2+"self.collisionDict[\"" + collnodeS + "\"].setTag('C_Z','%f'%pointC.getZ())\n")
            out_file.write(i2+"\n")


        for effect in AllScene.particleDict:
            parent=AllScene.particleNodes[effect].getParent().getName()
            if(parent=="render" or parent=="camera"):
                out_file.write(i2+"self.particleDict[\""+ str(effect) +"\"].reparentTo("  + parent + ")\n")
            else:
                out_file.write(i2+"self.particleDict[\""+ str(effect) +"\"].reparentTo(self."  + parent + ")\n")
            out_file.write(i2+"\n")


        ####################################################################################################################################################
        # Particle Saving
        ####################################################################################################################################################
        out_file.write("\n")
        out_file.write(i2+"##########################################################################################################\n")
        out_file.write(i2+"# Particle Effects\n")
        out_file.write(i2+"# Using Particles:\n")
        out_file.write(i2+"# theScene.enableeffect(\"Effect_Name\")\n")
        out_file.write(i2+"##########################################################################################################\n\n")
        out_file.write(i1+"def starteffects(self,mode,seParticleEffect=None,seParticles=None):\n")
        for effect in AllScene.particleDict:
            effectS=str(effect)
            out_file.write(i2+ "self." + effectS + "=" + effectS + "(mode,seParticleEffect,seParticles)\n")
            out_file.write(i2+ "effect=self."+ effectS + ".getEffect()\n")
            out_file.write(i2+ "self.particleDict[\"" + effectS + "\"]=effect\n")
            out_file.write(i2+ "effect.reparentTo(render)\n")
            thenode=AllScene.particleNodes[effect]
            out_file.write(i2+ "self.particleDict[\"" + effectS + "\"].setPosHprScale(%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f)\n"%(thenode.getX(),thenode.getY(),thenode.getZ(),thenode.getH(),thenode.getP(),thenode.getR(),thenode.getSx(),thenode.getSy(),thenode.getSz()))
            out_file.write("\n")
        out_file.write(i2+"return\n")
        out_file.write("\n")
        out_file.write(i1+"def enableeffect(self,effect_name):\n")
        out_file.write(i2+"self.particleDict[effect_name].enable()\n")
        out_file.write(i2+"return\n")
        out_file.write("\n")
        out_file.write(i1+"def disableeffect(self,effect_name):\n")
        out_file.write(i2+"self.particleDict[effect_name].disable()\n")
        out_file.write(i2+"return\n")
        out_file.write("\n")


        ####################################################################################################################################################
        # Animation Blending Methods
        ####################################################################################################################################################
        out_file.write("\n")
        out_file.write(i2+"##########################################################################################################\n")
        out_file.write(i2+"# Animation Blending\n")
        out_file.write(i2+"# Using blending:\n")
        out_file.write(i2+"# theScene.playBlendAnim(actor,blendname)\n")
        out_file.write(i2+"# theScene.stopBlendAnim(actor,blendname)\n")
        out_file.write(i2+"# theScene.changeBlendAnim(actor,blendname,blend_amount)\n")
        out_file.write(i2+"##########################################################################################################\n\n")
        out_file.write(i1+"def playBlendAnim(self,actor,blendName,loop=0):\n")
        out_file.write(i2+"actor.enableBlend()\n")
        out_file.write(i2+"blendDicts=self.blendAnimDict[actor.getName()]\n")
        out_file.write(i2+"blendList=blendDicts[blendName]\n")
        out_file.write(i2+"actor.setControlEffect(blendList[0],blendList[2])\n")
        out_file.write(i2+"actor.setControlEffect(blendList[1],1.0-blendList[2])\n")
        out_file.write(i2+"if(loop):\n")
        out_file.write(i2+i1+"actor.loop(blendList[0])\n")
        out_file.write(i2+i1+"actor.loop(blendList[1])\n")
        out_file.write(i2+"else:\n")
        out_file.write(i2+i1+"actor.start(blendList[0])\n")
        out_file.write(i2+i1+"actor.start(blendList[1])\n")
        out_file.write("\n")


        out_file.write(i1+"def stopBlendAnim(self,actor,blendName):\n")
        out_file.write(i2+"blendDicts=self.blendAnimDict[actor.getName()]\n")
        out_file.write(i2+"blendList=blendDicts[blendName]\n")
        out_file.write(i2+"actor.stop(blendList[0])\n")
        out_file.write(i2+"actor.stop(blendList[1])\n")
        out_file.write("\n")

        out_file.write(i1+"def changeBlending(self,actor,blendName,blending):\n")
        out_file.write(i2+"blendDicts=self.blendAnimDict[actor.getName()]\n")
        out_file.write(i2+"blendList=blendDicts[blendName]\n")
        out_file.write(i2+"blendList[2]=blending\n")
        out_file.write(i2+"self.blendAnimDict[actor.getName()]={blendName:[blendList[0],blendList[1],blending]}\n")
        out_file.write("\n")



        ####################################################################################################################################################
        # Hide and Show Methods
        ####################################################################################################################################################

        out_file.write("\n")
        out_file.write(i2+"##########################################################################################################\n")
        out_file.write(i2+"# Hide and Show Methods\n")
        out_file.write(i2+"# These will help you hide/show dummies, collision solids, effect nodes etc.\n")
        out_file.write(i2+"##########################################################################################################\n\n")


        out_file.write("\n")
        out_file.write(i1+"def hideDummies(self):\n")
        out_file.write("\n")
        out_file.write(i2+"for dummy in self.dummyDict:\n")
        out_file.write(i2+i1+"self.dummyDict[dummy].reparentTo(hidden)\n")


        out_file.write("\n")
        out_file.write(i1+"def hideCollSolids(self):\n")
        out_file.write("\n")
        out_file.write(i2+"for collSolid in self.collisionDict:\n")
        out_file.write(i2+i1+"self.collisionDict[collSolid].hide()\n")


        out_file.write("\n")
        out_file.write(i1+"def hideEffectNodes(self):\n")
        out_file.write("\n")
        out_file.write(i2+"for effectnode in self.particleNodes:\n")
        out_file.write(i2+i1+"self.particleNodes[effectnode].hide()\n")


        out_file.write("\n")
        out_file.write(i1+"def showDummies(self):\n")
        out_file.write("\n")
        out_file.write(i2+"for dummy in self.dummyDict:\n")
        out_file.write(i2+i1+"self.dummyDict[dummy].reparentTo(hidden)\n")


        out_file.write("\n")
        out_file.write(i1+"def showCollSolids(self):\n")
        out_file.write("\n")
        out_file.write(i2+"for collSolid in self.collisionDict:\n")
        out_file.write(i2+i1+"self.collisionDict[collSolid].show()\n")


        out_file.write("\n")
        out_file.write(i1+"def showEffectNodes(self):\n")
        out_file.write("\n")
        out_file.write(i2+"for effectnode in self.particleNodes:\n")
        out_file.write(i2+i1+"self.particleNodes[effectnode].show()\n\n")


        ##########################################################################################################
        # Saving Particle Parameters as a Class
        ##########################################################################################################

        out_file.write("\n")
        out_file.write(i2+"##########################################################################################################\n")
        out_file.write(i2+"# Particle Effects\n")
        out_file.write(i2+"# This is where effect parameters are saved in a class\n")
        out_file.write(i2+"# The class is then instantiated in the starteffects method and appended to the dictionaries\n")
        out_file.write(i2+"##########################################################################################################\n\n")

        for effect in AllScene.particleDict:

            out_file.write("\n\n")
            out_file.write("class " + str(effect) + ":\n")
            out_file.write(i1+"def __init__(self,mode=1,seParticleEffect=None,seParticles=None):\n")
            out_file.write(i2+"if(mode==0):\n")
            out_file.write(i2+i1+"self.effect=seParticleEffect.ParticleEffect()\n")
            out_file.write(i2+"else:\n")
            out_file.write(i2+i1+"self.effect=ParticleEffect.ParticleEffect()\n")
            AllScene.particleDict[effect].AppendConfig(out_file)
            #out_file.write(i2+"return self.effect\n")
            out_file.write("\n\n")
            out_file.write(i1+"def starteffect(self):\n")
            out_file.write(i2+"pass\n")
            out_file.write("\n\n")
            out_file.write(i1+"def stopeffect(self):\n")
            out_file.write(i2+"pass\n\n")
            out_file.write(i1+"def getEffect(self):\n")
            out_file.write(i2+"return self.effect\n\n")



        #Un-comment the lines below to make this a stand-alone file
        #out_file.write("Scene=SavedScene()\n")
        #out_file.write("run()\n")

        out_file.close()


