title = 'Colorscheme demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        frame = tkinter.Frame(parent)
        frame.pack(fill = 'both', expand = 1)

        defaultPalette = Pmw.Color.getdefaultpalette(parent)

        colors = ('red', 'green', 'blue')
        items = ('Testing', 'More testing', 'a test', 'foo', 'blah')
        for count in range(len(colors)):
            color = colors[count]
            normalcolor = Pmw.Color.changebrightness(parent, color, 0.85)
            Pmw.Color.setscheme(parent, normalcolor)
            combo = Pmw.ComboBox(frame,
                    scrolledlist_items = items,
                    entryfield_value = items[0])
            combo.grid(sticky='nsew', row = count, column = 0)

            normalcolor = Pmw.Color.changebrightness(parent, color, 0.35)
            Pmw.Color.setscheme(parent, normalcolor, foreground = 'white')
            combo = Pmw.ComboBox(frame,
                    scrolledlist_items = items,
                    entryfield_value = items[0])
            combo.grid(sticky='nsew', row = count, column = 1)

        Pmw.Color.setscheme(*(parent,), **defaultPalette)
        #normalcolor = Pmw.Color.changebrightness(parent, 'red', 0.85)
        #Pmw.Color.setscheme(parent, normalcolor)

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
