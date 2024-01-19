title = 'Pmw.TimeCounter demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import string
import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        self._time = Pmw.TimeCounter(parent,
                labelpos = 'w',
                label_text = 'HH:MM:SS',
                min = '00:00:00',
                max = '23:59:59')
        self._time.pack(padx=10, pady=5)

        button = tkinter.Button(parent, text = 'Show', command = self.show)
        button.pack()

    def show(self):
        stringVal = self._time.getstring()
        intVal =  self._time.getint()
        print((stringVal + '  (' + str(intVal) + ')'))


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
