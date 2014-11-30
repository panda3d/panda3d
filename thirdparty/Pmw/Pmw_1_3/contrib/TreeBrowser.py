#
#  FILE: TreeBrowser.py
#
#  DESCRIPTION:
#    This file provides a generic hierarchical tree browser widget.
#
#  AUTHOR:  Steve Kinneberg <skinneberg@mvista.com>,
#           MontaVista Software, Inc. <source@mvista.com>
#
#  Copyright 2001 MontaVista Software Inc.
#
#  This program is free software; you can redistribute  it and/or modify it
#  under  the terms of  the GNU General  Public License as published by the
#  Free Software Foundation;  either version 2 of the  License, or (at your
#  option) any later version.
#
#  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
#  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
#  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
#  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
#  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  You should have received a copy of the  GNU General Public License along
#  with this program; if not, write  to the Free Software Foundation, Inc.,
#  675 Mass Ave, Cambridge, MA 02139, USA.
#


import types
import Tkinter
import Pmw


class _Branching:
    def __init__(self):
        # List of branch names
        self._nodeNames = []

        # Map from branch name to branch info
        #   branch      Either _LeafNode or _BranchNode widget of the branch
        #   nodetype    Either 'TreeNode' or 'LeafNode'
        self._nodeAttrs = {}

    def addbranch(self, branchName = None, **kw):
        kw['indent'] = self['indent']
        return apply(self._insertnode,
                     ('tree', branchName, len(self._nodeNames),
                      self._treeRoot),
                     kw)

    def addleaf(self, leafName = None, **kw):
        return apply(self._insertnode,
                     ('leaf', leafName, len(self._nodeNames),
                      self._treeRoot),
                     kw)

    def insertbranch(self, branchName = None, before = 0, **kw):
        kw['indent'] = self['indent']
        return apply(self._insertnode,
                     ('tree', branchName, before, self._treeRoot),
                     kw)
    
    def insertleaf(self, leafName = None, before = 0, **kw):
        return apply(self._insertnode,
                     ('leaf', leafName, before, self._treeRoot),
                     kw)

    def _insertnode(self, type, nodeName, before, treeRoot, **kw):
        if 'selectbackground' not in kw.keys():
            kw['selectbackground'] = self['selectbackground']

        if 'selectforeground' not in kw.keys():
            kw['selectforeground'] = self['selectforeground']

        if 'background' not in kw.keys():
            kw['background'] = self['background']

        if 'foreground' not in kw.keys():
            kw['foreground'] = self['foreground']

        if nodeName == None:
            nodeName = self._nodeName + ".%d" % (len(self._nodeNames) + 1)
        
	if self._nodeAttrs.has_key(nodeName):
	    msg = 'Node "%s" already exists.' % nodeName
	    raise ValueError, msg

        # Do this early to catch bad <before> spec before creating any items.
	beforeIndex = self.index(before, 1)
        attributes = {}

        last = (beforeIndex == len(self._nodeNames))
        if last and len(self._nodeNames) > 0:
            # set the previous node to not last
            self._nodeAttrs[self._nodeNames[-1]]['branch']._setlast(0)
            
        if(type == 'tree'):
            node = apply(self.createcomponent, ('branch%d'%len(self._nodeNames),
                                                (), None,
                                                _BranchNode,
                                                self._branchFrame,
                                                nodeName,
                                                treeRoot,
                                                self,
                                                last,
                                                ), kw)
            attributes['nodetype'] = 'TreeNode'
        else:
            node = apply(self.createcomponent, ('leaf%d'%len(self._nodeNames),
                                                (), None,
                                                _LeafNode,
                                                self._branchFrame,
                                                nodeName,
                                                treeRoot,
                                                self,
                                                last,
                                                ), kw)
            attributes['nodetype'] = 'LeafNode'

        if len(self._nodeNames) == beforeIndex:
            node.pack(anchor='w')
        else:
            bname = self._nodeNames[beforeIndex]
            battrs = self._nodeAttrs[bname]
            node.pack(anchor='w', before=battrs['branch'])

        attributes['branch'] = node

        self._nodeAttrs[nodeName] = attributes
        self._nodeNames.insert(beforeIndex, nodeName)
        self._sizechange()
        return node

    def delete(self, *nodes):
        curSel = self._treeRoot.curselection()[0]
        for node in nodes:
            index = self.index(node)
            name = self._nodeNames.pop(index)
            dnode = self._nodeAttrs[name]['branch']
            del self._nodeAttrs[name]
            if dnode == curSel:
                self._treeRoot._unhightlightnode(dnode)
            dnode.destroy()
        self._sizechange()

    def destroy(self):
        for node in len(self._nodeNames):
            self.delete(node)
        Pmw.MegaWidget.destroy(self)

    def index(self, index, forInsert = 0):
        if isinstance(index, _LeafNode):
            index = index._nodeName
	listLength = len(self._nodeNames)
	if type(index) == types.IntType:
	    if forInsert and index <= listLength:
		return index
	    elif not forInsert and index < listLength:
		return index
	    else:
		raise ValueError, 'index "%s" is out of range' % index
        elif type(index) == types.StringType:
            if index in self._nodeNames:
                return self._nodeNames.index(index)
            raise ValueError, 'bad branch or leaf name: %s' % index
	elif index is Pmw.END:
	    if forInsert:
		return listLength
	    elif listLength > 0:
		return listLength - 1
	    else:
		raise ValueError, 'TreeNode has no branches'
	#elif index is Pmw.SELECT:
	#    if listLength == 0:
	#	raise ValueError, 'TreeNode has no branches'
        #    return self._pageNames.index(self.getcurselection())
	else:
	    validValues = 'a name, a number, Pmw.END, Pmw.SELECT, or a reference to a TreeBrowser Leaf or Branch'
	    raise ValueError, \
                'bad index "%s": must be %s' % (index, validValues)

    def getnodenames(self):
        return self._nodeNames

    def getnode(self, node):
        nodeName = self._nodeNames[self.index(node)]
        return self._nodeAttrs[nodeName]['branch']


