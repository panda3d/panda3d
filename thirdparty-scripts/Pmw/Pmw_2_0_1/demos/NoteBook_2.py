title = 'Pmw.NoteBook demonstration (more complex)'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent, withTabs = 1):

        # Repeat random number sequence for each run.
        self.rand = 12345

        # Default demo is to display a tabbed notebook.
        self.withTabs = withTabs

        # Create a frame to put everything in
        self.mainframe = tkinter.Frame(parent)
        self.mainframe.pack(fill = 'both', expand = 1)

        # Find current default colors
        button = tkinter.Button()
        defaultbg = button.cget('background')
        defaultfg = button.cget('foreground')
        button.destroy()

        # Create the list of colors to cycle through
        self.colorList = []
        self.colorList.append((defaultbg, defaultfg))
        self.colorIndex = 0
        for color in Pmw.Color.spectrum(6, 1.5, 1.0, 1.0, 1):
            bg = Pmw.Color.changebrightness(self.mainframe, color, 0.85)
            self.colorList.append((bg, 'black'))
            bg = Pmw.Color.changebrightness(self.mainframe, color, 0.55)
            self.colorList.append((bg, 'white'))

        # Set the color to the current default
        Pmw.Color.changecolor(self.mainframe, defaultbg, foreground = defaultfg)
        defaultPalette = Pmw.Color.getdefaultpalette(self.mainframe)
        Pmw.Color.setscheme(self.mainframe, defaultbg, foreground = defaultfg)

        # Create the notebook, but don't pack it yet.
        if self.withTabs:
            tabpos = 'n'
        else:
            tabpos = None
        self.notebook = Pmw.NoteBook(self.mainframe,
                tabpos = tabpos,
                createcommand = PrintOne('Create'),
                lowercommand = PrintOne('Lower'),
                raisecommand = PrintOne('Raise'),
                hull_width = 300,
                hull_height = 200,
                )

        # Create a buttonbox to configure the notebook and pack it first.
        buttonbox = Pmw.ButtonBox(self.mainframe)
        buttonbox.pack(side = 'bottom', fill = 'x')

        # Add some buttons to the buttonbox to configure the notebook.
        buttonbox.add('Insert\npage', command = self.insertpage)
        buttonbox.add('Delete\npage', command = self.deletepage)
        buttonbox.add('Add\nbutton', command = self.addbutton)
        buttonbox.add('Change\ncolor', command = self.changecolor)
        buttonbox.add('Natural\nsize', command =
                self.notebook.setnaturalsize)

        if not self.withTabs:
            # Create the selection widget to select the page in the notebook.
            self.optionmenu = Pmw.OptionMenu(self.mainframe,
                    menubutton_width = 10,
                    command = self.notebook.selectpage
            )
            self.optionmenu.pack(side = 'left', padx = 10)

        # Pack the notebook last so that the buttonbox does not disappear
        # when the window is made smaller.
        self.notebook.pack(fill = 'both', expand = 1, padx = 5, pady = 5)

        # Populate some pages of the notebook.
        page = self.notebook.add('tmp')
        self.notebook.delete('tmp')
        page = self.notebook.add('Appearance')
        if self.withTabs:
            self.notebook.tab('Appearance').focus_set()
        button = tkinter.Button(page,
            text = 'Welcome\nto\nthe\nAppearance\npage')
        button.pack(expand = 1)
        page = self.notebook.add('Fonts')
        button = tkinter.Button(page,
            text = 'This is a very very very very wide Fonts page')
        button.pack(expand = 1)
        page = self.notebook.insert('Applications', before = 'Fonts')
        button = tkinter.Button(page, text = 'This is the Applications page')
        button.pack(expand = 1)

        # Initialise the first page and the initial colour.
        if not self.withTabs:
            self.optionmenu.setitems(self.notebook.pagenames())
        Pmw.Color.setscheme(*(self.mainframe,), **defaultPalette)
        self.pageCounter = 0

    def insertpage(self):
        # Create a page at a random position

        defaultPalette = Pmw.Color.getdefaultpalette(self.mainframe)
        bg, fg = self.colorList[self.colorIndex]
        Pmw.Color.setscheme(self.mainframe, bg, foreground = fg)

        self.pageCounter = self.pageCounter + 1
        before = self.randomchoice(self.notebook.pagenames() + [Pmw.END])
        pageName = 'page%d' % self.pageCounter
        if self.pageCounter % 5 == 0:
            tab_text = pageName + '\nline two'
        else:
            tab_text = pageName
        classes = (None, tkinter.Button, tkinter.Label, tkinter.Checkbutton)
        cls = self.randomchoice((None,) + classes)
        if cls is None:
            print(('Adding', pageName, 'as a frame with a button'))
            if self.withTabs:
                page = self.notebook.insert(pageName, before, tab_text = tab_text)
            else:
                page = self.notebook.insert(pageName, before)
            button = tkinter.Button(page,
                    text = 'This is button %d' % self.pageCounter)
            button.pack(expand = 1)
        else:
            print(('Adding', pageName, 'using', cls))
            if self.withTabs:
                page = self.notebook.insert(pageName, before,
                        tab_text = tab_text,
                        page_pyclass = cls,
                        page_text = 'This is a page using\na %s' % str(cls)
                        )
            else:
                page = self.notebook.insert(pageName, before,
                        page_pyclass = cls,
                        page_text = 'This is a page using\na %s' % str(cls)
                        )
        if not self.withTabs:
            self.optionmenu.setitems(
                self.notebook.pagenames(), self.notebook.getcurselection())

        Pmw.Color.setscheme(*(self.mainframe,), **defaultPalette)

    def addbutton(self):
        # Add a button to a random page.

        defaultPalette = Pmw.Color.getdefaultpalette(self.mainframe)
        bg, fg = self.colorList[self.colorIndex]
        Pmw.Color.setscheme(self.mainframe, bg, foreground = fg)

        framePages = []
        for pageName in self.notebook.pagenames():
            page = self.notebook.page(pageName)
            if page.__class__ == tkinter.Frame:
                framePages.append(pageName)

        if len(framePages) == 0:
            self.notebook.bell()
            return

        pageName = self.randomchoice(framePages)
        print(('Adding extra button to', pageName))
        page = self.notebook.page(pageName)
        button = tkinter.Button(page, text = 'This is an extra button')
        button.pack(expand = 1)

        Pmw.Color.setscheme(*(self.mainframe,), **defaultPalette)

    def deletepage(self):
        # Delete a random page

        pageNames = self.notebook.pagenames()
        if len(pageNames) == 0:
            self.notebook.bell()
            return

        pageName = self.randomchoice(pageNames)
        print(('Deleting', pageName))
        self.notebook.delete(pageName)
        if not self.withTabs:
            self.optionmenu.setitems(
                self.notebook.pagenames(), self.notebook.getcurselection())

    def changecolor(self):
        self.colorIndex = self.colorIndex + 1
        if self.colorIndex == len(self.colorList):
            self.colorIndex = 0

        bg, fg = self.colorList[self.colorIndex]
        print(('Changing color to', bg))
        Pmw.Color.changecolor(self.mainframe, bg, foreground = fg)
        self.notebook.recolorborders()

    # Simple random number generator.
    def randomchoice(self, selection):
        num = len(selection)
        self.rand = (self.rand * 125) % 2796203
        index = self.rand % num
        return selection[index]

class PrintOne:
    def __init__(self, text):
        self.text = text

    def __call__(self, text):
        print((self.text, text))

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
