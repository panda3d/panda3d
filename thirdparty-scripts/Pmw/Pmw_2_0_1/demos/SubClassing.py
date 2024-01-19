title = 'More examples of subclassing'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class ExtraMethods(Pmw.EntryField):

    # How to subclass a Pmw megawidget when you only want to add or
    # override methods.

    def doubletext(self):
        self.setvalue(self.getvalue() + ' ' + self.getvalue())

class OverrideInit(Pmw.EntryField):

    # How to subclass a Pmw megawidget when you want to define
    # a new __init__ method.

    def __init__(self, textToAdd, parent = None, **kw):
        self._textToAdd = textToAdd
        Pmw.EntryField.__init__(*(self, parent), **kw)

    def addtext(self):
        self.setvalue(self.getvalue() + ' ' + self._textToAdd)

class DefaultOptions(Pmw.EntryField):

    # How to subclass a Pmw megawidget when you only want to set
    # existing options to new default values.

    def __init__(self, parent = None, **kw):
        kw['label_foreground'] = 'blue'
        kw['entry_background'] = 'white'
        Pmw.EntryField.__init__(*(self, parent), **kw)

class NewOptions(Pmw.EntryField):

    # How to subclass a Pmw megawidget when you want to add new options.

    def __init__(self, parent=None , **kw):

        # Define the megawidget options.
        optiondefs = (
            ('backgrounds',              None,     self._backgrounds),
        )
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.EntryField.__init__(self, parent)

        # Check keywords and initialise options.
        self.initialiseoptions()

    def _backgrounds(self):
        background = self['backgrounds']
        Pmw.Color.changecolor(self.component('hull'), background)

class Demo:
    def __init__(self, parent):
        # Create and pack the megawidgets.
        self._extraMethod = ExtraMethods(parent,
                labelpos = 'w',
                label_text = 'Sub class with extra method:',
                value = 'Hello'
        )
        self._overrideInit = OverrideInit('Again', parent,
                labelpos = 'w',
                label_text = 'Sub class with new __init__ method:',
                value = 'Hello'
        )
        self._defaultOptions = DefaultOptions(parent,
                labelpos = 'w',
                label_text = 'Sub class with new default options:',
                value = 'Hello'
        )

        self._newOptions = NewOptions(parent,
                labelpos = 'w',
                label_text = 'Sub class with new option:',
                value = 'Hello',
                backgrounds = 'white',
        )

        entries = (self._extraMethod, self._overrideInit,
                self._defaultOptions, self._newOptions)

        for entry in entries:
            entry.pack(fill='x', expand=1, padx=10, pady=5)
        Pmw.alignlabels(entries)

        bb = Pmw.ButtonBox(parent)
        bb.add('Double text', command = self._doubleText)
        bb.pack()
        bb.add('Add text', command = self._addText)
        bb.pack()
        bb.add('White', command = self._changeColorWhite)
        bb.pack()
        bb.add('Green', command = self._changeColorGreen)
        bb.pack()

    def _doubleText(self):
        self._extraMethod.doubletext()

    def _addText(self):
        self._overrideInit.addtext()

    def _changeColorWhite(self):
        self._newOptions.configure(backgrounds = 'white')

    def _changeColorGreen(self):
        self._newOptions.configure(backgrounds = 'green')

######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = tkinter.Tk()
    Pmw.initialise(root)
    root.title(title)

    exitButton = tkinter.Button(root, text = 'Exit', command = root.destroy)
    exitButton.pack(side = 'bottom')
    widget = Demo(root)
    root.mainloop()
