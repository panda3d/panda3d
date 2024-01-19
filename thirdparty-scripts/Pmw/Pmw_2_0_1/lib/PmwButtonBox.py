# Based on iwidgets2.2.0/buttonbox.itk code.

import types
import tkinter
import Pmw

class ButtonBox(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):

        # Define the megawidget options.
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('labelmargin',       0,              INITOPT),
            ('labelpos',          None,           INITOPT),
            ('orient',            'horizontal',   INITOPT),
            ('padx',              3,              INITOPT),
            ('pady',              3,              INITOPT),
        )
        self.defineoptions(kw, optiondefs, dynamicGroups = ('Button',))

        # Initialise the base class (after defining the options).
        Pmw.MegaWidget.__init__(self, parent)

        # Create the components.
        interior = self.interior()
        if self['labelpos'] is None:
            self._buttonBoxFrame = self._hull
            columnOrRow = 0
        else:
            self._buttonBoxFrame = self.createcomponent('frame',
                    (), None,
                    tkinter.Frame, (interior,))
            self._buttonBoxFrame.grid(column=2, row=2, sticky='nsew')
            columnOrRow = 2

            self.createlabel(interior)

        orient = self['orient']
        if orient == 'horizontal':
            interior.grid_columnconfigure(columnOrRow, weight = 1)
        elif orient == 'vertical':
            interior.grid_rowconfigure(columnOrRow, weight = 1)
        else:
            raise ValueError('bad orient option ' + repr(orient) + \
                ': must be either \'horizontal\' or \'vertical\'')

        # Initialise instance variables.

        # List of tuples describing the buttons:
        #   - name
        #   - button widget
        self._buttonList = []

        # The index of the default button.
        self._defaultButton = None

        self._timerId = None

        # Check keywords and initialise options.
        self.initialiseoptions()

    def destroy(self):
        if self._timerId:
            self.after_cancel(self._timerId)
            self._timerId = None
        Pmw.MegaWidget.destroy(self)

    def numbuttons(self):
        return len(self._buttonList)

    def index(self, index, forInsert = 0):
        listLength = len(self._buttonList)
        if type(index) == int:
            if forInsert and index <= listLength:
                return index
            elif not forInsert and index < listLength:
                return index
            else:
                raise ValueError('index "%s" is out of range' % index)
        elif index is Pmw.END:
            if forInsert:
                return listLength
            elif listLength > 0:
                return listLength - 1
            else:
                raise ValueError('ButtonBox has no buttons')
        elif index is Pmw.DEFAULT:
            if self._defaultButton is not None:
                return self._defaultButton
            raise ValueError('ButtonBox has no default')
        else:
            names = [t[0] for t in self._buttonList]
            if index in names:
                return names.index(index)
            validValues = 'a name, a number, Pmw.END or Pmw.DEFAULT'
            raise ValueError('bad index "%s": must be %s' % (index, validValues))

    def insert(self, componentName, beforeComponent = 0, **kw):
        if componentName in self.components():
            raise ValueError('button "%s" already exists' % componentName)
        if 'text' not in kw:
            kw['text'] = componentName
        kw['default'] = 'normal'
        button = self.createcomponent(*(componentName,
                (), 'Button',
                tkinter.Button, (self._buttonBoxFrame,)), **kw)

        index = self.index(beforeComponent, 1)
        horizontal = self['orient'] == 'horizontal'
        numButtons = len(self._buttonList)

        # Shift buttons up one position.
        for i in range(numButtons - 1, index - 1, -1):
            widget = self._buttonList[i][1]
            pos = i * 2 + 3
            if horizontal:
                widget.grid(column = pos, row = 0)
            else:
                widget.grid(column = 0, row = pos)

        # Display the new button.
        if horizontal:
            button.grid(column = index * 2 + 1, row = 0, sticky = 'ew',
                    padx = self['padx'], pady = self['pady'])
            self._buttonBoxFrame.grid_columnconfigure(
                    numButtons * 2 + 2, weight = 1)
        else:
            button.grid(column = 0, row = index * 2 + 1, sticky = 'ew',
                    padx = self['padx'], pady = self['pady'])
            self._buttonBoxFrame.grid_rowconfigure(
                    numButtons * 2 + 2, weight = 1)
        self._buttonList.insert(index, (componentName, button))

        return button

    def add(self, componentName, **kw):
        return self.insert(*(componentName, len(self._buttonList)), **kw)

    def delete(self, index):
        index = self.index(index)
        (name, widget) = self._buttonList[index]
        widget.grid_forget()
        self.destroycomponent(name)

        numButtons = len(self._buttonList)

        # Shift buttons down one position.
        horizontal = self['orient'] == 'horizontal'
        for i in range(index + 1, numButtons):
            widget = self._buttonList[i][1]
            pos = i * 2 - 1
            if horizontal:
                widget.grid(column = pos, row = 0)
            else:
                widget.grid(column = 0, row = pos)

        if horizontal:
            self._buttonBoxFrame.grid_columnconfigure(numButtons * 2 - 1,
                    minsize = 0)
            self._buttonBoxFrame.grid_columnconfigure(numButtons * 2, weight = 0)
        else:
            self._buttonBoxFrame.grid_rowconfigure(numButtons * 2, weight = 0)
        del self._buttonList[index]

    def setdefault(self, index):
        # Turn off the default ring around the current default button.
        if self._defaultButton is not None:
            button = self._buttonList[self._defaultButton][1]
            button.configure(default = 'normal')
            self._defaultButton = None

        # Turn on the default ring around the new default button.
        if index is not None:
            index = self.index(index)
            self._defaultButton = index
            button = self._buttonList[index][1]
            button.configure(default = 'active')

    def invoke(self, index = Pmw.DEFAULT, noFlash = 0):
        # Invoke the callback associated with the *index* button.  If
        # *noFlash* is not set, flash the button to indicate to the
        # user that something happened.

        button = self._buttonList[self.index(index)][1]
        if not noFlash:
            state = button.cget('state')
            relief = button.cget('relief')
            button.configure(state = 'active', relief = 'sunken')
            self.update_idletasks()
            self.after(100)
            button.configure(state = state, relief = relief)
        return button.invoke()

    def button(self, buttonIndex):
        return self._buttonList[self.index(buttonIndex)][1]

    def alignbuttons(self, when = 'later'):
        if when == 'later':
            if not self._timerId:
                self._timerId = self.after_idle(self.alignbuttons, 'now')
            return
        self.update_idletasks()
        self._timerId = None

        # Determine the width of the maximum length button.
        max = 0
        horizontal = (self['orient'] == 'horizontal')
        for index in range(len(self._buttonList)):
            gridIndex = index * 2 + 1
            if horizontal:
                width = self._buttonBoxFrame.grid_bbox(gridIndex, 0)[2]
            else:
                width = self._buttonBoxFrame.grid_bbox(0, gridIndex)[2]
            if width > max:
                max = width

        # Set the width of all the buttons to be the same.
        if horizontal:
            for index in range(len(self._buttonList)):
                self._buttonBoxFrame.grid_columnconfigure(index * 2 + 1,
                        minsize = max)
        else:
            self._buttonBoxFrame.grid_columnconfigure(0, minsize = max)