class _LeafNode(Pmw.MegaWidget):
    
    def __init__(self, parent, nodeName, treeRoot, parentnode, last = 1, **kw):
        colors = Pmw.Color.getdefaultpalette(parent)
        
        self._nodeName = nodeName
        self._treeRoot = treeRoot
        self._parentNode = parentnode

        self._last = last
	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
            ('selectbackground', colors['selectBackground'], INITOPT),
            ('selectforeground', colors['selectForeground'], INITOPT),
            ('background',       colors['background'],       INITOPT),
            ('foreground',       colors['foreground'],       INITOPT),
            ('selectcommand',    None,                       None),
            ('deselectcommand',  None,                       None),
            ('labelpos',         'e',                        INITOPT),
            ('labelmargin',      0,                          INITOPT),
            ('label',            None,                       None),
        )
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)

        # Create the components
        interior = self._hull


        labelpos = self['labelpos']

        if self['label'] == None:
            self._labelWidget = self.createcomponent('labelwidget',
                                                     (), None,
                                                     Pmw.LabeledWidget,
                                                     (interior,),
                                                     #background = self['background'],
                                                     #foreground = self['foreground'],
                                                     )
        else:
            self._labelWidget = self.createcomponent('labelwidget',
                                                     (), None,
                                                     Pmw.LabeledWidget,
                                                     (interior,),
                                                     label_background = self['background'],
                                                     label_foreground = self['foreground'],
                                                     labelpos = labelpos,
                                                     labelmargin = self['labelmargin'],
                                                     label_text = self['label'],
                                                     )
            self._labelWidget.component('label').bind('<ButtonRelease-1>',
                                                      self._selectevent)

        self._labelWidget.grid(column = 1, row = 0, sticky = 'w')

        self._labelWidget.update()

        self._labelheight = self._labelWidget.winfo_height()

        self._lineCanvas = self.createcomponent('linecanvas',
                                                (), None,
                                                Tkinter.Canvas,
                                                (interior,),
                                                width = self._labelheight,
                                                height = self._labelheight,
                                                )
        self._lineCanvas.grid( column = 0, row = 0, sticky = 'news')
        self._lineCanvas.update()

        cw = int(self._lineCanvas['width'])
        ch = int(self._lineCanvas['height'])

        self._lineCanvas.create_line(cw/2, ch/2, cw, ch/2, tag='hline')
        if last:
            self._lineCanvas.create_line(cw/2, 0, cw/2, ch/2, tag='vline')
        else:
            self._lineCanvas.create_line(cw/2, 0, cw/2, ch, tag='vline')

        # Check keywords and initialise options.
        self.initialiseoptions()


    def interior(self):
        return self._labelWidget.interior()

    def select(self):
        self._highlight()

    def getname(self):
        return self._nodeName

    def getlabel(self):
        return self['label']
    
    def _selectevent(self, event):
        self._highlight()

    def _highlight(self):
        self._treeRoot._highlightnode(self)
        #self._subHull.configure(background = self._selectbg, relief = 'raised')
        if self['label'] != None:
            self._labelWidget.configure(label_background = self['selectbackground'])
            self._labelWidget.configure(label_foreground = self['selectforeground'])
        #self._viewButton.configure(background = self._selectbg)
        cmd = self['selectcommand']
        if callable(cmd):
            cmd(self)

    def _unhighlight(self):
        #self._subHull.configure(background = self._bg, relief = 'flat')
        if self['label'] != None:
            self._labelWidget.configure(label_background = self['background'])
            self._labelWidget.configure(label_foreground = self['foreground'])
        #self._viewButton.configure(background = self._bg)
        cmd = self['deselectcommand']
        if callable(cmd):
            cmd(self)

    def _setlast(self, last):
        self._last = last

        cw = int(self._lineCanvas['width'])
        ch = int(self._lineCanvas['height'])

        if last:
            self._lineCanvas.create_line(cw/2, 0, cw/2, ch/2, tag='vline')
        else:
            self._lineCanvas.create_line(cw/2, 0, cw/2, ch, tag='vline')
        

