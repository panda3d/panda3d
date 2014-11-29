'''
COLLECTION OF SOME DIRECT GUI EXTENSION CLASSES
'''
__all__ = ['myDirectButton','myDirectCheckButton','myEntry']


from pandac.PandaModules import TextNode
from direct.interval.IntervalGlobal import *
from direct.gui.DirectGui import *
import types
# collidable-button system
from PolyButton import *

MODE_a2dEntryFocus='DirectEntryFocus'

# universal in-focus custom DirectEntry.
# both this extension module and the main script need to modify this,
# so store it in builtins dictionary.
__builtins__['FOCUS_Entry'] = None


class myDirectButton(DirectButton):
  '''
  CUSTOM DIRECT BUTTON & DIRECT CHECK BUTTON :
  basically only to make them aware of the custom DirectEntry (myEntry class),
  so while there is an in-focus myEntry, and user presses any custom button,
  the in-focus myEntry's value would be restored to it's previous value, and
  the focus would be dropped.
  '''
  def __init__(self,*args,**kw):
      DirectButton.__init__(self,*args,**kw)
      self.initialiseoptions(myDirectButton)
      # for each command button press (LMB &/ MMB &/ RMB) event,
      # hook a function to restore any in-focus myEntry instance value
      # and drop the focus
      for b in self['commandButtons']:
          self.bind(DGG.B1PRESS, self.dropEntryFocus)

  def dropEntryFocus(self,e):
      if FOCUS_Entry!=None:
         Sequence(
            Wait(.05),
            Func(FOCUS_Entry.destroyExpectedValueType),
            Func(FOCUS_Entry.restoreEntry),
            Func(FOCUS_Entry.setProp,'focus',0)
         ) .start()



class myDirectCheckButton(DirectCheckButton):
  '''
  CUSTOM DIRECT BUTTON & DIRECT CHECK BUTTON :
  basically only to make them aware of the custom DirectEntry (myEntry class),
  so while there is an in-focus myEntry, and user presses any custom button,
  the in-focus myEntry's value would be restored to it's previous value, and
  the focus would be dropped.
  '''
  def __init__(self,*args,**kw):
      DirectCheckButton.__init__(self,*args,**kw)
      self.initialiseoptions(myDirectCheckButton)
      # for each command button press (LMB &/ MMB &/ RMB) event,
      # hook a function to restore any in-focus myEntry value
      # and drop the focus
      for b in self['commandButtons']:
          event='press-mouse%s-%s' %(b+1,self.guiId)
          if not self.isAccepting(event):
             self.accept(event, self.dropEntryFocus)

  def dropEntryFocus(self,e):
      if FOCUS_Entry!=None:
         Sequence(
            Wait(.05),
            Func(FOCUS_Entry.destroyExpectedValueType),
            Func(FOCUS_Entry.restoreEntry),
            Func(FOCUS_Entry.setProp,'focus',0)
         ) .start()




