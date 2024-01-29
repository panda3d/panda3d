title = 'Pmw.ScrolledCanvas demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create the ScrolledCanvas.
        self.sc = Pmw.ScrolledCanvas(parent,
                borderframe = 1,
                labelpos = 'n',
                label_text = 'ScrolledCanvas',
                usehullsize = 1,
                hull_width = 400,
                hull_height = 300,
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
        self.sc.pack(padx = 5, pady = 5, fill = 'both', expand = 1)

        self.sc.component('canvas').bind('<1>', self.addcircle)

        testEntry = tkinter.Entry(parent)
        self.sc.create_line(20, 20, 100, 100)
        self.sc.create_oval(100, 100, 200, 200, fill = 'green')
        self.sc.create_text(100, 20, anchor = 'nw',
                text = 'Click in the canvas\nto draw ovals',
                font = testEntry.cget('font'))
        button = tkinter.Button(self.sc.interior(),
                text = 'Hello,\nWorld!\nThis\nis\na\nbutton.')
        self.sc.create_window(200, 200,
                anchor='nw',
                window = button)

        # Set the scroll region of the canvas to include all the items
        # just created.
        self.sc.resizescrollregion()

        self.colours = ('red', 'green', 'blue', 'yellow', 'cyan', 'magenta',
                'black', 'white')
        self.oval_count = 0
        self.rand = 12345

    def sethscrollmode(self, tag):
        self.sc.configure(hscrollmode = tag)

    def setvscrollmode(self, tag):
        self.sc.configure(vscrollmode = tag)

    def addcircle(self, event):
        x = self.sc.canvasx(event.x)
        y = self.sc.canvasy(event.y)
        width = 10 + self.random() % 100
        height = 10 + self.random() % 100
        self.sc.create_oval(
            x - width, y - height, x + width, y + height,
            fill = self.colours[self.oval_count])
        self.oval_count = (self.oval_count + 1) % len(self.colours)
        self.sc.resizescrollregion()

    # Simple random number generator.
    def random(self):
        self.rand = (self.rand * 125) % 2796203
        return self.rand

    def showYView(self):
        print((self.sc.yview()))

    def pageDown(self):
        self.sc.yview('scroll', 1, 'page')

    def centerPage(self):
        top, bottom = self.sc.yview()
        size = bottom - top
        middle = 0.5 - size / 2
        self.sc.yview('moveto', middle)

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
