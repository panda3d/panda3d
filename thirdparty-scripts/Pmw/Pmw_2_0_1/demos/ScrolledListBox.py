title = 'Pmw.ScrolledListBox demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create the ScrolledListBox.
        self.box = Pmw.ScrolledListBox(parent,
                items=('Sydney', 'Melbourne', 'Brisbane'),
                labelpos='nw',
                label_text='Cities',
                listbox_height = 6,
                selectioncommand=self.selectionCommand,
                dblclickcommand=self.defCmd,
                usehullsize = 1,
                hull_width = 200,
                hull_height = 200,
        )

        # Create a group widget to contain the scrollmode options.
        w = Pmw.Group(parent, tag_text='Scroll mode')
        w.pack(side = 'bottom', padx = 5, pady = 5)

        hmode = Pmw.OptionMenu(w.interior(),
                labelpos = 'w',
                label_text = 'Horizontal:',
                items = ['none', 'static', 'dynamic'],
                command = self.sethscrollmode,
                menubutton_width = 8,
        )
        hmode.pack(side = 'top', padx = 5, pady = 5)
        hmode.invoke('dynamic')

        vmode = Pmw.OptionMenu(w.interior(),
                labelpos = 'w',
                label_text = 'Vertical:',
                items = ['none', 'static', 'dynamic'],
                command = self.setvscrollmode,
                menubutton_width = 8,
        )
        vmode.pack(side = 'top', padx = 5, pady = 5)
        vmode.invoke('dynamic')

        buttonBox = Pmw.ButtonBox(parent)
        buttonBox.pack(side = 'bottom')
        buttonBox.add('yview', text = 'Show\nyview', command = self.showYView)
        buttonBox.add('scroll', text = 'Page\ndown', command = self.pageDown)
        buttonBox.add('center', text = 'Center', command = self.centerPage)

        # Pack this last so that the buttons do not get shrunk when
        # the window is resized.
        self.box.pack(fill = 'both', expand = 1, padx = 5, pady = 5)

        # Do this after packing the scrolled list box, so that the
        # window does not resize as soon as it appears (because
        # alignlabels has to do an update_idletasks).
        Pmw.alignlabels((hmode, vmode))

        # Add some more entries to the listbox.
        items = ('Andamooka', 'Coober Pedy', 'Innamincka', 'Oodnadatta')
        self.box.setlist(items)
        self.box.insert(2, 'Wagga Wagga', 'Perth', 'London')
        self.box.insert('end', 'Darwin', 'Auckland', 'New York')
        index = list(self.box.get(0, 'end')).index('London')
        self.box.delete(index)
        self.box.delete(7, 8)
        self.box.insert('end', 'Bulli', 'Alice Springs', 'Woy Woy')
        self.box.insert('end', 'Wallumburrawang', 'Willandra Billabong')

    def sethscrollmode(self, tag):
        self.box.configure(hscrollmode = tag)

    def setvscrollmode(self, tag):
        self.box.configure(vscrollmode = tag)

    def selectionCommand(self):
        sels = self.box.getcurselection()
        if len(sels) == 0:
            print('No selection')
        else:
            print(('Selection:', sels[0]))

    def defCmd(self):
        sels = self.box.getcurselection()
        if len(sels) == 0:
            print('No selection for double click')
        else:
            print(('Double click:', sels[0]))

    def showYView(self):
        print((self.box.yview()))

    def pageDown(self):
        self.box.yview('scroll', 1, 'page')

    def centerPage(self):
        top, bottom = self.box.yview()
        size = bottom - top
        middle = 0.5 - size / 2
        self.box.yview('moveto', middle)

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
