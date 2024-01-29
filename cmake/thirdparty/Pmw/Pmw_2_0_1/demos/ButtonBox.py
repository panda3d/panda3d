title = 'Pmw.ButtonBox demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create and pack the ButtonBox.
        self.buttonBox = Pmw.ButtonBox(parent,
                labelpos = 'nw',
                label_text = 'ButtonBox:',
                frame_borderwidth = 2,
                frame_relief = 'groove')
        self.buttonBox.pack(fill = 'both', expand = 1, padx = 10, pady = 10)

        # Add some buttons to the ButtonBox.
        self.buttonBox.add('OK', command = self.ok)
        self.buttonBox.add('Apply', command = self.apply)
        self.buttonBox.add('Cancel', command = self.cancel)

        # Set the default button (the one executed when <Return> is hit).
        self.buttonBox.setdefault('OK')
        parent.bind('<Return>', self._processReturnKey)
        parent.focus_set()

        # Make all the buttons the same width.
        self.buttonBox.alignbuttons()

    def _processReturnKey(self, event):
        self.buttonBox.invoke()

    def ok(self):
        print('You clicked on OK')

    def apply(self):
        print('You clicked on Apply')

    def cancel(self):
        print('You clicked on Cancel')

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
