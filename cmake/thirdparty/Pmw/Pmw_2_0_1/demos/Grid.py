title = 'Grid geometry manager demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        frame = tkinter.Frame(parent)
        frame.pack(fill = 'both', expand = 1)

        button = {}
        for num in range(0, 10):
            button[num] = tkinter.Button(frame, text = 'Button ' + str(num))

        button[0].grid(column=0, row=0, rowspan=2, sticky='nsew')
        button[1].grid(column=1, row=0, columnspan=3, sticky='nsew')
        button[2].grid(column=1, row=1, rowspan=2, sticky='nsew')
        button[3].grid(column=2, row=1)
        button[4].grid(column=3, row=1)
        button[5].grid(column=0, row=2)
        button[6].grid(column=0, row=3, columnspan=2, sticky='nsew')
        button[7].grid(column=2, row=2, columnspan=2, rowspan=2, sticky='nsew')
        button[8].grid(column=0, row=4)
        button[9].grid(column=3, row=4, sticky='e')

        frame.grid_rowconfigure(3, weight=1)
        frame.grid_columnconfigure(3, weight=1)

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
