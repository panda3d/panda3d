"""Undocumented Module"""

__all__ = ['SceneGraphExplorer', 'SceneGraphExplorerItem', 'explore']

from direct.showbase.DirectObject import DirectObject
from direct.showbase.TkGlobal import *
from Tkinter import *
from Tree import *
import Pmw

# changing these strings requires changing DirectSession.py SGE_ strs too!
DEFAULT_MENU_ITEMS = [
    'Update Explorer',
    'Expand All',
    'Collapse All',
    'Separator',
    'Select', 'Deselect',
    'Separator',
    'Delete',
    'Separator',
    'Fit', 'Flash', 'Isolate', 'Toggle Vis', 'Show All',
    'Separator',
    'Set Reparent Target', 'Reparent', 'WRT Reparent',
    'Separator',
    'Place', 'Set Name', 'Set Color', 'Explore',
    'Separator']

class SceneGraphExplorer(Pmw.MegaWidget, DirectObject):
    "Graphical display of a scene graph"
    def __init__(self, parent = None, nodePath = render, isItemEditable = True, **kw):
        # Define the megawidget options.
        optiondefs = (
            ('menuItems',   [],   Pmw.INITOPT),
            )
        self.defineoptions(kw, optiondefs)

        # Initialise superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Initialize some class variables
        self.nodePath = nodePath

        # Create the components.

        # Setup up container
        interior = self.interior()
        interior.configure(relief = GROOVE, borderwidth = 2)

        # Create a label and an entry
        self._scrolledCanvas = self.createcomponent(
            'scrolledCanvas',
            (), None,
            Pmw.ScrolledCanvas, (interior,),
            hull_width = 200, hull_height = 300,
            usehullsize = 1)
        self._canvas = self._scrolledCanvas.component('canvas')
        self._canvas['scrollregion'] = ('0i', '0i', '2i', '4i')
        self._scrolledCanvas.resizescrollregion()
        self._scrolledCanvas.pack(padx = 3, pady = 3, expand=1, fill = BOTH)

        self._canvas.bind('<ButtonPress-2>', self.mouse2Down)
        self._canvas.bind('<B2-Motion>', self.mouse2Motion)
        self._canvas.bind('<Configure>',
                          lambda e, sc = self._scrolledCanvas:
                          sc.resizescrollregion())
        self.interior().bind('<Destroy>', self.onDestroy)

        # Create the contents
        self._treeItem = SceneGraphExplorerItem(self.nodePath, isItemEditable)

        self._node = TreeNode(self._canvas, None, self._treeItem,
                              DEFAULT_MENU_ITEMS + self['menuItems'])
        self._node.expand()

        self._parentFrame = Frame(interior)
        self._label = self.createcomponent(
            'parentLabel',
            (), None,
            Label, (interior,),
            text = 'Active Reparent Target: ',
            anchor = W, justify = LEFT)
        self._label.pack(fill = X)

        # Add update parent label
        def updateLabel(nodePath = None, s = self):
            s._label['text'] = 'Active Reparent Target: ' + nodePath.getName()
        self.accept('DIRECT_activeParent', updateLabel)

        # Add update hook
        self.accept('SGE_Update Explorer',
                    lambda np, s = self: s.update())

        # Check keywords and initialise options based on input values.
        self.initialiseoptions(SceneGraphExplorer)

    # [gjeon] to set childrenTag and fModeChildrenTag of tree node
    def setChildrenTag(self, tag, fModeChildrenTag):
        self._node.setChildrenTag(tag, fModeChildrenTag)
        self._node.update()

    # [gjeon] to set fSortChildren of tree node
    def setFSortChildren(self, fSortChildren):
        self._node.setFSortChildren(fSortChildren)
        self._node.update()

    def update(self, fUseCachedChildren = 1):
        """ Refresh scene graph explorer """
        self._node.update(fUseCachedChildren)

    def mouse2Down(self, event):
        self._width = 1.0 * self._canvas.winfo_width()
        self._height = 1.0 * self._canvas.winfo_height()
        xview = self._canvas.xview()
        yview = self._canvas.yview()
        self._left = xview[0]
        self._top = yview[0]
        self._dxview = xview[1] - xview[0]
        self._dyview = yview[1] - yview[0]
        self._2lx = event.x
        self._2ly = event.y

    def mouse2Motion(self, event):
        newx = self._left - ((event.x - self._2lx)/self._width) * self._dxview
        self._canvas.xview_moveto(newx)
        newy = self._top - ((event.y - self._2ly)/self._height) * self._dyview
        self._canvas.yview_moveto(newy)
        self._2lx = event.x
        self._2ly = event.y
        self._left = self._canvas.xview()[0]
        self._top = self._canvas.yview()[0]

    def onDestroy(self, event):
        # Remove hooks
        self.ignore('DIRECT_activeParent')
        self.ignore('SGE_Update Explorer')
   
    def updateSelection(self, searchKey):
        # [gjeon] update SGE selection with directSession
        sceneGraphItem = self._node.find(searchKey)
        if sceneGraphItem:
            sceneGraphItem.reveal()
            sceneGraphItem.select()

class SceneGraphExplorerItem(TreeItem):

    """Example TreeItem subclass -- browse the file system."""

    def __init__(self, nodePath, isItemEditable = True):
        self.nodePath = nodePath
        self.isItemEditable = isItemEditable

    def GetText(self):
        type = self.nodePath.node().getType().getName()
        name = self.nodePath.getName()
        return type + "  " + name

    def GetKey(self):
        return self.nodePath.id()

    def IsEditable(self):
        # All nodes' names can be edited nowadays.
        return self.isItemEditable
        #return issubclass(self.nodePath.node().__class__, NamedNode)

    def SetText(self, text):
        try:
            self.nodePath.setName(text)
        except AttributeError:
            pass

    def GetIconName(self):
        return "sphere2" # XXX wish there was a "file" icon

    def IsExpandable(self):
        return self.nodePath.getNumChildren() != 0

    def GetSubList(self):
        sublist = []
        for nodePath in self.nodePath.getChildren():
            item = SceneGraphExplorerItem(nodePath, self.isItemEditable)
            sublist.append(item)
        return sublist

    def OnSelect(self):
        messenger.send('SGE_Flash', [self.nodePath])

    def MenuCommand(self, command):
        messenger.send('SGE_' + command, [self.nodePath])


def explore(nodePath = render):
    tl = Toplevel()
    tl.title('Explore: ' + nodePath.getName())
    sge = SceneGraphExplorer(parent = tl, nodePath = nodePath)
    sge.pack(expand = 1, fill = 'both')
    return sge
