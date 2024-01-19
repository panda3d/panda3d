title = 'Pmw.MenuBar demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create the Balloon.
        self.balloon = Pmw.Balloon(parent)

        # Create and pack the MenuBar.
        menuBar = Pmw.MenuBar(parent,
                hull_relief = 'raised',
                hull_borderwidth = 1,
                balloon = self.balloon)
        menuBar.pack(fill = 'x')
        self.menuBar = menuBar

        # Add some buttons to the MenuBar.
        menuBar.addmenu('File', 'Close this window or exit')
        menuBar.addmenuitem('File', 'command', 'Close this window',
                command = PrintOne('Action: close'),
                label = 'Close')
        menuBar.addmenuitem('File', 'separator')
        menuBar.addmenuitem('File', 'command', 'Exit the application',
                command = PrintOne('Action: exit'),
                label = 'Exit')

        menuBar.addmenu('Edit', 'Cut, copy or paste')
        menuBar.addmenuitem('Edit', 'command', 'Delete the current selection',
                command = PrintOne('Action: delete'),
                label = 'Delete')

        menuBar.addmenu('Options', 'Set user preferences')
        menuBar.addmenuitem('Options', 'command', 'Set general preferences',
                command = PrintOne('Action: general options'),
                label = 'General...')

        # Create a checkbutton menu item.
        self.toggleVar = tkinter.IntVar()
        # Initialise the checkbutton to 1:
        self.toggleVar.set(1)
        menuBar.addmenuitem('Options', 'checkbutton', 'Toggle me on/off',
                label = 'Toggle',
                command = self._toggleMe,
                variable = self.toggleVar)
        self._toggleMe()

        menuBar.addcascademenu('Options', 'Size',
                'Set some other preferences', traverseSpec = 'z', tearoff = 1)
        for size in ('tiny', 'small', 'average', 'big', 'huge'):
            menuBar.addmenuitem('Size', 'command', 'Set size to ' + size,
                    command = PrintOne('Action: size ' + size),
                    label = size)

        menuBar.addmenu('Help', 'User manuals', side = 'right')
        menuBar.addmenuitem('Help', 'command', 'About this application',
                command = PrintOne('Action: about'),
                label = 'About...')

        # Create and pack the main part of the window.
        self.mainPart = tkinter.Label(parent,
                text = 'This is the\nmain part of\nthe window',
                background = 'black',
                foreground = 'white',
                padx = 30,
                pady = 30)
        self.mainPart.pack(fill = 'both', expand = 1)

        # Create and pack the MessageBar.
        self.messageBar = Pmw.MessageBar(parent,
                entry_width = 40,
                entry_relief='groove',
                labelpos = 'w',
                label_text = 'Status:')
        self.messageBar.pack(fill = 'x', padx = 10, pady = 10)
        self.messageBar.message('state', 'OK')

        buttonBox = Pmw.ButtonBox(parent)
        buttonBox.pack(fill = 'x')
        buttonBox.add('Disable\nall', command = menuBar.disableall)
        buttonBox.add('Enable\nall', command = menuBar.enableall)
        buttonBox.add('Create\nmenu', command = self.add)
        buttonBox.add('Delete\nmenu', command = self.delete)
        buttonBox.add('Create\nitem', command = self.additem)
        buttonBox.add('Delete\nitem', command = self.deleteitem)

        # Configure the balloon to displays its status messages in the
        # message bar.
        self.balloon.configure(statuscommand = self.messageBar.helpmessage)

        self.testMenuList = []

    def _toggleMe(self):
        print(('Toggle value:', self.toggleVar.get()))

    def add(self):
        if len(self.testMenuList) == 0:
            num = 0
        else:
            num = self.testMenuList[-1]
        num = num + 1
        name = 'Menu%d' % num
        self.testMenuList.append(num)

        self.menuBar.addmenu(name, 'This is ' + name)

    def delete(self):
        if len(self.testMenuList) == 0:
            self.menuBar.bell()
        else:
            num = self.testMenuList[0]
            name = 'Menu%d' % num
            del self.testMenuList[0]
            self.menuBar.deletemenu(name)

    def additem(self):
        if len(self.testMenuList) == 0:
            self.menuBar.bell()
        else:
            num = self.testMenuList[-1]
            menuName = 'Menu%d' % num
            menu = self.menuBar.component(menuName + '-menu')
            if menu.index('end') is None:
                label = 'item X'
            else:
                label = menu.entrycget('end', 'label') + 'X'
            self.menuBar.addmenuitem(menuName, 'command', 'Help for ' + label,
                    command = PrintOne('Action: ' + menuName + ': ' + label),
                    label = label)

    def deleteitem(self):
        if len(self.testMenuList) == 0:
            self.menuBar.bell()
        else:
            num = self.testMenuList[-1]
            menuName = 'Menu%d' % num
            menu = self.menuBar.component(menuName + '-menu')
            if menu.index('end') is None:
                self.menuBar.bell()
            else:
                self.menuBar.deletemenuitems(menuName, 0)

class PrintOne:
    def __init__(self, text):
        self.text = text

    def __call__(self):
        print((self.text))

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