class _BranchNode(_LeafNode, _Branching): #Pmw.MegaWidget):

    def __init__(self, parent, nodeName, treeRoot, parentnode, last = 1, **kw):
	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
            ('view',            'collapsed', None),
            ('expandcommand',   None,        None),
            ('collapsecommand', None,        None),
            ('indent',          0,           INITOPT)
        )
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
        apply(_LeafNode.__init__,
              (self, parent, nodeName, treeRoot, parentnode, last),
              kw)
        _Branching.__init__(self)

        # Create the components
        interior = self._hull

        # Create the expand/collapse button
        self._viewButton = self.createcomponent('viewbutton', (), None,
                                                Tkinter.Canvas,
                                                (interior,),
                                                background = self['background'],
                                                width = self._labelheight - 4,
                                                height = self._labelheight - 4,
                                                borderwidth = 2,
                                                relief = 'raised')

        self._viewButton.grid(column = 0, row = 0, sticky='se')
        self._viewButton.bind('<ButtonPress-1>', self._showbuttonpress)
        self._viewButton.bind('<ButtonRelease-1>', self._toggleview)

        # The label widget is already created by the base class, however
        # we do need to make some slight modifications.
        if self['label'] != None:
            self._labelWidget.component('label').bind('<Double-1>',
                                                      self._toggleview)
        self._labelWidget.grid(column=1, row=0, columnspan = 3, sticky='sw')

        # A line canvas is already created for us, we just need to make
        # some slight modifications
        self._lineCanvas.delete('hline')
        self._lineCanvas.grid_forget()

        
        # Set the minsize of column 1 to control additional branch frame indentation
        self.grid_columnconfigure(1, minsize = self['indent'])

        # Create the branch frame that will contain all the branch/leaf nodes
        self._branchFrame = self.createcomponent('frame', (), None,
                                                 Tkinter.Frame, (interior,),
                                                 #borderwidth=2,
                                                 #relief='ridge',
                                                 )
        self.grid_columnconfigure(2,minsize=0, weight=1)
        #self.grid_rowconfigure(0,minsize=0)

        if(self['view'] == 'expanded'):
            Pmw.drawarrow(self._viewButton,
                          self['foreground'],
                          'down', 'arrow')
            self._branchFrame.grid(column = 2, row = 1, sticky='nw')
            if not self._last:
                self._branchFrame.update()
                bh = self._branchFrame.winfo_height()
                self._lineCanvas.configure(height = bh)
                self._lineCanvas.grid(column = 0, row = 1, sticky='news')
                cw = int(self._lineCanvas['width'])
                ch = int(self._lineCanvas['height'])
                #self._lineCanvas.create_line(cw/2, 1, cw/2, ch, tag = 'vline')
                self._lineCanvas.coords('vline', cw/2, 1, cw/2, ch)
        else:
            Pmw.drawarrow(self._viewButton,
                          self['foreground'],
                          'right', 'arrow')
            self._viewButton.configure(relief = 'raised')
        

        # Check keywords and initialise options.
        self.initialiseoptions()



    def _showbuttonpress(self, event):
        self._viewButton.configure(relief = 'sunken')
        
    def _toggleview(self, event):
        self._viewButton.configure(relief = 'sunken')
        self.select()
        if(self['view'] == 'expanded'):
            self.collapsetree()
        else:
            self.expandtree()
        self._viewButton.configure(relief = 'raised')
        
    def expandtree(self):
        if(self['view'] == 'collapsed'):
            cmd = self['expandcommand']
            if cmd is not None:
                cmd(self)
            self['view'] = 'expanded'
            Pmw.drawarrow(self._viewButton,
                          self['foreground'],
                          'down', 'arrow')
            self._branchFrame.grid(column = 2, row = 1, sticky='nw')
            
            if not self._last:
                self._branchFrame.update()
                bh = self._branchFrame.winfo_height()
                self._lineCanvas.configure(height = bh)
                self._lineCanvas.grid(column = 0, row = 1, sticky='news')
                cw = int(self._lineCanvas['width'])
                ch = int(self._lineCanvas['height'])
                #self._lineCanvas.create_line( cw/2, 1, cw/2, ch, tag = 'vline')
                self._lineCanvas.coords('vline', cw/2, 1, cw/2, ch)
            self._parentNode._sizechange()
        
    def collapsetree(self):
        if(self['view'] == 'expanded'):
            cmd = self['collapsecommand']
            if cmd is not None:
                cmd(self)
            self['view'] = 'collapsed'
            Pmw.drawarrow(self._viewButton,
                          self['foreground'],
                          'right', 'arrow')
            self._branchFrame.grid_forget()
            if not self._last:
                #self._lineCanvas.delete('vline')
                self._lineCanvas.grid_forget()
            self._parentNode._sizechange()

    def _setlast(self, last):
        self._last = last
        if self['view'] == 'expanded':
            self._branchFrame.update()
            bh = self._branchFrame.winfo_height()
            self._lineCanvas.configure(height = bh)
            cw = int(self._lineCanvas['width'])
            ch = int(self._lineCanvas['height'])
            self._lineCanvas.delete('vline')
            if not last:
                self._lineCanvas.create_line(cw/2, 1, cw/2, ch, tag='vline')


    def _sizechange(self):
        if not self._last and self['view'] == 'expanded':
            self._branchFrame.update()
            bh = self._branchFrame.winfo_height()
            self._lineCanvas.configure(height = bh)
            if self._lineCanvas.coords('vline')[3] < bh:
                cw = int(self._lineCanvas['width'])
                ch = int(self._lineCanvas['height'])
                #self._lineCanvas.delete('vline')
                #self._lineCanvas.create_line(cw/2, 1, cw/2, ch, tag='vline')
                self._lineCanvas.coords('vline', cw/2, 1, cw/2, ch)
        self._parentNode._sizechange()

