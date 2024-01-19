title = 'Pmw.ScrolledText demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import os
import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create the ScrolledText.
        self.st = Pmw.ScrolledText(parent,
                borderframe = 1,
                labelpos = 'n',
                label_text='ScrolledText.py',
                usehullsize = 1,
                hull_width = 400,
                hull_height = 300,
                text_padx = 10,
                text_pady = 10,
                text_wrap='none'
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
        hmode.pack(side = 'left', padx = 5, pady = 5)
        hmode.invoke('dynamic')

        vmode = Pmw.OptionMenu(w.interior(),
                labelpos = 'w',
                label_text = 'Vertical:',
                items = ['none', 'static', 'dynamic'],
                command = self.setvscrollmode,
                menubutton_width = 8,
        )
        vmode.pack(side = 'left', padx = 5, pady = 5)
        vmode.invoke('dynamic')

        buttonBox = Pmw.ButtonBox(parent)
        buttonBox.pack(side = 'bottom')
        buttonBox.add('yview', text = 'Show\nyview', command = self.showYView)
        buttonBox.add('scroll', text = 'Page\ndown', command = self.pageDown)
        buttonBox.add('center', text = 'Center', command = self.centerPage)

        # Pack this last so that the buttons do not get shrunk when
        # the window is resized.
        self.st.pack(padx = 5, pady = 5, fill = 'both', expand = 1)

        # Read this file into the text widget.
        head, tail = os.path.split(sys.argv[0])
        self.st.importfile(os.path.join(head,'ScrolledText.py'))

        self.st.insert('end', '\nThis demonstrates how to\n' +
            'add a window to a text widget:  ')
        counter = Pmw.Counter(self.st.component('text'),
            entryfield_value = 9999)
        self.st.window_create('end', window = counter)

    def sethscrollmode(self, tag):
        self.st.configure(hscrollmode = tag)

    def setvscrollmode(self, tag):
        self.st.configure(vscrollmode = tag)

    def showYView(self):
        print((self.st.yview()))

    def pageDown(self):
        self.st.yview('scroll', 1, 'page')

    def centerPage(self):
        top, bottom = self.st.yview()
        size = bottom - top
        middle = 0.5 - size / 2
        self.st.yview('moveto', middle)

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
