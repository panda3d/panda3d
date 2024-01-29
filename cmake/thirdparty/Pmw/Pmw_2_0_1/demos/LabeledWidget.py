title = 'Pmw.LabeledWidget demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):

        # Create a frame to put the LabeledWidgets into
        frame = tkinter.Frame(parent, background = 'grey90')
        frame.pack(fill = 'both', expand = 1)

        # Create and pack the LabeledWidgets.
        column = 0
        row = 0
        for pos in ('n', 'nw', 'wn', 'w'):
            lw = Pmw.LabeledWidget(frame,
                    labelpos = pos,
                    label_text = pos + ' label')
            lw.component('hull').configure(relief='sunken', borderwidth=2)
            lw.grid(column=column, row=row, padx=10, pady=10)
            cw = tkinter.Button(lw.interior(), text='child\nsite')
            cw.pack(padx=10, pady=10, expand='yes', fill='both')

            # Get ready for next grid position.
            column = column + 1
            if column == 2:
                column = 0
                row = row + 1

######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = tkinter.Tk()
    Pmw.initialise(root)
    root.title(title)

    widget = Demo(root)
    exitButton = tkinter.Button(root, text = 'Exit', command = root.destroy)
    exitButton.pack()
    root.mainloop()