class TreeBrowser(Pmw.MegaWidget, _Branching):

    def __init__(self, parent = None, nodeName = '0', **kw):
        colors = Pmw.Color.getdefaultpalette(parent)

	# Define the megawidget options.
	INITOPT = Pmw.INITOPT
	optiondefs = (
            ('indent',           0,                          INITOPT),
            ('selectbackground', colors['selectBackground'], INITOPT),
            ('selectforeground', colors['selectForeground'], INITOPT),
            ('background',       colors['background'],       INITOPT),
            ('foreground',       colors['foreground'],       INITOPT),
            #('selectrelief',     'raised',       INITOPT),
        )
        
	self.defineoptions(kw, optiondefs)

	# Initialise the base class (after defining the options).
	Pmw.MegaWidget.__init__(self, parent)
        _Branching.__init__(self)

        
        # Create the components
        interior = self._hull

        browserFrame = self.createcomponent('frame', (), None,
                                            Pmw.ScrolledFrame,
                                            (interior,),
                                            )
        
        browserFrame.pack(expand = 1, fill='both')

        self._branchFrame = browserFrame.interior()

        self._highlightedNode = None
        self._treeRoot = self
        self._nodeName = nodeName
        # Check keywords and initialise options.
        self.initialiseoptions()

    def _highlightnode(self, newNode):
        if self._highlightedNode != newNode:
            if self._highlightedNode != None:
                self._highlightedNode._unhighlight()
            self._highlightedNode = newNode

    def _unhighlightnode(self):
        if self._highlightedNode != None:
            self._highlightedNode._unhighlight()
            self._highlightedNode = None

    def curselection(self):
        retVal = None
        if self._highlightedNode != None:
            retVal = (self._highlightedNode,
                      self._highlightedNode._nodeName,
                      self._highlightedNode['label'])
        return retVal

    def getname(self):
        return self._nodeName

    # The top-level TreeBrowser widget only shows nodes in an expanded view
    # but still provides collapsetree() and expandtree() methods so that users
    # don't have to special case the top-level node

    def collapsetree(self):
        return

    def expandtree(self):
        return
    
    def _sizechange(self):
        return

