title = 'Pmw error handling demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import string
import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create two buttons to generate errors.
        w = tkinter.Button(parent, text = 'Click here to generate\n' +
                'an error in a command callback.', command = self.execute)
        w.pack(padx = 8, pady = 8)

        w = tkinter.Button(parent, text = 'Click here to generate\n' +
                'an error in a callback called\nfrom an event binding.')
        w.pack(padx = 8, pady = 8)
        w.bind('<ButtonRelease-1>', self.execute)
        w.bind('<Key-space>', self.execute)

    def execute(self, event = None):
        self._error()

    def _error(self):
        # Divide by zero
        1/0

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
