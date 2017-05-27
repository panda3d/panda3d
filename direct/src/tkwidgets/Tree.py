"""Defines tree widgets for the tkinter GUI system."""

__all__ = ['TreeNode', 'TreeItem']

# ADAPTED FROM IDLE TreeWidget.py
# XXX TO DO:
# - popup menu
# - support partial or total redisplay
# - key bindings (instead of quick-n-dirty bindings on Canvas):
#   - up/down arrow keys to move focus around
#   - ditto for page up/down, home/end
#   - left/right arrows to expand/collapse and move out/in
# - more doc strings
# - add icons for "file", "module", "class", "method"; better "python" icon
# - callback for selection???
# - multiple-item selection
# - tooltips
# - redo geometry without magic numbers
# - keep track of object ids to allow more careful cleaning
# - optimize tree redraw after expand of subnode

import os
from direct.showbase.TkGlobal import *
from panda3d.core import *

# Initialize icon directory
ICONDIR = ConfigVariableSearchPath('model-path').findFile(Filename('icons')).toOsSpecific()
if not os.path.isdir(ICONDIR):
    raise RuntimeError("can't find DIRECT icon directory (%s)" % repr(ICONDIR))

class TreeNode:

    def __init__(self, canvas, parent, item, menuList = []):
        self.canvas = canvas
        self.parent = parent
        self.item = item
        self.state = 'collapsed'
        self.selected = 0
        self.children = {}
        self.kidKeys = []
        self.x = self.y = None
        self.iconimages = {} # cache of PhotoImage instances for icons
        self.menuList = menuList
        if self.menuList:
            if self.menuList[-1] == 'Separator':
                self.menuList = self.menuList[:-1]
        self.menuVar = IntVar()
        self.menuVar.set(0)
        self._popupMenu = None
        self.fSortChildren = False # [gjeon] flag for sorting children or not
        self.fModeChildrenTag = 0 # [gjeon] flag for using filter or not
        self.childrenTag = None # [gjeon] filter dictionary for
        self.setAsTarget = 0 # [gjeon] to visualize reparent target

    # [gjeon] to set fSortChildren
    def setFSortChildren(self, fSortChildren):
        self.fSortChildren = fSortChildren

    def setChildrenTag(self, tag, fModeChildrenTag):
        self.childrenTag = tag
        self.fModeChildrenTag = fModeChildrenTag

    def destroy(self):
        if self._popupMenu:
            self._popupMenu.destroy()
        for key in self.kidKeys:
            c = self.children[key]
            del self.children[key]
            c.destroy()
        self.parent = None

    def geticonimage(self, name):
        try:
            return self.iconimages[name]
        except KeyError:
            pass
        file, ext = os.path.splitext(name)
        ext = ext or ".gif"
        fullname = os.path.join(ICONDIR, file + ext)
        image = PhotoImage(master=self.canvas, file=fullname)
        self.iconimages[name] = image
        return image

    def select(self, event=None):
        if self.selected:
            return
        self.deselectall()
        self.selected = 1
        self.canvas.delete(self.image_id)
        self.drawicon()
        self.drawtext()
        self.item.OnSelect()

    def deselect(self, event=None):
        if not self.selected:
            return
        self.selected = 0
        self.canvas.delete(self.image_id)
        self.drawicon()
        self.drawtext()

    def deselectall(self):
        if self.parent:
            self.parent.deselectall()
        else:
            self.deselecttree()

    def deselecttree(self):
        if self.selected:
            self.deselect()
        for key in self.kidKeys:
            child = self.children[key]
            child.deselecttree()

    def flip(self, event=None):
        if self.state == 'expanded':
            self.collapse()
        else:
            self.expand()
        self.item.OnDoubleClick()
        return "break"

    def createPopupMenu(self):
        if self.menuList:
            self._popupMenu = Menu(self.canvas, tearoff = 0)
            for i in range(len(self.menuList)):
                item = self.menuList[i]
                if item == 'Separator':
                    self._popupMenu.add_separator()
                else:
                    self._popupMenu.add_radiobutton(
                        label = item,
                        variable = self.menuVar,
                        value = i,
                        indicatoron = 0,
                        command = self.popupMenuCommand)

    def popupMenu(self, event=None):
        if not self._popupMenu:
            self.createPopupMenu()
        if self._popupMenu:
            self._popupMenu.post(event.widget.winfo_pointerx(),
                                 event.widget.winfo_pointery())
            return "break"

    def popupMenuCommand(self):
        command = self.menuList[self.menuVar.get()]

        if (command == 'Expand All'):
            self.updateAll(1)
        elif (command == 'Collapse All'):
            self.updateAll(0)
        else:
            skipUpdate = self.item.MenuCommand(command)
            if not skipUpdate and self.parent and (command != 'Update Explorer'):
                # Update parent to try to keep explorer up to date
                self.parent.update()

    def expand(self, event=None):
        if not self.item.IsExpandable():
            return
        if self.state != 'expanded':
            self.state = 'expanded'
            self.update()
            self.view()

    def collapse(self, event=None):
        if self.state != 'collapsed':
            self.state = 'collapsed'
            self.update()

    def view(self):
        top = self.y - 2
        bottom = self.lastvisiblechild().y + 17
        height = bottom - top
        visible_top = self.canvas.canvasy(0)
        visible_height = self.canvas.winfo_height()
        visible_bottom = self.canvas.canvasy(visible_height)
        if visible_top <= top and bottom <= visible_bottom:
            return
        x0, y0, x1, y1 = self.canvas._getints(self.canvas['scrollregion'])
        if top >= visible_top and height <= visible_height:
            fraction = top + height - visible_height
        else:
            fraction = top
        fraction = float(fraction) / y1
        self.canvas.yview_moveto(fraction)

    def reveal(self):
        # Make sure all parent nodes are marked as expanded
        parent = self.parent
        while parent:
            if parent.state == 'collapsed':
                parent.state = 'expanded'
                parent = parent.parent
            else:
                break
        # Redraw tree accordingly
        self.update()
        # Bring this item into view
        self.view()

    def lastvisiblechild(self):
        if self.kidKeys and self.state == 'expanded':
            return self.children[self.kidKeys[-1]].lastvisiblechild()
        else:
            return self

    # [gjeon] function to expand or collapse all the tree nodes
    def updateAll(self, fMode, depth = 0, fUseCachedChildren = 1):
        depth = depth + 1
        if not self.item.IsExpandable():
            return
        if fMode:
            self.state = 'expanded'
        else:
            if depth > 1:
                self.state = 'collapsed'

        sublist = self.item._GetSubList()
        if not sublist:
            return
        self.kidKeys = []
        for item in sublist:
            key = item.GetKey()
            if fUseCachedChildren and key in self.children:
                child = self.children[key]
            else:
                child = TreeNode(self.canvas, self, item, self.menuList)

            self.children[key] = child
            self.kidKeys.append(key)

        # Remove unused children
        for key in list(self.children.keys()):
            if key not in self.kidKeys:
                del(self.children[key])

        for key in self.kidKeys:
            child = self.children[key]
            child.updateAll(fMode, depth=depth)

        # [gjeon] to update the tree one time only
        if depth == 1:
            self.update()
            self.view()

    def update(self, fUseCachedChildren = 1, fExpandMode = 0):
        if self.parent:
            self.parent.update(fUseCachedChildren, fExpandMode = fExpandMode)
        else:
            oldcursor = self.canvas['cursor']
            self.canvas['cursor'] = "watch"
            self.canvas.update()
            self.canvas.delete(ALL)     # XXX could be more subtle
            self.draw(7, 2, fUseCachedChildren)
            x0, y0, x1, y1 = self.canvas.bbox(ALL)
            self.canvas.configure(scrollregion=(0, 0, x1, y1))
            self.canvas['cursor'] = oldcursor

    def draw(self, x, y, fUseCachedChildren = 1):
        # XXX This hard-codes too many geometry constants!
        self.x, self.y = x, y
        self.drawicon()
        self.drawtext()

        if self.state != 'expanded':
            return y+17
        # draw children
        sublist = self.item._GetSubList()
        if not sublist:
            # IsExpandable() was mistaken; that's allowed
            return y+17
        self.kidKeys = []

        # [gjeon] to sort children
        if self.fSortChildren:
            def compareText(x, y):
                textX = x.GetText()
                textY = y.GetText()
                if (textX > textY):
                    return 1
                elif (textX == textY):
                    return 0
                else: # textX < textY
                    return -1
            sublist.sort(compareText)
        for item in sublist:
            key = item.GetKey()
            if fUseCachedChildren and key in self.children:
                child = self.children[key]
            else:
                child = TreeNode(self.canvas, self, item, self.menuList)

            # [gjeon] to set flag recursively
            child.setFSortChildren(self.fSortChildren)
            child.setChildrenTag(self.childrenTag, self.fModeChildrenTag)

            self.children[key] = child
            self.kidKeys.append(key)

            # [gjeon] to filter by given tag
            if self.fModeChildrenTag:
                if self.childrenTag:
                    showThisItem = False
                    for tagKey in list(self.childrenTag.keys()):
                        if item.nodePath.hasTag(tagKey):
                            showThisItem = self.childrenTag[tagKey]
                    if not showThisItem:
                        self.kidKeys.remove(key)

        # Remove unused children
        for key in list(self.children.keys()):
            if key not in self.kidKeys:
                del(self.children[key])
        cx = x+20
        cy = y+17
        cylast = 0
        for key in self.kidKeys:
            child = self.children[key]
            cylast = cy
            self.canvas.create_line(x+9, cy+7, cx, cy+7, fill="gray50")
            cy = child.draw(cx, cy, fUseCachedChildren)
            if child.item.IsExpandable():
                if child.state == 'expanded':
                    iconname = "minusnode"
                    callback = child.collapse
                else:
                    iconname = "plusnode"
                    callback = child.expand
                image = self.geticonimage(iconname)
                id = self.canvas.create_image(x+9, cylast+7, image=image)
                # XXX This leaks bindings until canvas is deleted:
                self.canvas.tag_bind(id, "<1>", callback)
                self.canvas.tag_bind(id, "<Double-1>", lambda x: None)
        id = self.canvas.create_line(x+9, y+10, x+9, cylast+7,
            ##stipple="gray50",     # XXX Seems broken in Tk 8.0.x
            fill="gray50")
        self.canvas.tag_lower(id) # XXX .lower(id) before Python 1.5.2
        return cy

    def drawicon(self):
        if self.selected:
            imagename = (self.item.GetSelectedIconName() or
                         self.item.GetIconName() or
                         "openfolder")
        else:
            imagename = self.item.GetIconName() or "folder"
        image = self.geticonimage(imagename)
        id = self.canvas.create_image(self.x, self.y, anchor="nw", image=image)
        self.image_id = id
        self.canvas.tag_bind(id, "<1>", self.select)
        self.canvas.tag_bind(id, "<Double-1>", self.flip)
        self.canvas.tag_bind(id, "<3>", self.popupMenu)

    def drawtext(self):
        textx = self.x+20-1
        texty = self.y-1
        labeltext = self.item.GetLabelText()
        if labeltext:
            id = self.canvas.create_text(textx, texty, anchor="nw",
                                         text=labeltext)
            self.canvas.tag_bind(id, "<1>", self.select)
            self.canvas.tag_bind(id, "<Double-1>", self.flip)
            x0, y0, x1, y1 = self.canvas.bbox(id)
            textx = max(x1, 200) + 10
        text = self.item.GetText() or "<no text>"
        try:
            self.entry
        except AttributeError:
            pass
        else:
            self.edit_finish()
        try:
            label = self.label
        except AttributeError:
            # padding carefully selected (on Windows) to match Entry widget:
            self.label = Label(self.canvas, text=text, bd=0, padx=2, pady=2)
        if self.selected:
            self.label.configure(fg="white", bg="darkblue")
        elif self.setAsTarget:
            self.label.configure(fg="white", bg="red")
        else:
            fg = self.item.GetTextFg()
            bg = self.item.GetTextBg()
            self.label.configure(fg=fg, bg=bg)
        id = self.canvas.create_window(textx, texty,
                                       anchor="nw", window=self.label)
        self.label.bind("<1>", self.select_or_edit)
        self.label.bind("<Double-1>", self.flip)
        self.label.bind("<3>", self.popupMenu)
        # Update text if necessary
        if text != self.label['text']:
            self.label['text'] = text
        self.text_id = id

    def select_or_edit(self, event=None):
        if self.selected and self.item.IsEditable():
            self.edit(event)
        else:
            self.select(event)

    def edit(self, event=None):
        self.entry = Entry(self.label, bd=0, highlightthickness=1, width=0)
        self.entry.insert(0, self.label['text'])
        self.entry.selection_range(0, END)
        self.entry.pack(ipadx=5)
        self.entry.focus_set()
        self.entry.bind("<Return>", self.edit_finish)
        self.entry.bind("<Escape>", self.edit_cancel)

    def edit_finish(self, event=None):
        try:
            entry = self.entry
            del self.entry
        except AttributeError:
            return
        text = entry.get()
        entry.destroy()
        if text and text != self.item.GetText():
            self.item.SetText(text)
        text = self.item.GetText()
        self.label['text'] = text
        self.drawtext()
        self.canvas.focus_set()

    def edit_cancel(self, event=None):
        self.drawtext()
        self.canvas.focus_set()

    def find(self, searchKey):
        # Search for a node who's key matches the given key
        # Is it this node
        if searchKey == self.item.GetKey():
            # [gjeon] to filter by given tag
            if self.fModeChildrenTag:
                if self.childrenTag:
                    showThisItem = False
                    for tagKey in list(self.childrenTag.keys()):
                        if self.item.nodePath.hasTag(tagKey):
                            showThisItem = self.childrenTag[tagKey]
                    if not showThisItem:
                        return None
            return self
        # Nope, check the children
        sublist = self.item._GetSubList()
        for item in sublist:
            key = item.GetKey()

            # Use existing child or create new TreeNode if none exists
            if key in self.children:
                child = self.children[key]
            else:
                child = TreeNode(self.canvas, self, item, self.menuList)
                # Update local list of children and keys
                self.children[key] = child
                self.kidKeys.append(key)
            # [gjeon] to set flag recursively
            child.setChildrenTag(self.childrenTag, self.fModeChildrenTag)

            # See if node is child (or one of child's descendants)
            retVal = child.find(searchKey)
            if retVal:
                return retVal
        # Not here
        return None

class TreeItem:

    """Abstract class representing tree items.

    Methods should typically be overridden, otherwise a default action
    is used.

    """

    def __init__(self):
        """Constructor.  Do whatever you need to do."""

    def GetText(self):
        """Return text string to display."""

    def GetTextFg(self):
        return "black"

    def GetTextBg(self):
        return "white"

    def GetLabelText(self):
        """Return label text string to display in front of text (if any)."""

    def IsExpandable(self):
        """Return whether there are subitems."""
        return 1

    def _GetSubList(self):
        """Do not override!  Called by TreeNode."""
        if not self.IsExpandable():
            return []
        sublist = self.GetSubList()
        return sublist

    def IsEditable(self):
        """Return whether the item's text may be edited."""

    def SetText(self, text):
        """Change the item's text (if it is editable)."""

    def GetIconName(self):
        """Return name of icon to be displayed normally."""

    def GetSelectedIconName(self):
        """Return name of icon to be displayed when selected."""

    def GetSubList(self):
        """Return list of items forming sublist."""

    def OnDoubleClick(self):
        """Called on a double-click on the item."""

    def OnSelect(self):
        """Called when item selected."""



