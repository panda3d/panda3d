###########################################################
# Class to create and maintain a scrolled list
# that can be embedded in a LevelAttribute instance
###########################################################

from direct.gui.DirectGui import *
from toontown.toonbase import ToontownGlobals

class ScrollMenu:
    def __init__(self, nodePath, textList):
        self.action = None              #Call back fucntion
        self.textList = textList
        
        self.parent = nodePath
        self.frame = None

        self.initialState = None        # To maintain backward compatibility
        
    def createScrolledList(self):
        # First create a frame in which direct elements maybe placed
        self.frame = DirectFrame(scale = 1.1, relief = 1.0,
                                 frameSize = (-0.5,0.2,-0.05,0.59),
                                 frameColor = (0.737, 0.573, 0.345, 0.000))

        numItemsVisible = 4
        itemHeight = 0.08

        myScrolledList = DirectScrolledList(
            decButton_pos= (0.35, 0, 0.53),
            decButton_text = "Dec",
            decButton_text_scale = 0.04,
            decButton_borderWidth = (0.005, 0.005),

            incButton_pos= (0.35, 0, -0.02),
            incButton_text = "Inc",
            incButton_text_scale = 0.04,
            incButton_borderWidth = (0.005, 0.005),

            frameSize = (0.0, 0.8, -0.05, 0.59),
            frameColor = (0,0,1,0.5),
            pos = (-0.5, 0, 0),
            items = [],
            numItemsVisible = numItemsVisible,
            forceHeight = itemHeight,
            itemFrame_frameSize = (-0.35, 0.35, -0.37, 0.11),
            itemFrame_pos = (0.4, 0, 0.4),
        )
        
        for t in self.textList:
            myScrolledList.addItem(DirectButton(text = (t, t, t),
                                    text_scale = 0.05, command = self.__selected,
                                    extraArgs = [t], relief = None,
                                    # text_font = ToontownGlobals.getSignFont(),
                                    text0_fg = (0.152, 0.750, 0.258, 1),
                                    text1_fg = (0.152, 0.750, 0.258, 1),
                                    text2_fg = (0.977, 0.816, 0.133, 1), ))
        myScrolledList.reparentTo(self.frame)
        
        #An exit button
        b1 = DirectButton(parent = self.frame, text  = "Exit",
                  text_scale=0.05, borderWidth = (0.01, 0.01),
                  relief=1, command = self.__hide)
        b1.setPos(0.15, 0, -0.025)
        
        self.frame.reparentTo(self.parent)
        
        
    def __selected(self, text):
        if(self.action):
            self.action(text)
            
    def __hide(self):
        self.frame.reparentTo(self.parent)


    #######################################################
    # Functions that allow compaitibility with the
    # existing architecture that is tied into pie menu's
    #######################################################
    def spawnPieMenuTask(self):
         # Where did the user press the button?
        originX = base.direct.dr.mouseX
        originY = base.direct.dr.mouseY

        # Pop up menu
        self.frame.reparentTo(aspect2d)
        self.frame.setPos(originX, 0.0, originY)
        
    def removePieMenuTask(self):
        pass
    
    def setInitialState(self, state):
        self.initialState = state

    def getInitialState(self):
        return self.initialState