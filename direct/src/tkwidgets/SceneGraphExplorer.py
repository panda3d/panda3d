from PandaObject import *
from Tkinter import *
from Tree import *
import Pmw


class SceneGraphExplorer(Pmw.MegaWidget):
    "Graphical display of a scene graph"
    def __init__(self, root = render, parent = None, **kw):
        # Define the megawidget options.
        optiondefs = (
            ('menuItems',   ['Select'],   None),
            )
        self.defineoptions(kw, optiondefs)
 
        # Initialise superclass
        Pmw.MegaWidget.__init__(self, parent)
        
        # Initialize some class variables
        self.root = root

        # Create the components.
        
        # Setup up container
        interior = self.interior()
        interior.configure(relief = GROOVE, borderwidth = 2)
        
        # Create a label and an entry
        self._scrolledCanvas = self.createcomponent(
            'scrolledCanvas',
            (), None,
            Pmw.ScrolledCanvas, (interior,),
            hull_width = 200, hull_height = 400,
            usehullsize = 1)
        self._canvas = self._scrolledCanvas.component('canvas')
        self._canvas['scrollregion'] = ('0i', '0i', '2i', '4i')
        self._scrolledCanvas.resizescrollregion()
        self._scrolledCanvas.pack(padx = 5, pady = 5, expand=1, fill = BOTH)
        
        self._canvas.bind('<ButtonPress-2>', self.mouse2Down)
        self._canvas.bind('<B2-Motion>', self.mouse2Motion)
        self._canvas.bind('<Configure>',
                          lambda e, sc = self._scrolledCanvas:
                          sc.resizescrollregion())

        # Create the contents
        self._treeItem = SceneGraphExplorerItem(self.root)

        self._node = TreeNode(self._canvas, None, self._treeItem,
                              self['menuItems'])
        self._node.expand()

        # Check keywords and initialise options based on input values.
        self.initialiseoptions(SceneGraphExplorer)

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


class SceneGraphExplorerItem(TreeItem):

    """Example TreeItem subclass -- browse the file system."""

    def __init__(self, nodePath):
        self.nodePath = nodePath

    def GetText(self):
        type = self.nodePath.node().getType().getName()
        name = self.nodePath.getName()
        return type + "  " + name

    def IsEditable(self):
        return issubclass(self.nodePath.node().__class__, NamedNode)

    def SetText(self, text):
        try:
            self.nodePath.node().setName(text)
        except AttributeError:
            pass

    def GetIconName(self):
        if not self.IsExpandable():
            return "sphere2" # XXX wish there was a "file" icon

    def IsExpandable(self):
        return self.nodePath.getNumChildren() != 0

    def GetSubList(self):
        sublist = []
        for nodePath in self.nodePath.getChildrenAsList():
            item = SceneGraphExplorerItem(nodePath)
            sublist.append(item)
        return sublist

    def OnSelect(self):
        messenger.send('SGEFlashNodePath', [self.nodePath])

    def MenuCommand(self, command):
        messenger.send('SGE' + command + 'NodePath', [self.nodePath])


