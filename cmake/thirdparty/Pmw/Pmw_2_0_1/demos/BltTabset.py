title = 'Blt Tabset demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        if not Pmw.Blt.haveblt(parent):
            message = 'Sorry\nThe BLT package has not been\n' + \
                    'installed on this system.\n' + \
                    'Please install it and try again.'
            w = tkinter.Label(parent, text = message)
            w.pack(padx = 8, pady = 8)
            return

        self.tabset = Pmw.Blt.Tabset(parent,
            borderwidth = 0,
            highlightthickness = 0,
            selectpad = 0,
            tiers = 2,
        )
        background = self.tabset.cget('background')
        self.tabset.configure(selectbackground = background,
                tabbackground = background, activebackground = background)

        configurePanel = tkinter.Frame(self.tabset)
        sideMenu = Pmw.OptionMenu (configurePanel,
                labelpos = 'w',
                label_text = 'Side:',
                items = ('top', 'bottom', 'left', 'right'),
                menubutton_width = 10,
                command = self.changeSide,
        )
        sideMenu.pack(anchor = 'w', padx = 10, pady = 10)

        rotateMenu = Pmw.ComboBox(configurePanel,
                labelpos = 'w',
                label_text = 'Text rotation:',
                entryfield_validate = 'integer',
                entry_width = 8,
                selectioncommand = self.rotateText,
                scrolledlist_items = (0, 45, 90, 135, 180, 225, 270, 315),
        )
        rotateMenu.pack(side = 'left', padx = 10, pady = 10)

        rotateMenu.selectitem(0)
        self.rotateText('0')

        self.appearancePanel = tkinter.Label(self.tabset)
        helpersPanel = tkinter.Button(self.tabset,
                text = 'This is a lot\nof help!')

        self.tabset.insert('end',
                'Appearance', 'Configure', 'Helpers', 'Images')

        self.tabset.tab_configure('Appearance',
                command = self.appearance_cb, fill = 'both')
        self.tabset.tab_configure('Configure', window = configurePanel)
        self.tabset.tab_configure('Images',
                command = self.images_cb, fill = 'both')
        self.tabset.tab_configure('Helpers',
                window = helpersPanel, padx = 100, pady = 150)

        self.tabset.invoke(1)
        self.tabset.pack(fill = 'both', expand = 1, padx = 5, pady = 5)
        self.tabset.focus()

    def appearance_cb(self):
        self.appearancePanel.configure(
                text = 'Don\'t judge a book\nby it\'s cover.')
        self.tabset.tab_configure('Appearance', window = self.appearancePanel)

    def images_cb(self):
        self.appearancePanel.configure(text = 'Beauty is only\nskin deep.')
        self.tabset.tab_configure('Images', window = self.appearancePanel)

    def changeSide(self, side):
        self.tabset.configure(side = side)

    def rotateText(self, angle):
        if Pmw.integervalidator(angle) == Pmw.OK:
            self.tabset.configure(rotate = angle)
        else:
            self.tabset.bell()

######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = tkinter.Tk()
    Pmw.initialise(root)
    Pmw.Blt.setBltDisable(root, False)
    root.title(title)

    exitButton = tkinter.Button(root, text = 'Exit', command = root.destroy)
    exitButton.pack(side = 'bottom')
    widget = Demo(root)
    root.mainloop()
