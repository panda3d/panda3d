title = 'Demonstration of Pmw megawidget destruction'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create and pack an EntryField.
        self.entryfield = Pmw.EntryField(parent,
            command = self.execute,
            value = 'Press <Return> to destroy me',
            entry_width = 30)
        self.entryfield.pack(fill='x', expand=1, padx=10, pady=5)

        self.entryfield.component('entry').focus_set()

    def execute(self):
        print('Return pressed, destroying EntryField.')
        self.entryfield.destroy()

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
