from DirectButton import *
from DirectLabel import *

class DirectCheckButton(DirectButton):
    """
    DirectCheckButton(parent) - Create a DirectGuiWidget which responds
    to mouse clicks by setting a state of on or off and execute a callback
    function (passing that state through) if defined
    """
    def __init__(self, parent = aspect2d, **kw):
        # Inherits from DirectButton
        # A Direct Frame can have:
        # - A background texture (pass in path to image, or Texture Card)
        # - A midground geometry item (pass in geometry)
        # - A foreground text Node (pass in text string or Onscreen Text)
        # For a direct button:
        # Each button has 4 states (ready, press, rollover, disabled)
        # The same image/geom/text can be used for all four states or each
        # state can have a different text/geom/image
        # State transitions happen automatically based upon mouse interaction
        # Responds to click event and calls command if None
        
        optiondefs = (
            ('indicatorValue', 0, self.setIndicatorValue),
            # boxBorder defines the space created around the check box
            ('boxBorder', 0, None),
            # boxPlacement maps left, above, right, below
            ('boxPlacement', 'left', None),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)
        # Initialize superclasses
        DirectButton.__init__(self, parent)
        
        self.indicator = self.createcomponent("indicator", (), None,
                                              DirectLabel, (self,),
                                              numStates = 2,
                                              state = 'disabled',
                                              text = ('X' , 'X'),
                                              relief = 'sunken',
                                              )
       
        # Call option initialization functions
        self.initialiseoptions(DirectCheckButton)
        # After initialization with X giving it the correct size, put back space 
        self.indicator['text'] = (' ', '*')
        self.indicator['text_pos'] = (0,-.17)

        
    # Override the resetFrameSize of DirectGuiWidget inorder to provide space for label
    def resetFrameSize(self):
        self.setFrameSize(fClearFrame = 1)
        
    def setFrameSize(self, fClearFrame = 0):
        
        if self['frameSize']:
            # Use user specified bounds
            self.bounds = self['frameSize']
        else:
            # Use ready state to compute bounds
            frameType = self.frameStyle[0].getType()
            if fClearFrame and (frameType != PGFrameStyle.TNone):
                self.frameStyle[0].setType(PGFrameStyle.TNone)
                self.guiItem.setFrameStyle(0, self.frameStyle[0])
                # To force an update of the button
                self.guiItem.getStateDef(0)
            # Clear out frame before computing bounds
            self.getBounds()
            # Restore frame style if necessary
            if (frameType != PGFrameStyle.TNone):
                self.frameStyle[0].setType(frameType)
                self.guiItem.setFrameStyle(0, self.frameStyle[0])

            # Ok, they didn't set specific bounds,
            #  let's add room for the label indicator
            #  get the difference in height
            
            diff = self.indicator.getHeight() + (2*self['boxBorder']) - (self.bounds[3] - self.bounds[2])
            # If background is smaller then indicator, enlarge background
            if diff > 0:
                if self['boxPlacement'] == 'left':            #left
                    self.bounds[0] += -(self.indicator.getWidth() + (2*self['boxBorder']))
                    self.bounds[3] += diff/2
                    self.bounds[2] -= diff/2
                elif self['boxPlacement'] == 'below':          #below
                    self.bounds[2] += -(self.indicator.getHeight() + (2*self['boxBorder']))
                elif self['boxPlacement'] == 'right':          #right
                    self.bounds[1] += self.indicator.getWidth() + (2*self['boxBorder'])
                    self.bounds[3] += diff/2
                    self.bounds[2] -= diff/2
                else:                                    #above
                    self.bounds[3] += self.indicator.getHeight() + (2*self['boxBorder'])

            # Else make space on correct side for indicator
            else:
                if self['boxPlacement'] == 'left':            #left
                    self.bounds[0] += -(self.indicator.getWidth() + (2*self['boxBorder']))
                elif self['boxPlacement'] == 'below':          #below
                    self.bounds[2] += -(self.indicator.getHeight() + (2*self['boxBorder']))
                elif self['boxPlacement'] == 'right':          #right
                    self.bounds[1] += self.indicator.getWidth() + (2*self['boxBorder'])
                else:                                    #above
                    self.bounds[3] += self.indicator.getHeight() + (2*self['boxBorder'])

        # Set frame to new dimensions
        self.guiItem.setFrame(self.bounds[0], self.bounds[1],
                              self.bounds[2], self.bounds[3])  #3 is top border!!

        # If they didn't specify a position, put it in the center of new area
        if not self.indicator['pos']:
            bbounds = self.bounds
            lbounds = self.indicator.bounds
            newpos = [0,0,0]

            if self['boxPlacement'] == 'left':            #left
                newpos[0] += bbounds[0]-lbounds[0] + self['boxBorder']
                dropValue = (bbounds[3]-bbounds[2]-lbounds[3]+lbounds[2])/2 + self['boxBorder']
                newpos[2] += bbounds[3]-lbounds[3] + self['boxBorder'] - dropValue
            elif self['boxPlacement'] == 'right':            #right
                newpos[0] += bbounds[1]-lbounds[1] - self['boxBorder']
                dropValue = (bbounds[3]-bbounds[2]-lbounds[3]+lbounds[2])/2 + self['boxBorder']
                newpos[2] += bbounds[3]-lbounds[3] + self['boxBorder'] - dropValue
            elif self['boxPlacement'] == 'above':            #above
                newpos[2] += bbounds[3]-lbounds[3] - self['boxBorder']
            else:                                      #below
                newpos[2] += bbounds[2]-lbounds[2] + self['boxBorder']

            self.indicator.setPos(newpos[0],newpos[1],newpos[2])

        
    def commandFunc(self, event):
        self['indicatorValue'] = 1 - self['indicatorValue']

        if self['command']:
            # Pass any extra args to command
            apply(self['command'], [self['indicatorValue']] + self['extraArgs'])
            
    def setIndicatorValue(self):
        self.component('indicator').guiItem.setState(self['indicatorValue'])
        







