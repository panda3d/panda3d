title = 'Pmw.OptionMenu demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create and pack the OptionMenu megawidgets.
        # The first one has a textvariable.
        self.var = tkinter.StringVar()
        self.var.set('steamed')
        self.method_menu = Pmw.OptionMenu(parent,
                labelpos = 'w',
                label_text = 'Choose method:',
                menubutton_textvariable = self.var,
                items = ['baked', 'steamed', 'stir fried', 'boiled', 'raw'],
                menubutton_width = 10,
        )
        self.method_menu.pack(anchor = 'w', padx = 10, pady = 10)

        self.vege_menu = Pmw.OptionMenu (parent,
                labelpos = 'w',
                label_text = 'Choose vegetable:',
                items = ('broccoli', 'peas', 'carrots', 'pumpkin'),
                menubutton_width = 10,
                command = self._printOrder,
        )
        self.vege_menu.pack(anchor = 'w', padx = 10, pady = 10)

        self.direction_menu = Pmw.OptionMenu (parent,
                labelpos = 'w',
                label_text = 'Menu direction:',
                items = ('flush', 'above', 'below', 'left', 'right'),
                menubutton_width = 10,
                command = self._changeDirection,
        )
        self.direction_menu.pack(anchor = 'w', padx = 10, pady = 10)

        menus = (self.method_menu, self.vege_menu, self.direction_menu)
        Pmw.alignlabels(menus)

    def _printOrder(self, vege):
        # Can use 'self.var.get()' instead of 'getcurselection()'.
        print(('You have chosen %s %s.' % \
            (self.method_menu.getcurselection(), vege)))

    def _changeDirection(self, direction):
        for menu in (self.method_menu, self.vege_menu, self.direction_menu):
            menu.configure(menubutton_direction = direction)

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