if __name__ == '__main__':

    rootWin = Tkinter.Tk()

    Pmw.initialise()

    rootWin.title('TreeBrowser Demo')

    # Create the hierarchical tree browser widget
    treeBrowser = TreeBrowser(rootWin,
                                          #selectbackground = "darkgreen",
                                          #selectforeground = 'lightgreen',
                                          #background = 'green',
                                          #indent = 10,
                                          )


    def printselected(node):
        selection = treeBrowser.curselection()
        if selection != None:
            print "Selected node name:", selection[1], "   label:", selection[2]


    def printdeselected(node):
        selection = treeBrowser.curselection()
        if selection != None:
            print "Deselected node name:", selection[1], "   label:", selection[2]

    def printexpanded(node):
        print "Expanded node name:", node.getname(), "   label:", node.getlabel()

    def printcollapsed(node):
        print "Collapsed node name:", node.getname(), "   label:", node.getlabel()



    for i in range(3):
        # Add a tree node to the top level
        treeNodeLevel1 = treeBrowser.addbranch(label = 'TreeNode %d'%i,
                                               selectcommand = printselected,
                                               deselectcommand = printdeselected,
                                               expandcommand = printexpanded,
                                               collapsecommand = printcollapsed,
                                               )
        for j in range(3):
            # Add a tree node to the second level
            treeNodeLevel2 = treeNodeLevel1.addbranch(label = 'TreeNode %d.%d'%(i,j),
                                                      #selectforeground = 'yellow',
                                                      selectcommand = printselected,
                                                      deselectcommand = printdeselected,
                                                      expandcommand = printexpanded,
                                                      collapsecommand = printcollapsed,
                                                      )
            if i == 0 and j == 1:
                dynamicTreeRootNode = treeNodeLevel1
                dynamicTreePosNode = treeNodeLevel2
                
            for item in range((i+1)*(j+1)):
                # Add a leaf node to the third level
                leaf = treeNodeLevel2.addleaf(label = "Item %c"%(item+65),
                                              #selectbackground = 'blue',
                                              selectcommand = printselected,
                                              deselectcommand = printdeselected)
        for item in range(i+1):
            # Add a leaf node to the top level
            leaf = treeNodeLevel1.addleaf(label = "Item %c"%(item+65),
                                          selectcommand = printselected,
                                          deselectcommand = printdeselected)


    treeNodeLevel1 = treeBrowser.addbranch(label = 'Check Button Label',
                                           selectcommand = printselected,
                                           deselectcommand = printdeselected,
                                           expandcommand = printexpanded,
                                           collapsecommand = printcollapsed,
                                           )
    checkButton = Tkinter.Checkbutton(treeNodeLevel1.interior(),
                                      text = 'Da Check Button',
                                      relief = 'ridge',
                                      command = treeNodeLevel1.select)
    checkButton.pack()

    treeNodeLevel1.addleaf(label = 'Labeled Leaf',
                           selectcommand = printselected,
                           deselectcommand = printdeselected)
    leaf = treeNodeLevel1.addleaf(label = 'Labeled Leaf w/ Checkbutton',
                                  selectcommand = printselected,
                                  deselectcommand = printdeselected)
    checkButton = Tkinter.Checkbutton(leaf.interior(),
                                      text = 'Da Check Button',
                                      relief = 'ridge',
                                      command = leaf.select)
    checkButton.pack()


    treeNodeLevel1 = treeBrowser.addbranch(selectcommand = printselected,
                                           deselectcommand = printdeselected,
                                           expandcommand = printexpanded,
                                           collapsecommand = printcollapsed,
                                           )
    checkButton = Tkinter.Checkbutton(treeNodeLevel1.interior(),
                                      text = 'Check Button with no label',
                                      relief = 'ridge',
                                      command = treeNodeLevel1.select)
    checkButton.pack()

    treeNodeLevel1 = treeBrowser.addbranch(label = 'Label',
                                           selectcommand = printselected,
                                           deselectcommand = printdeselected,
                                           expandcommand = printexpanded,
                                           collapsecommand = printcollapsed,
                                           )

    # setup dynamic tree node insertion and removal
    class dynTree:
        def __init__(self):
            self.dyn = Tkinter.IntVar()
            self.dtree = None

            self.dLeaf = treeBrowser.addleaf(selectcommand = self.dynSelected,
                                             deselectcommand = self.dynDeselected)
            
            self.dCheckButton = Tkinter.Checkbutton(self.dLeaf.interior(),
                                                    text = 'Enable Dynamic Tree',
                                                    variable = self.dyn,
                                                    command = self.ChkBtnHandler)
            self.dCheckButton.pack()

            
        def dynSelected(self, node):
            self.dCheckButton.configure(background = self.dLeaf.configure('selectbackground')[4])
            printselected(node)
                
        def dynDeselected(self, node):
            self.dCheckButton.configure(background = self.dLeaf.configure('background')[4])
            printdeselected(node)
                    
        def ChkBtnHandler(self):
            self.dLeaf.select()
            if self.dyn.get() == 1:
                self.dtree = dynamicTreeRootNode.insertbranch(label = 'Dynamic Tree Node',
                                                              selectcommand = printselected,
                                                              deselectcommand = printdeselected,
                                                              expandcommand = printexpanded,
                                                              collapsecommand = printcollapsed,
                                                              before = dynamicTreePosNode)
                self.dtree.addleaf(label = 'Dynamic Leaf 1',
                                   selectcommand = printselected,
                                   deselectcommand = printdeselected)
                self.dtree.addleaf(label = 'Dynamic Leaf 2',
                                   selectcommand = printselected,
                                   deselectcommand = printdeselected)
            else:
                if self.dtree != None:
                    dynamicTreeRootNode.delete(self.dtree)
                    self.dtree = None


    foo = dynTree()


    treeBrowser.pack(expand = 1, fill='both')

    exitButton = Tkinter.Button(rootWin, text="Quit", command=rootWin.quit)
    exitButton.pack()

    rootWin.mainloop()
