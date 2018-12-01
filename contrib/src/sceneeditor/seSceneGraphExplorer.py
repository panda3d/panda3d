#################################################################
# seSceneGraphExplorer.py
# Originally from SceneGraphExplorer.py
# Altered by Yi-Hong Lin, yihhongl@andrew.cmu.edu, 2004
#
# we need a customized SceneGraphExplorer.
#
# Do forget to check the seTree.
#
#################################################################
from direct.showbase.DirectObject import DirectObject
from seTree import TreeNode, TreeItem

import Pmw, sys

if sys.version_info >= (3, 0):
    from tkinter import IntVar, Frame, Label
    import tkinter
else:
    from Tkinter import IntVar, Frame, Label
    import Tkinter as tkinter

# changing these strings requires changing sceneEditor.py SGE_ strs too!
# This list of items will be showed on the pop out window when user right click on
# any node on the graph. And, this is also the main reason we decide to copy from
# the original one but not inherited from it.
# Because except drawing part, we have changed a lot of things...
DEFAULT_MENU_ITEMS = [
    'Update Explorer',
    'Separator',
    'Properties',
    'Separator',
    'Duplicate',
    'Remove',
    'Add Dummy',
    'Add Collision Object',
    'Metadata',
    'Separator',
    'Set as Reparent Target',
    'Reparent to Target',
    'Separator',
    'Animation Panel',
    'Blend Animation Panel',
    'MoPath Panel',
    'Align Tool',
    'Separator']

class seSceneGraphExplorer(Pmw.MegaWidget, DirectObject):
    "Graphical display of a scene graph"
    def __init__(self, parent = None, nodePath = render, **kw):
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
        interior.configure(relief = tkinter.GROOVE, borderwidth = 2)

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
        self._scrolledCanvas.pack(padx = 3, pady = 3, expand=1, fill = tkinter.BOTH)

        self._canvas.bind('<ButtonPress-2>', self.mouse2Down)
        self._canvas.bind('<B2-Motion>', self.mouse2Motion)
        self._canvas.bind('<Configure>',
                          lambda e, sc = self._scrolledCanvas:
                          sc.resizescrollregion())
        self.interior().bind('<Destroy>', self.onDestroy)

        # Create the contents
        self._treeItem = SceneGraphExplorerItem(self.nodePath)

        self._node = TreeNode(self._canvas, None, self._treeItem,
                              DEFAULT_MENU_ITEMS + self['menuItems'])
        self._node.expand()

        self._parentFrame = Frame(interior)
        self._label = self.createcomponent(
            'parentLabel',
            (), None,
            Label, (interior,),
            text = 'Active Reparent Target: ',
            anchor = tkinter.W, justify = tkinter.LEFT)
        self._label.pack(fill = tkinter.X)

        # Add update parent label
        def updateLabel(nodePath = None, s = self):
            s._label['text'] = 'Active Reparent Target: ' + nodePath.getName()
        self.accept('DIRECT_activeParent', updateLabel)

        # Add update hook
        self.accept('SGE_Update Explorer',
                    lambda np, s = self: s.update())

        # Check keywords and initialise options based on input values.
        self.initialiseoptions(seSceneGraphExplorer)

    def update(self):
        """ Refresh scene graph explorer """
        self._node.update()

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

    def mouse2Motion(self,event):
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

    def deSelectTree(self):
        self._node.deselecttree()

    def selectNodePath(self,nodePath, callBack=True):
        item = self._node.find(nodePath.get_key())
        if item!= None:
            item.select(callBack)
        else:
            print('----SGE: Error Selection')

class SceneGraphExplorerItem(TreeItem):

    """Example TreeItem subclass -- browse the file system."""

    def __init__(self, nodePath):
        self.nodePath = nodePath

    def GetText(self):
        type = self.nodePath.node().getType().getName()
        name = self.nodePath.getName()
        return type + "  " + name

    def GetTextForEdit(self):
        name = self.nodePath.getName()
        return name

    def GetKey(self):
        return self.nodePath.get_key()

    def IsEditable(self):
        # All nodes' names can be edited nowadays.
        return 1
        #return issubclass(self.nodePath.node().__class__, NamedNode)

    def SetText(self, text):
        try:
            messenger.send('SGE_changeName', [self.nodePath, text])
        except AttributeError:
            pass

    def GetIconName(self):
        return "sphere2" # XXX wish there was a "file" icon

    def IsExpandable(self):
        return self.nodePath.getNumChildren() != 0

    def GetSubList(self):
        sublist = []
        for nodePath in self.nodePath.getChildren():
            item = SceneGraphExplorerItem(nodePath)
            sublist.append(item)
        return sublist

    def OnSelect(self, callback):
        messenger.send('SGE_Flash', [self.nodePath])
        if not callback:
            messenger.send('SGE_madeSelection', [self.nodePath, callback])
        else:
            messenger.send('SGE_madeSelection', [self.nodePath])

    def MenuCommand(self, command):
        messenger.send('SGE_' + command, [self.nodePath])


def explore(nodePath = render):
    tl = Toplevel()
    tl.title('Explore: ' + nodePath.getName())
    sge = seSceneGraphExplorer(parent = tl, nodePath = nodePath)
    sge.pack(expand = 1, fill = 'both')
    return sge