class myEntry(DirectEntry):
  ''''
  CUSTOM DIRECT ENTRY :
  Features :
  1. frame & text color distinction when in-focus
  2. able to set the desired entry value type : string, float, integer
  3. equipped with value validity check. It would run the given user command
     if the user enters invalid value type (inconvertible),
     and then force user to fix the value
  3. able to keep the entry focus after running user valid-command
  4. pressing Escape cancels the input and restore the previous value & color
  5. WARNING : you should not use 'command' keyword, because it's used by the extension
     class internally. Use 'commandIfValid' instead.
     Why does it turn out this way ? Here is the reason :
     sure I can accept 'enter' key event, but that would be thrown
     after the OUT event. What I need is it's thrown before the OUT event,
     and the 'command' keyword already behaves that way.
  '''
  valueMap={
     'STR':'string',
     'INT':'integer',
     'FLOAT':'floating-point',
     }
  def __init__(self,*args,**kw):
      # the default added extension properties of custom DirectEntry
      extensionProps={
         'valueType'        : "I'm a string",  # pass the desired input value type :
                                               # string, float, or integer.
                                               # e.g. if you need float entry, pass a float

         'stayFocus'        : True,   # keep the focus after user pressed Enter.
                                      # just in case the result is not satisfying enough,
                                      # user only need to re-type the value,
                                      # without any need to re-select the entry box.
                                      # After satisfied, user should press Esc to drop the focus.

         'active'           : True,   # active status at the beginning
         'commandIfValid'   : None,   # user command executed when the input value is valid
         'ifValidExtraArgs' : None,   # extra arguments for the user command above
         'commandIfInvalid' : None,   # user command executed when the input value is INVALID
         }
      for p in extensionProps:
          if p in kw:
             extensionProps[p]=kw.pop(p)
          self.__setattr__(p,extensionProps[p])
      if 'command' in kw:
         kw['command']=None
         print "\nWARNING (DirectEntry extension):\nplease do not use 'command' keyword.\nIt is reserved for internal use only. You should use 'commandIfValid' instead.\nYour 'command' keyword discarded.\n"
      if 'extraArgs' in kw:
         del kw['extraArgs']
         print "\nWARNING (DirectEntry extension):\nplease do not use 'extraArgs' keyword.\nIt is abandoned. You should use 'ifValidExtraArgs' instead.\nYour 'extraArgs' keyword discarded.\n"
      DirectEntry.__init__(self,*args,**kw)
      self.initialiseoptions(myEntry)
      self.prevValue=''
      if 'initialText' in kw:
         self.prevValue=kw['initialText']
      self.set(self.prevValue)
      if 'text_fg' in kw:
         self.idleTextColor=self['text_fg']
      else:
         self.idleTextColor=(0,0,0,1)
      self.idleFrameColor=self['frameColor']
      self['focusInCommand']=self.entryInFocus
      self['focusOutCommand']=self.entryOutFocus
      self['command']=self.acceptInput
      if not self.active:
         self.node().setActive(0)

  def getValidValue(self,text=None):
      if text==None:
         text=self.get()
      try:
          if type(self.valueType)==types.StringType:
             i=str(text)
          elif type(self.valueType)==types.FloatType:
             i=float(text)
          elif type(self.valueType)==types.IntType:
             i=int(float(text))
          return i
      except:
          return None

  def entryInFocus(self):
      print 'IN: ',self.getName()
      if FOCUS_Entry==self:
         self.acceptOnce(MODE_a2dEntryFocus+'escape', self.cancelEnteringText)
         return
      elif type(FOCUS_Entry)==types.TupleType:
         return
      if hasattr(self,'BTprefixBeforeMyEntryFocus'):
         if self.BTprefixBeforeMyEntryFocus!=MODE_a2dEntryFocus:
            self.BTprefixBeforeMyEntryFocus = getBTprefix()
            setBTprefix(MODE_a2dEntryFocus)
      else:
         self.BTprefixBeforeMyEntryFocus = getBTprefix()
         setBTprefix(MODE_a2dEntryFocus)
      __builtins__['FOCUS_Entry']=self
      self.prevValue=self.get()
      self['frameColor']=(.5,0,0,.5)
      self['text_fg']=(1,1,0,1)
      self.set(self.prevValue)
      self.acceptOnce(MODE_a2dEntryFocus+'escape', self.cancelEnteringText)

  def entryOutFocus(self):
      print 'OUT: ',self.getName()
      if FOCUS_Entry==None:
         return
      elif type(FOCUS_Entry)==types.TupleType:
         __builtins__['FOCUS_Entry']=FOCUS_Entry[0]
         return
      validVal = self.getValidValue()
      if validVal != self.getValidValue(self.prevValue):
         self.ignore(MODE_a2dEntryFocus+'escape')
         # the result is not the same, so it was changed.
         # BEWARE : it could be invalid input !!!
         # so let's check it
         if validVal!=None:
            # valid input
            self.prevValue=str(validVal)
            self.restoreEntry()
            self.destroyExpectedValueType()
            # run the command
            self.commandIfValid(validVal, *self.ifValidExtraArgs)
         else:
            # INVALID input !!!
            self.showExpectedValueType()
            self.commandIfInvalid(self)
            __builtins__['FOCUS_Entry']=(self,'INVALID')
            self['focus']=1
            return
      else:
         # no value changes to the last entry,
         # so store the valid value instead of the raw edited string
         if validVal==None:
            validVal=''
         self.prevValue=str(validVal)
         self.restoreEntry()
         self.ignore(MODE_a2dEntryFocus+'escape')
      setBTprefix(self.BTprefixBeforeMyEntryFocus)
      __builtins__['FOCUS_Entry']=None

  def acceptInput(self,a):
      print 'ACCEPTED',self.getName()
      self.ignore(MODE_a2dEntryFocus+'escape')
      validVal = self.getValidValue()
      if validVal != self.getValidValue(self.prevValue):
         # the result is not the same, so it was changed.
         # BEWARE : it could be invalid input !!!
         # so let's check it
         if validVal!=None:
            # valid input
            self.prevValue=str(validVal)
            self.set(self.prevValue)
            self.destroyExpectedValueType()
            # run the command
            self.commandIfValid(validVal, *self.ifValidExtraArgs)
            # keep focus ?
            if self.stayFocus:
               self['focus']=1
               return
            else:
               self.restoreEntry()
         else:
            # INVALID input !!!
            self.showExpectedValueType()
            self.commandIfInvalid(self)
            __builtins__['FOCUS_Entry']=(self,'INVALID')
            self['focus']=1
            return
      else:
         # no value changes to the last entry,
         # so store the valid value instead of the raw edited string
         if validVal==None:
            validVal=''
         self.prevValue=str(validVal)
         self.restoreEntry()
      setBTprefix(self.BTprefixBeforeMyEntryFocus)
      __builtins__['FOCUS_Entry']=None

  def cancelEnteringText(self):
      self.destroyExpectedValueType()
      self.restoreEntry()
      self['focus']=0
      __builtins__['FOCUS_Entry']=None
      setBTprefix(self.BTprefixBeforeMyEntryFocus)

  def restoreEntry(self):
      self['frameColor']=self.idleFrameColor
      self['text_fg']=self.idleTextColor
      self.set(self.prevValue)

  def showExpectedValueType(self):
      self.destroyExpectedValueType()
      self.expectedValText = DirectLabel( parent=self.getParent(), frameColor=(0,1,0,.8),
           text='expected type : '+self.valueMap[type(self.valueType).__name__.upper()],
           text_align=TextNode.ALeft, text_fg=(0,0,0,1),
           relief=DGG.GROOVE, pad=(.3,.05),
           scale=.045, enableEdit=0, suppressMouse=0, suppressKeys=0)
      self.expectedValText.setPos(self,-1.5,-10,1.5)

  def destroyExpectedValueType(self):
      if hasattr(self,'expectedValText'):
         self.expectedValText.destroy()

