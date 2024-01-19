title = 'Demonstration of how to create a megawidget'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class TextDisplay(Pmw.MegaWidget):

    # Demo Pmw megawidget.

    def __init__(self, parent = None, **kw):

        # Define the megawidget options.
        optiondefs = ()
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.MegaWidget.__init__(self, parent)

        # Create the components.
        interior = self.interior()

        self._text = self.createcomponent('text',
                (), None,
                tkinter.Text, (interior,), state = 'disabled')
        self._text.pack(side='left', fill='both', expand='yes')

        self._scrollbar = self.createcomponent('scrollbar',
                (), None,
                tkinter.Scrollbar, (interior,), command = self._text.yview)
        self._scrollbar.pack(side='right', fill='y')
        self._text.configure(yscrollcommand = self._scrollbar.set)

        # Check keywords and initialise options.
        self.initialiseoptions()

    def display(self, info):
        self._text.configure(state = 'normal')
        self._text.delete('1.0', 'end')
        self._text.insert('1.0', info)
        self._text.configure(state = 'disabled')

    def append(self, info):
        self._text.configure(state = 'normal')
        self._text.insert('end', info)
        self._text.configure(state = 'disabled')

class Demo:
    def __init__(self, parent):
        # Create and pack the megawidget.
        text = TextDisplay(parent,
                text_background = 'aliceblue',
                text_width = 40,
                text_height = 10,
                text_wrap = 'none',
        )
        text.pack(fill = 'both', expand = 1)
        text.display('This is an example of a simple Pmw megawidget.\n\n' +
                'Public attributes of the Tkinter module:\n\n')
        for name in dir(tkinter):
            if name[0] != '_':
                text.append('    ' + name + '\n')

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
