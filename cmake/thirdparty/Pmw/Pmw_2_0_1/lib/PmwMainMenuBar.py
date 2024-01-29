# Main menubar

import string
import types
import tkinter
import Pmw

class MainMenuBar(Pmw.MegaArchetype):

    def __init__(self, parent = None, **kw):

        # Define the megawidget options.
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('balloon',      None,       None),
            ('hotkeys',      1,          INITOPT),
            ('hull_tearoff', 0,          None),
        )
        self.defineoptions(kw, optiondefs, dynamicGroups = ('Menu',))

        # Initialise the base class (after defining the options).
        Pmw.MegaArchetype.__init__(self, parent, tkinter.Menu)

        self._menuInfo = {}
        self._menuInfo[None] = (None, [])
        # Map from a menu name to a tuple of information about the menu.
        # The first item in the tuple is the name of the parent menu (for
        # toplevel menus this is None). The second item in the tuple is
        # a list of status help messages for each item in the menu.
        # The key for the information for the main menubar is None.

        self._menu = self.interior()
        self._menu.bind('<Leave>', self._resetHelpmessage)
        self._menu.bind('<Motion>',
            lambda event=None, self=self: self._menuHelp(event, None))

        # Check keywords and initialise options.
        self.initialiseoptions()

    def deletemenuitems(self, menuName, start, end = None):
        self.component(menuName).delete(start, end)
        if end is None:
            del self._menuInfo[menuName][1][start]
        else:
            self._menuInfo[menuName][1][start:end+1] = []

    def deletemenu(self, menuName):
        """Delete should be called for cascaded menus before main menus.
        """

        parentName = self._menuInfo[menuName][0]
        del self._menuInfo[menuName]
        if parentName is None:
            parentMenu = self._menu
        else:
            parentMenu = self.component(parentName)

        menu = self.component(menuName)
        menuId = str(menu)
        for item in range(parentMenu.index('end') + 1):
            if parentMenu.type(item) == 'cascade':
                itemMenu = str(parentMenu.entrycget(item, 'menu'))
                if itemMenu == menuId:
                    parentMenu.delete(item)
                    del self._menuInfo[parentName][1][item]
                    break

        self.destroycomponent(menuName)

    def disableall(self):
        for index in range(len(self._menuInfo[None][1])):
            self.entryconfigure(index, state = 'disabled')

    def enableall(self):
        for index in range(len(self._menuInfo[None][1])):
            self.entryconfigure(index, state = 'normal')

    def addmenu(self, menuName, balloonHelp, statusHelp = None,
            traverseSpec = None, **kw):
        if statusHelp is None:
            statusHelp = balloonHelp
        self._addmenu(None, menuName, balloonHelp, statusHelp,
            traverseSpec, kw)

    def addcascademenu(self, parentMenuName, menuName, statusHelp='',
            traverseSpec = None, **kw):
        self._addmenu(parentMenuName, menuName, None, statusHelp,
            traverseSpec, kw)

    def _addmenu(self, parentMenuName, menuName, balloonHelp, statusHelp,
            traverseSpec, kw):

        if (menuName) in self.components():
            raise ValueError('menu "%s" already exists' % menuName)

        menukw = {}
        if 'tearoff' in kw:
            menukw['tearoff'] = kw['tearoff']
            del kw['tearoff']
        else:
            menukw['tearoff'] = 0
        if 'name' in kw:
            menukw['name'] = kw['name']
            del kw['name']

        if 'label' not in kw:
            kw['label'] = menuName

        self._addHotkeyToOptions(parentMenuName, kw, traverseSpec)

        if parentMenuName is None:
            parentMenu = self._menu
            balloon = self['balloon']
            # Bug in Tk: balloon help not implemented
            # if balloon is not None:
            #     balloon.mainmenubind(parentMenu, balloonHelp, statusHelp)
        else:
            parentMenu = self.component(parentMenuName)

        parentMenu.add_cascade(*(), **kw)

        menu = self.createcomponent(*(menuName,
                (), 'Menu',
                tkinter.Menu, (parentMenu,)), **menukw)
        parentMenu.entryconfigure('end', menu = menu)

        self._menuInfo[parentMenuName][1].append(statusHelp)
        self._menuInfo[menuName] = (parentMenuName, [])

        menu.bind('<Leave>', self._resetHelpmessage)
        menu.bind('<Motion>',
            lambda event=None, self=self, menuName=menuName:
                    self._menuHelp(event, menuName))

    def addmenuitem(self, menuName, itemType, statusHelp = '',
            traverseSpec = None, **kw):

        menu = self.component(menuName)
        if itemType != 'separator':
            self._addHotkeyToOptions(menuName, kw, traverseSpec)

        if itemType == 'command':
            command = menu.add_command
        elif itemType == 'separator':
            command = menu.add_separator
        elif itemType == 'checkbutton':
            command = menu.add_checkbutton
        elif itemType == 'radiobutton':
            command = menu.add_radiobutton
        elif itemType == 'cascade':
            command = menu.add_cascade
        else:
            raise ValueError('unknown menuitem type "%s"' % itemType)

        self._menuInfo[menuName][1].append(statusHelp)
        command(*(), **kw)

    def _addHotkeyToOptions(self, menuName, kw, traverseSpec):

        if (not self['hotkeys'] or 'underline' in kw or
                'label' not in kw):
            return

        if type(traverseSpec) == int:
            kw['underline'] = traverseSpec
            return

        if menuName is None:
            menu = self._menu
        else:
            menu = self.component(menuName)
        hotkeyList = []
        end = menu.index('end')
        if end is not None:
            for item in range(end + 1):
                if menu.type(item) not in ('separator', 'tearoff'):
                    #Python 3 conversion
#                    underline = \
#                            string.atoi(str(menu.entrycget(item, 'underline')))
                    underline = \
                            int(str(menu.entrycget(item, 'underline')))
                    if underline != -1:
                        label = str(menu.entrycget(item, 'label'))
                        if underline < len(label):
                            hotkey = label[underline].lower()
                            if hotkey not in hotkeyList:
                                hotkeyList.append(hotkey)

        name = kw['label']

        if type(traverseSpec) is str:
            lowerLetter = traverseSpec.lower()
            if traverseSpec in name and lowerLetter not in hotkeyList:
                kw['underline'] = name.index(traverseSpec)
        else:
            targets = string.digits + string.ascii_letters
            lowerName = name.lower()
            for letter_index in range(len(name)):
                letter = lowerName[letter_index]
                if letter in targets and letter not in hotkeyList:
                    kw['underline'] = letter_index
                    break

    def _menuHelp(self, event, menuName):
        if menuName is None:
            menu = self._menu
            index = menu.index('@%d'% event.x)
        else:
            menu = self.component(menuName)
            index = menu.index('@%d'% event.y)

        balloon = self['balloon']
        if balloon is not None:
            if index is None:
                balloon.showstatus('')
            else:
                if str(menu.cget('tearoff')) == '1':
                    index = index - 1
                if index >= 0:
                    help = self._menuInfo[menuName][1][index]
                    balloon.showstatus(help)

    def _resetHelpmessage(self, event=None):
        balloon = self['balloon']
        if balloon is not None:
            balloon.clearstatus()

Pmw.forwardmethods(MainMenuBar, tkinter.Menu, '_hull')
