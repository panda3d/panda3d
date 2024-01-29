title = 'Pmw toplevel megawidget demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class MessageInfo(Pmw.MegaToplevel):

    # Demo Pmw toplevel megawidget.

    def __init__(self, parent=None, **kw):

        # Define the megawidget options.
        optiondefs = ()
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.MegaToplevel.__init__(self, parent)

        # Create the components.
        interior = self.interior()

        self._dismiss = self.createcomponent('dismiss',
                (), None,
                tkinter.Button, (interior,),
                text = 'Dismiss',
                command = self.goodbye)
        self._dismiss.pack(side = 'bottom', pady = 4)

        self._separator = self.createcomponent('separator',
                (), None,
                tkinter.Frame, (interior,),
                height = 2,
                borderwidth = 1,
                relief = 'sunken')
        self._separator.pack(side = 'bottom', fill = 'x', pady = 4)

        self._icon = self.createcomponent('icon',
                (), None,
                tkinter.Label, (interior,))
        self._icon.pack(side = 'left', padx = 8, pady = 8)

        self._infoFrame = self.createcomponent('infoframe',
                (), None,
                tkinter.Frame, (interior,))
        self._infoFrame.pack(
                side = 'left',
                fill = 'both',
                expand = 1,
                padx = 4,
                pady = 4)

        self._message = self.createcomponent('message',
                (), None,
                tkinter.Label, (interior,))
        self._message.pack(expand = 1, fill = 'both', padx = 10, pady = 10)

        self.bind('<Return>', self.goodbye)

        # Check keywords and initialise options.
        self.initialiseoptions()

    def goodbye(self, event = None):
        self.destroy()

class Demo:
    def __init__(self, parent):
        # Create button to launch the megawidget.
        self.button = tkinter.Button(parent,
                command = self.showMessageInfo,
                text = 'Show toplevel megawidget')
        self.button.pack(padx = 8, pady = 8)

        self.count = 0
        self.parent = parent

    def showMessageInfo(self):
        bitmaps = ('warning', 'hourglass', 'error', 'info',
                'gray25', 'gray50', 'question', 'questhead')
        bitmap = bitmaps[self.count % len(bitmaps)]

        message = 'This is a demonstration of\na megawidget.\n' + \
                'It contains a configurable\nmessage area and bitmap.\n' + \
                'This instance is displaying\nthe "' + bitmap + '" bitmap.'

        # Make the toplevel window a child of this window, so that it
        # is destroyed when the demo is destroyed.
        MessageInfo(self.parent, message_text = message, icon_bitmap = bitmap)

        self.count = self.count + 1
        if self.count == 1:
            self.button.configure(text = 'Show another\ntoplevel megawidget')

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
