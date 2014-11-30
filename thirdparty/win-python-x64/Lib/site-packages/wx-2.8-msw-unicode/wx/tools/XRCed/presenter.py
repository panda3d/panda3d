# Name:         presenter.py
# Purpose:      Presenter part
# Author:       Roman Rolinsky <rolinsky@femagsoft.com>
# Created:      07.06.2007
# RCS-ID:       $Id: presenter.py 64627 2010-06-18 18:17:45Z ROL $

import os,tempfile,shutil
from xml.parsers import expat
import cPickle
from globals import *
import view
from model import Model, MyDocument
from component import Manager
import undo

# Presenter class linking model to view objects
class _Presenter:
    def init(self):
        Model.init()
        self.path = ''
        # Global modified state
        self.setModified(False) # sets applied
        view.frame.Clear()
        view.tree.Clear()
        view.tree.SetPyData(view.tree.root, Model.mainNode)
        view.testWin.Init()
        g.undoMan.Clear()
        # Insert/append mode flags
        self.createSibling = self.insertBefore = False
        # Select main node attributes
        self.setData(view.tree.root)

    def loadXML(self, path):
        Model.loadXML(path)
        view.tree.Flush()
        view.tree.SetPyData(view.tree.root, Model.mainNode)
        self.setData(view.tree.root)
        if g.conf.expandOnOpen:
            view.tree.ExpandAll()

    def saveXML(self, path):
        Model.saveXML(path)

    def open(self, path):
        if not os.path.exists(path):
            wx.LogError('File does not exists: %s' % path)
            raise IOError
        try:
            self.path = os.path.abspath(path)
            TRACE('Loading XML file: %s', self.path)
            self.loadXML(self.path)
            # Change dir
            dir = os.path.dirname(self.path)
            if dir: os.chdir(dir)
            self.setModified(False)
            g.conf.localconf = self.createLocalConf(path)
        except:
            logger.exception('error loading XML file')
            wx.LogError('Error loading XML file: %s' % path)
            raise
            
    def save(self, path):
        # Apply changes if needed
        if not self.applied:
            self.update(self.item)
        try:
            tmpFile,tmpName = tempfile.mkstemp(prefix='xrced-')
            os.close(tmpFile)
            TRACE('Saving temporary file: %s', tmpName)
            self.saveXML(tmpName)
            TRACE('copying to the main file: %s', path)
            shutil.copy(tmpName, path)
            self.path = path
            self.setModified(False)
        except:
            logger.exception('error saving XML file')
            wx.LogError('Error saving XML file: %s' % path)
            raise

    def setModified(self, state=True, setDirty=True):
        '''Set global modified state.'''
        TRACE('setModified %s %s', state, setDirty)
        self.modified = state
        # Set applied flag
        if not state: self.applied = True
        name = os.path.basename(self.path)
        if not name: name = 'UNTITLED'
        # Update GUI
        if state:
            view.frame.SetTitle(progname + ': ' + name + ' *')
            # Update test window
            if view.testWin.IsShown() and setDirty:
                view.testWin.isDirty = True
                if g.conf.autoRefresh:
                    self.refreshTestWin()
        else:
            view.frame.SetTitle(progname + ': ' + name)

    def setApplied(self, state=True):
        '''Set panel state.'''
        TRACE('setApplied %s', state)
        self.applied = state
        if not state and not self.modified: 
            self.setModified(setDirty=False)  # toggle global state

    def createUndoEdit(self, item=None, page=None):
        TRACE('createUndoEdit')
        # Create initial undo object
        if item is None: item = self.item
        if page is None: page = view.panel.nb.GetSelection()
        view.panel.undo = undo.UndoEdit(item, page)

    def registerUndoEdit(self):
        TRACE('registerUndoEdit')
        g.undoMan.RegisterUndo(view.panel.undo)
        view.panel.undo = None

    def panelIsDirty(self):
        '''Check if the panel was changed since last undo.'''
        # Register undo
        if view.panel.undo:
            panel = view.panel.GetActivePanel()
            if view.panel.undo.values != panel.GetValues():
                return True
        return False

    def setData(self, item):
        '''Set data and view for current tree item.'''

        self.item = item
        if item == view.tree.root:
            TRACE('setData: root node')
            self.container = None
            self.comp = Manager.rootComponent
            self.panels = view.panel.SetData(self.container, self.comp, Model.mainNode)
        else:
            node = view.tree.GetPyData(item)
            if node.nodeType != node.COMMENT_NODE:
                TRACE('setData: %s', node.getAttribute('class'))
            self.comp = Manager.getNodeComp(node)
            parentItem = view.tree.GetItemParent(item)
            parentNode = view.tree.GetPyData(parentItem)
            if parentNode == Model.mainNode:
                self.container = Manager.rootComponent
            else:
                parentClass = parentNode.getAttribute('class')
                self.container = Manager.components[parentClass]
            self.panels = view.panel.SetData(self.container, self.comp, node)
        # Create new pending undo
        self.createUndoEdit(self.item)

        if view.testWin.IsShown():
            self.highlight(item)

    def highlight(self, item):
        TRACE('highlight')
        if view.testWin.IsDirty() or item == view.tree.root or \
            view.tree.GetPyData(item).nodeType == Model.dom.COMMENT_NODE:
            view.testWin.RemoveHighlight()
            return
        try:
            rect = view.testWin.FindObjectRect(item)
            if not rect:
                view.testWin.RemoveHighlight()
                return
            view.testWin.Highlight(rect)
        except:
            logger.exception('highlighting failed')

    def updateCreateState(self, forceSibling, forceInsert):
        if self.container:
            if self.comp.isContainer():
                self.createSibling = forceSibling
            else:
                self.createSibling = True
        else:
            self.createSibling = False
        self.insertBefore = forceInsert        
        TRACE('updateCreateState: %s %s', self.createSibling, self.insertBefore)

    def popupMenu(self, forceSibling, forceInsert, pos):
        '''Show popup menu and set sibling/insert flags.'''
        self.updateCreateState(forceSibling, forceInsert)
        menu = view.XMLTreeMenu(self.container, self.comp, view.tree,
                                self.createSibling, self.insertBefore)
        view.tree.PopupMenu(menu, pos)
        menu.Destroy()        

    def create(self, comp, child=None):
        '''
        Add DOM node as child or sibling depending on flags. Return new item.

        If child is passed replace by existing data.
        '''
        if child is None:
            child = Model.createObjectNode(comp.klass)
            # Set default values
            for k,v in comp.defaults.items():
                comp.addAttribute(child, k, v)
        data = wx.TreeItemData(child)
        item = self.item
        if not self.applied:
            self.update(item)
        if item == view.tree.root:
            self.createSibling = False # can't create sibling of root
        if self.createSibling:
            parentItem = view.tree.GetItemParent(item)
            parentNode = view.tree.GetPyData(parentItem)
        else:
            parentNode = view.tree.GetPyData(item)
        label = comp.getTreeText(child)
        imageId = comp.getTreeImageId(child)
        if self.createSibling:
            node = view.tree.GetPyData(item)
            if self.insertBefore:
                self.container.insertBefore(parentNode, child, node)
                item = view.tree.InsertItemBefore(
                    parentItem, item, label, imageId, data=data)

            else:
                self.container.insertAfter(parentNode, child, node)
                item = view.tree.InsertItem(
                    parentItem, item, label, imageId, data=data)
        else:
            if self.insertBefore and view.tree.ItemHasChildren(item):
                nextNode = view.tree.GetPyData(view.tree.GetFirstChild(item)[0])
                self.comp.insertBefore(parentNode, child, nextNode)
                item = view.tree.PrependItem(item, label, imageId, data=data)
            else:
                self.comp.appendChild(parentNode, child)
                item = view.tree.AppendItem(item, label, imageId, data=data)
        view.tree.SetItemStyle(item, child)
        view.tree.EnsureVisible(item)
        view.tree.UnselectAll()
        if view.testWin.IsShown():
            view.testWin.isDirty = True
        view.tree.SelectItem(item)
        self.setModified()
        return item

    def createRef(self, ref, child=None):
        '''Create object_ref element node.'''
        if child is None:
            child = Model.createRefNode(ref)
        refNode = Model.findResource(ref)
        if refNode: 
            comp = Manager.getNodeComp(refNode)
        else:
            comp = Manager.getNodeComp(child)
        self.create(comp, child)

    def createComment(self):
        '''Create comment node.'''
        node = Model.createCommentNode()
        comp = Manager.getNodeComp(node)
        self.create(comp, node)

    def replace(self, comp, node=None):
        '''Replace DOM node by new or passed node. Return new item.'''
        TRACE('replace')
        if node is None:
            node = Model.createObjectNode(comp.klass)
        if not self.applied:
            self.update(item)
        data = wx.TreeItemData(node)
        item = self.item
        parentItem = view.tree.GetItemParent(item)
        parentNode = view.tree.GetPyData(parentItem)
        oldNode = view.tree.GetPyData(item)
        self.container.replaceChild(parentNode, node, oldNode)
        # Replace tree item: insert new, remove old
        label = comp.getTreeText(node)
        imageId = comp.getTreeImageId(node)
        item = view.tree.InsertItem(parentItem, item, label, imageId, data=data)
        view.tree.Delete(view.tree.GetPrevSibling(item))
        self.item = item
        # Add children
        for n in filter(is_object, node.childNodes):
            view.tree.AddNode(item, comp.getTreeNode(n))
        view.tree.EnsureVisible(item)
        # Update panel
        view.tree.SelectItem(item)
        self.setModified()
        return oldNode

    def subclass(self, item, subclass):
        node = view.tree.GetPyData(item)
        if subclass:
            node.setAttribute('subclass', subclass)
        elif node.hasAttribute('subclass'):
            node.removeAttribute('subclass')
        # Update item label
        view.tree.SetItemImage(item, self.comp.getTreeImageId(node))
        view.tree.SetItemText(item, self.comp.getTreeText(node))        
        # Update panel
        view.tree.SelectItem(item)
        self.setModified()        

    def update(self, item):
        '''Update DOM with new attribute values. Update tree if necessary.'''
        node = view.tree.GetPyData(item)
        isComment = node.nodeType == node.COMMENT_NODE
        if isComment:
            subclass = None
        else:
            subclass = node.getAttribute('subclass')
        # Update (sub)class if needed
        cls = view.panel.textClass.GetValue()
        if not subclass:
            if not isComment and cls != self.comp.klass:
                if node.tagName == 'object_ref' and not cls:
                    if node.hasAttribute('class'):
                        node.removeAttribute('class')
                        TRACE('removed "class" tag')
                else:
                    TRACE('update class: %s', cls)
                    node.setAttribute('class', cls)
        else:
            value = subclass + '(%s)' % self.comp.klass
            if cls != value:
                iLeft = cls.find('(')
                iRight = cls.find(')')
                if iLeft != -1 and iLeft < iRight:
                    subclass = cls[:iLeft]
                    klass = cls[iLeft+1:iRight]
                    TRACE('update class/subclass: %s', cls)
                    node.setAttribute('class', klass)
                    node.setAttribute('subclass', subclass)
                else:
                    TRACE('remove subclass')
                    node.removeAttribute('subclass')
                    node.setAttribute('class', cls)
        if self.comp and self.comp.hasName:
            name = view.panel.textName.GetValue()
            if name:
                node.setAttribute('name', name)
            elif node.hasAttribute('name'): # clean up empty names
                node.removeAttribute('name')
        if item != view.tree.root: 
            for panel in self.panels:
                if not panel.node: continue
                # Replace node contents except object children
                for n in panel.node.childNodes[:]:
                    if not is_object(n):
                        panel.node.removeChild(n)
                        n.unlink()
        for panel in self.panels:
            for a,value in panel.GetValues():
                if value:
                    try:
                        if isinstance(panel, view.AttributePanel) and panel.comp:
                            comp = panel.comp
                        else:
                            comp = self.comp
                        comp.addAttribute(panel.node, a, value)
                    except:
                        logger.exception('addAttribute error: %s %s', a, value)
        if item != view.tree.root:
            view.tree.SetItemImage(item, self.comp.getTreeImageId(node))
            view.tree.SetItemText(item, self.comp.getTreeText(node))
        self.setApplied()
        # Set dirty flag
        if view.testWin.IsShown():
            view.testWin.isDirty = True

    def unselect(self):
        if not self.applied:
            self.update(self.item)
            if view.testWin.IsShown() and view.testWin.item == self.item:
                view.testWin.Destroy()
        view.tree.UnselectAll()
        self.setData(view.tree.root)

    def flushSubtree(self, item=None, node=None):
        # Remember test item index
        TRACE('flushSubtree')
        if view.testWin.item is not None:
            itemIndex = view.tree.ItemFullIndex(view.testWin.item)
        view.tree.FlushSubtree(item, node)
        if view.testWin.item is not None:
            view.testWin.item = view.tree.ItemAtFullIndex(itemIndex)

    def delete(self, item):
        '''Delete selected object(s). Return removed XML node.'''
        TRACE('delete')
        parentItem = view.tree.GetItemParent(item)
        parentNode = view.tree.GetPyData(parentItem)
        node = view.tree.GetPyData(item)
        node = self.container.removeChild(parentNode, node)
        view.tree.Delete(item)
        # If deleting the top-level object, remove view
        if view.testWin.IsShown() and view.testWin.item == item:
            view.testWin.Destroy()
        self.setApplied()
        self.unselect()
        self.setModified()
        return node

    def deleteMany(self, items):
        '''Delete selected object(s).'''
        for item in items:
            if not item.IsOk(): continue # child already deleted
            parentItem = view.tree.GetItemParent(item)
            parentNode = view.tree.GetPyData(parentItem)
            node = view.tree.GetPyData(item)
            node = self.container.removeChild(parentNode, node)
            node.unlink()       # delete completely
            view.tree.Delete(item)
        self.setApplied()
        self.unselect()
        self.setModified()

    def cut(self):
        self.copy()
        return self.delete(view.tree.GetSelection())

    def copy(self):
        # Update values from panel first
        item = view.tree.GetSelection()
        if not self.applied:
            self.update(item)
        node = view.tree.GetPyData(item)
        if self.container.requireImplicit(node):
            implicit = node.parentNode
        else:
            implicit = None
        if wx.TheClipboard.Open():
            if node.nodeType == node.ELEMENT_NODE:
                data = wx.CustomDataObject('XRCED_elem')
                s = node.toxml(encoding=expat.native_encoding)
                # Replace by a pair
                if implicit: s = [s, implicit.toxml(encoding=expat.native_encoding)]
            else:
                # Non-element nodes are normally comments
                data = wx.CustomDataObject('XRCED_node')
                s = node.data
            data.SetData(cPickle.dumps(s))
            wx.TheClipboard.SetData(data)
            wx.TheClipboard.Close()
        else:
            wx.MessageBox("Unable to open the clipboard", "Error")        

    def checkCompatibility(self, comp):
        '''Check parent/child compatibility.'''
        if self.createSibling: container = self.container
        else: container = self.comp
        if not container.canHaveChild(comp):
            wx.LogError('Incompatible parent/child: parent is %s, child is %s!' %
                        (container.klass, comp.klass))
            return False
        return True

    def paste(self):
        success = success_node = False
        if wx.TheClipboard.IsOpened() or wx.TheClipboard.Open():
            try:
                data = wx.CustomDataObject('XRCED_elem')
                if wx.TheClipboard.IsSupported(data.GetFormat()):
                    try:
                        success = wx.TheClipboard.GetData(data)
                    except:
                        # there is a problem if XRCED_node is in clipboard
                        # but previous SetData was for XRCED
                        pass
                if not success:             # try other format
                    data = wx.CustomDataObject('XRCED_node')
                    if wx.TheClipboard.IsSupported(data.GetFormat()):
                        success_node = wx.TheClipboard.GetData(data)
            finally:
                wx.TheClipboard.Close()

        if not success and not success_node:
            wx.MessageBox(
                "There is no data in the clipboard in the required format",
                "Error")
            return

        # XML representation of element or node value string
        data = cPickle.loads(data.GetData()) 
        implicit = None
        if success:
            if type(data) is list:
                node = Model.parseString(data[0])
                implicit = Model.parseString(data[1])
            else:
                node = Model.parseString(data)
        else:
            node = Model.dom.createComment(data)
        comp = Manager.getNodeComp(node)

        # Check compatibility
        if not self.checkCompatibility(comp):
            node.unlink()
            return

        item = view.tree.GetSelection()
        if item and not self.applied:
            self.update(item)
        
        item = self.create(comp, node)
        if implicit:   # copy parameters for implicit node if possible
            parentNode = view.tree.GetPyData(view.tree.GetItemParent(item))
            parentComp = Manager.getNodeComp(parentNode)
            if parentComp.requireImplicit(node) and \
                    parentComp.implicitKlass == implicit.getAttribute('class'):
                parentComp.copyImplicitAttributes(implicit, node.parentNode, parentComp)
            implicit.unlink()

        # Add children
        for n in filter(is_object, node.childNodes):
            view.tree.AddNode(item, comp.getTreeNode(n))
        self.setModified()
        return item

    def moveUp(self):
        parentItem = view.tree.GetItemParent(self.item)
        treeNode = view.tree.GetPyData(self.item)
        node = self.container.getTreeOrImplicitNode(treeNode)
        parent = node.parentNode
        prevNode = node.previousSibling
        while not is_object(prevNode):
            prevNode = prevNode.previousSibling
        parent.removeChild(node)
        parent.insertBefore(node, prevNode)
        index = view.tree.ItemFullIndex(self.item)
        self.flushSubtree(parentItem, parent)
        index[-1] -= 1
        self.item = view.tree.ItemAtFullIndex(index)
        self.setModified()
        view.tree.SelectItem(self.item)
        
    def moveDown(self):
        parentItem = view.tree.GetItemParent(self.item)
        treeNode = view.tree.GetPyData(self.item)
        node = self.container.getTreeOrImplicitNode(treeNode)
        parent = node.parentNode
        nextNode = node.nextSibling
        while not is_object(nextNode):
            nextNode = nextNode.nextSibling
        nextNode = nextNode.nextSibling
        while nextNode and not is_object(nextNode):
            nextNode = nextNode.nextSibling
        parent.removeChild(node)
        parent.insertBefore(node, nextNode)
        index = view.tree.ItemFullIndex(self.item)
        self.flushSubtree(parentItem, parent)
        index[-1] += 1
        self.item = view.tree.ItemAtFullIndex(index)
        self.setModified()
        view.tree.SelectItem(self.item)

    def moveLeft(self):
        parentItem = view.tree.GetItemParent(self.item)
        grandParentItem = view.tree.GetItemParent(parentItem)
        parent = view.tree.GetPyData(parentItem)
        grandParent = view.tree.GetPyData(grandParentItem)
        if grandParent is Model.mainNode:
            grandParentComp = Manager.rootComponent
        else:
            grandParentComp = Manager.getNodeComp(grandParent)
        if not grandParentComp.canHaveChild(self.comp):
            wx.LogError('Incompatible parent/child: parent is %s, child is %s!' %
                        (grandParentComp.klass, self.comp.klass))
            return

        node = view.tree.GetPyData(self.item)
        nextItem = view.tree.GetNextSibling(parentItem)
        self.container.removeChild(parent, node)
        if nextItem:
            nextNode = view.tree.GetPyData(nextItem)
            grandParentComp.insertBefore(grandParent, node, nextNode)
        else:
            grandParentComp.appendChild(grandParent, node)
        index = view.tree.ItemFullIndex(self.item)
        self.flushSubtree(grandParentItem, grandParent)
        index.pop()
        index[-1] += 1
        self.item = view.tree.ItemAtFullIndex(index)
        self.setModified()
        view.tree.SelectItem(self.item)

    def moveRight(self):
        parentItem = view.tree.GetItemParent(self.item)
        parent = view.tree.GetPyData(parentItem)
        newParent = view.tree.GetPyData(view.tree.GetPrevSibling(self.item))
        newParentComp = Manager.getNodeComp(newParent)
        if not newParentComp.canHaveChild(self.comp):
            wx.LogError('Incompatible parent/child: parent is %s, child is %s!' %
                        (newParentComp.klass, self.comp.klass))
            return

        node = view.tree.GetPyData(self.item)
        self.container.removeChild(parent, node)
        newParentComp.appendChild(newParent, node)
        index = view.tree.ItemFullIndex(self.item)
        n = view.tree.GetChildrenCount(view.tree.GetPrevSibling(self.item))
        self.flushSubtree(parentItem, parent)
        index[-1] -= 1
        index.append(n)
        self.item = view.tree.ItemAtFullIndex(index)
        self.setModified()
        view.tree.SelectItem(self.item)

    def createLocalConf(self, path):
        name = os.path.splitext(path)[0]
        name += '.xcfg'
        return wx.FileConfig(localFilename=name)

    def createTestWin(self, item):
        TRACE('createTestWin')
        # Create a window with this resource
        node = view.tree.GetPyData(item)
        # Execute "pragma" comment node
        if node.nodeType == node.COMMENT_NODE:
            if node.data and node.data[0] == '%' and g.conf.allowExec != 'no':
                say = wx.NO
                if g.conf.allowExec == 'ask' and Model.allowExec is None:
                    say = wx.MessageBox('Execute comment directive?', 'Warning', 
                                        wx.ICON_EXCLAMATION | wx.YES_NO)
                if g.conf.allowExec == 'yes' or say == wx.YES:
                    code = node.data[1:] # skip '%'
                    view.tree.ExecCode(code)
            return
        # Close old window, remember where it was
        comp = Manager.getNodeComp(node)
        # Create memory XML file
        elem = node.cloneNode(True)
        if not node.hasAttribute('name'):
            name = 'noname'
        else:
            name = node.getAttribute('name')
        elem.setAttribute('name', STD_NAME)
        Model.setTestElem(elem)
        Model.saveTestMemoryFile()
        xmlFlags = 0
        if not g.conf.useSubclassing:
            xmlFlags |= xrc.XRC_NO_SUBCLASSING
        # Use translations if encoding is not specified
        if not Model.dom.encoding:
            xmlFlags |= xrc.XRC_USE_LOCALE
        res = xrc.EmptyXmlResource(xmlFlags)
        xrc.XmlResource.Set(res)        # set as global
        # Init other handlers
        Manager.addXmlHandlers(res)
        Manager.preload(res)
        # Same module list
        res.Load('memory:test.xrc')
        object = None
        testWin = view.testWin
        try:
            try:
                frame, object = comp.makeTestWin(res, name)
                if not object:  # skip the rest
                    raise EOFError
                # Reset previous tree item and locate tool
                if testWin.item:
                    view.tree.SetItemBold(testWin.item, False)
                testWin.SetView(frame, object, item)
                testWin.Show()
                view.tree.SetItemBold(item, True)
            except EOFError:
                pass
            except NotImplementedError:
                wx.LogError('Test window not implemented for %s' % node.getAttribute('class'))
                logger.exception('error creating test view')
            except:
                logger.exception('error creating test view')
                wx.LogError('Error creating test view')
                if get_debug(): raise
        finally:
            # Cleanup
            res.Unload(TEST_FILE)
            xrc.XmlResource.Set(None)
            wx.MemoryFSHandler.RemoveFile(TEST_FILE)

    def closeTestWin(self):
        TRACE('closeTestWin')
        if not view.testWin.object: return
        view.tree.SetItemBold(view.testWin.item, False)
        view.frame.tb.ToggleTool(view.frame.ID_TOOL_LOCATE, False)
        if view.frame.miniFrame:
            view.frame.miniFrame.tb.ToggleTool(view.frame.ID_TOOL_LOCATE, False)
        view.testWin.Destroy()

    def refreshTestWin(self):
        '''Refresh test window after some change.'''
        TRACE('refreshTestWin')
        if not view.testWin.IsDirty(): return
        if not self.applied: self.update(self.item)
        # Dumb refresh
        self.createTestWin(view.testWin.item)
        self.highlight(self.item)
        if view.frame.miniFrame and view.frame.miniFrame.IsShown():
            view.frame.miniFrame.Raise()
        else:
            view.frame.Raise()

    def showXML(self):
        '''Show some source.'''
        node = view.tree.GetPyData(self.item)
        dom = MyDocument()
        node = dom.appendChild(node.cloneNode(True))
        Model.indent(dom, node)
        text = node.toxml()#Model.dom.encoding)
        dom.unlink()
        lines = text.split('\n')
        maxLen = max(map(len, lines))
        w = max(40, min(80, maxLen))
        h = max(20, min(40, len(lines)))
        dlg = view.ScrolledMessageDialog(view.frame, text, 'XML Source',
                                         textSize=(w,h), centered=False)
        dlg.Bind(wx.EVT_CLOSE, lambda evt: dlg.Destroy())
        dlg.Bind(wx.EVT_BUTTON, lambda evt: dlg.Destroy(), id=wx.ID_OK)
        dlg.Show()

    def generatePython(self, dataFile, pypath, embed, genGettext):
        try:
            from wx.tools import pywxrc
            rescomp = pywxrc.XmlResourceCompiler()
            rescomp.MakePythonModule([dataFile], pypath, embed, genGettext, 
                                     assignVariables=False)
        except:
            logger.exception('error generating python code')
            wx.LogError('Error generating python code : %s' % pypath)
            raise

# Singleton class
Presenter = g.Presenter = _Presenter()

undo.Presenter = Presenter
