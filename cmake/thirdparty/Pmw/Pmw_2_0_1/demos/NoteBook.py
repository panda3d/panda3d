title = 'Pmw.NoteBook demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create and pack the NoteBook.
        notebook = Pmw.NoteBook(parent)
        notebook.pack(fill = 'both', expand = 1, padx = 10, pady = 10)

        # Add the "Appearance" page to the notebook.
        page = notebook.add('Appearance')
        notebook.tab('Appearance').focus_set()

        # Create the "Toolbar" contents of the page.
        group = Pmw.Group(page, tag_text = 'Toolbar')
        group.pack(fill = 'both', expand = 1, padx = 10, pady = 10)
        b1 = tkinter.Checkbutton(group.interior(), text = 'Show toolbar')
        b1.grid(row = 0, column = 0)
        b2 = tkinter.Checkbutton(group.interior(), text = 'Toolbar tips')
        b2.grid(row = 0, column = 1)

        # Create the "Startup" contents of the page.
        group = Pmw.Group(page, tag_text = 'Startup')
        group.pack(fill = 'both', expand = 1, padx = 10, pady = 10)
        home = Pmw.EntryField(group.interior(), labelpos = 'w',
            label_text = 'Home page location:')
        home.pack(fill = 'x', padx = 20, pady = 10)

        # Add two more empty pages.
        page = notebook.add('Helpers')
        page = notebook.add('Images')

        notebook.setnaturalsize()

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
