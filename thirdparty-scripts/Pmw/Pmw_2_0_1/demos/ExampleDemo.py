title = 'Pmw.EXAMPLE demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):

        # Create and pack the EXAMPLEs.
        self.widget1 = Pmw.Counter(parent)
        self.widget1.setentry('1')
        self.widget1.pack()

        self.widget2 = Pmw.Counter(parent, increment = 10)
        self.widget2.setentry('100')
        self.widget2.pack()

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
