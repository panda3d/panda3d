# Manager widget for menus.

import string
import types
import Tkinter
import Pmw

class MenuBar(Pmw.MegaWidget):

    def __init__(self, parent = None, **kw):

        # Define the megawidget options.
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('balloon',      None,       None),
            ('hotkeys',      1,          INITOPT),
            ('padx',         0,          INITOPT),
        )
        self.defineoptions(kw, optiondefs, dynamicGroups = ('Menu', 'Button'))

        # Initialise the base class (after defining the options).
        Pmw.MegaWidget.__init__(self, parent)

        self._menuInfo = {}
        # Map from a menu name to a tuple of information about the menu.
        # The first item in the tuple is the name of the parent menu (for
        # toplevel menus this is None). The second item in the tuple is
        # a list of status help messages for each item in the menu.
        # The third item in the tuple is the id of the binding used
        # to detect mouse motion to display status help.
        # Information for the toplevel menubuttons is not stored here.

        self._mydeletecommand = self.component('hull').tk.deletecommand
        # Cache this method for use later.

        # Check keywords and initialise options.
        self.initialiseoptions()

    def deletemenuitems(self, menuName, start, end = None):
        self.component(menuName + '-menu').delete(start, end)
        if end is None:
            del self._menuInfo[menuName][1][start]
        else:
            self._menuInfo[menuName][1][start:end+1] = []

    def deletemenu(self, menuName):
        """Delete should be called for cascaded menus before main menus.
        """

        # Clean up binding for this menu.
        parentName = self._menuInfo[menuName][0]
        bindId = self._menuInfo[menuName][2]
        _bindtag = 'PmwMenuBar' + str(self) + menuName
        self.unbind_class(_bindtag, '<Motion>')
        self._mydeletecommand(bindId) # unbind_class does not clean up
        del self._menuInfo[menuName]

        if parentName is None:
            self.destroycomponent(menuName + '-button')
        else:
            parentMenu = self.component(parentName + '-menu')

            menu = self.component(menuName + '-menu')
            menuId = str(menu)
            for item in range(parentMenu.index('end') + 1):
                if parentMenu.type(item) == 'cascade':
                    itemMenu = str(parentMenu.entrycget(item, 'menu'))
                    if itemMenu == menuId:
                        parentMenu.delete(item)
                        del self._menuInfo[parentName][1][item]
                        break

        self.destroycomponent(menuName + '-menu')

    def disableall(self):
        for menuName in self._menuInfo.keys():
            if self._menuInfo[menuName][0] is None:
                menubutton = self.component(menuName + '-button')
                menubutton.configure(state = 'disabled')

    def enableall(self):
        for menuName in self._menuInfo.keys():
            if self._menuInfo[menuName][0] is None:
                menubutton = self.component(menuName + '-button')
                menubutton.configure(state = 'normal')

    def addmenu(self, menuName, balloonHelp, statusHelp = None,
            side = 'left', traverseSpec = None, **kw):

        self._addmenu(None, menuName, balloonHelp, statusHelp,
            traverseSpec, side, 'text', kw)

    def addcascademenu(self, parentMenuName, menuName, statusHelp = '',
            traverseSpec = None, **kw):

        self._addmenu(parentMenuName, menuName, None, statusHelp,
            traverseSpec, None, 'label', kw)

    def _addmenu(self, parentMenuName, menuName, balloonHelp, statusHelp,
            traverseSpec, side, textKey, kw):

        if (menuName + '-menu') in self.components():
            raise ValueError, 'menu "%s" already exists' % menuName

        menukw = {}
        if kw.has_key('tearoff'):
            menukw['tearoff'] = kw['tearoff']
            del kw['tearoff']
        else:
            menukw['tearoff'] = 0

        if not kw.has_key(textKey):
            kw[textKey] = menuName

        self._addHotkeyToOptions(parentMenuName, kw, textKey, traverseSpec)

        if parentMenuName is None:
            button = apply(self.createcomponent, (menuName + '-button',
                    (), 'Button',
                    Tkinter.Menubutton, (self.interior(),)), kw)
            button.pack(side=side, padx = self['padx'])
            balloon = self['balloon']
            if balloon is not None:
                balloon.bind(button, balloonHelp, statusHelp)
            parentMenu = button
        else:
            parentMenu = self.component(parentMenuName + '-menu')
            apply(parentMenu.add_cascade, (), kw)
            self._menuInfo[parentMenuName][1].append(statusHelp)

        menu = apply(self.createcomponent, (menuName + '-menu',
                (), 'Menu',
                Tkinter.Menu, (parentMenu,)), menukw)
        if parentMenuName is None:
            button.configure(menu = menu)
        else:
            parentMenu.entryconfigure('end', menu = menu)

        # Need to put this binding after the class bindings so that
        # menu.index() does not lag behind.
        _bindtag = 'PmwMenuBar' + str(self) + menuName
        bindId = self.bind_class(_bindtag, '<Motion>',
            lambda event=None, self=self, menuName=menuName:
                    self._menuHelp(menuName))
        menu.bindtags(menu.bindtags() + (_bindtag,))
        menu.bind('<Leave>', self._resetHelpmessage)

        self._menuInfo[menuName] = (parentMenuName, [], bindId)

    def addmenuitem(self, menuName, itemType, statusHelp = '',
            traverseSpec = None, **kw):

        menu = self.component(menuName + '-menu')
        if itemType != 'separator':
            self._addHotkeyToOptions(menuName, kw, 'label', traverseSpec)

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
            raise ValueError, 'unknown menuitem type "%s"' % itemType

        self._menuInfo[menuName][1].append(statusHelp)
        apply(command, (), kw)

    def _addHotkeyToOptions(self, menuName, kw, textKey, traverseSpec):

        if (not self['hotkeys'] or kw.has_key('underline') or
                not kw.has_key(textKey)):
            return

        if type(traverseSpec) == types.IntType:
            kw['underline'] = traverseSpec
            return

        hotkeyList = []
        if menuName is None:
            for menuName in self._menuInfo.keys():
                if self._menuInfo[menuName][0] is None:
                    menubutton = self.component(menuName + '-button')
                    underline = string.atoi(str(menubutton.cget('underline')))
                    if underline != -1:
                        label = str(menubutton.cget(textKey))
                        if underline < len(label):
                            hotkey = string.lower(label[underline])
                            if hotkey not in hotkeyList:
                                hotkeyList.append(hotkey)
        else:
            menu = self.component(menuName + '-menu')
            end = menu.index('end')
            if end is not None:
                for item in range(end + 1):
                    if menu.type(item) not in ('separator', 'tearoff'):
                        underline = string.atoi(
                            str(menu.entrycget(item, 'underline')))
                        if underline != -1:
                            label = str(menu.entrycget(item, textKey))
                            if underline < len(label):
                                hotkey = string.lower(label[underline])
                                if hotkey not in hotkeyList:
                                    hotkeyList.append(hotkey)

        name = kw[textKey]

        if type(traverseSpec) == types.StringType:
            lowerLetter = string.lower(traverseSpec)
            if traverseSpec in name and lowerLetter not in hotkeyList:
                kw['underline'] = string.index(name, traverseSpec)
        else:
            targets = string.digits + string.letters
            lowerName = string.lower(name)
            for letter_index in range(len(name)):
                letter = lowerName[letter_index]
                if letter in targets and letter not in hotkeyList:
                    kw['underline'] = letter_index
                    break

    def _menuHelp(self, menuName):
        menu = self.component(menuName + '-menu')
        index = menu.index('active')

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
