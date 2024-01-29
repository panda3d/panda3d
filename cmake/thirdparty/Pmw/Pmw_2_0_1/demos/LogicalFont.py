title = 'Pmw LogicalFont demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import string
import tkinter
import Pmw

class Demo:

    # The fonts to demonstrate.
    fontList = (
      (('Times', 0), {}),
      (('Helvetica', 0), {}),
      (('Typewriter', 0), {}),
      (('Fixed', 0), {}),
      (('Courier', 0), {}),
      (('Helvetica', 2), {'slant' : 'italic'}),
      (('Helvetica', 0), {'size' : 18}),
      (('Helvetica', 0), {'weight' : 'bold'}),
      (('Helvetica', 12), {'weight' : 'bold', 'slant' : 'italic'}),
      (('Typewriter', 0), {'size' : 8, 'weight' : 'bold'}),
      (('Fixed', 0), {'size' : 8, 'weight' : 'bold'}),
      (('Times', 0), {'size' : 24, 'weight' : 'bold', 'slant' : 'italic'}),
      (('Typewriter', 0), {'width' : 'condensed'}),
      (('Typewriter', -1), {'width' : 'condensed'}),
      (('Fixed', 0), {'width' : 'condensed'}),
      (('Fixed', -1), {'width' : 'condensed'}),
      (('Helvetica', 0), {'weight' : 'bogus'}),
    )

    fontText = []

    def __init__(self, parent):

        self.parent = parent

        # Create the text to display to the user to represent each font.
        if Demo.fontText == []:
            for args, dict in Demo.fontList:
                text = args[0]
                if args[1] != 0:
                    text = text + ' ' + str(args[1])
                for name, value in list(dict.items()):
                    text = text + ' ' + name + ': ' + str(value)
                Demo.fontText.append(text)

        # Create a listbox to contain the font selections.
        self.box = Pmw.ScrolledListBox(parent, listbox_selectmode='single',
                listbox_width = 35,
                listbox_height = 10,
                items=Demo.fontText,
                label_text='Font', labelpos='nw',
                selectioncommand=self.selectionCommand)
        self.box.pack(fill = 'both', expand = 1, padx = 10, pady = 10)

        # Create a label to display the selected font.
        self.target = tkinter.Label(parent,
                text = 'The quick brown fox jumps\nover the lazy dog',
                relief = 'sunken', padx = 10, pady = 10)
        self.target.pack(fill = 'both', expand = 1, padx = 10, pady = 10)

    def selectionCommand(self):
        sel = self.box.curselection()
        if len(sel) > 0:
            args, dict = Demo.fontList[string.atoi(sel[0])]
            font = Pmw.logicalfont(*args, **dict)
            self.target.configure(font = font)
            print(font)

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
