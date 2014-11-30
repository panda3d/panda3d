# Name:         component.py
# Purpose:      base component classes
# Author:       Roman Rolinsky <rolinsky@femagsoft.com>
# Created:      31.05.2007
# RCS-ID:       $Id: component.py 64627 2010-06-18 18:17:45Z ROL $

"""
Component plugin classes.

This module defines base classes and for deriving component plugin
classes and contains some global variables used to register components
with XRCed.

Components are objects of Component class or one of the derived
classes used to define specialized components (such as sizers). After
a component object is constructed and configured it can be registered
using the Manager global object.

"""


import os,sys,bisect
import wx
from sets import Set
from globals import *
from model import Model
from attribute import *
import params
import view
import images

DEFAULT_POS = (1000,1000)

# Group compatibility specifications. 
# Key is the parent group, value is the list of child groups.
# !value means named main group is excluded from possible children.
# "root" is a special group for the tree root
parentChildGroups = {
    'root': ['top_level', 'component'],      # top-level objects
    'frame': ['toolbar', 'menubar', 'statusbar'],
    'mdi_parent_frame': ['toolbar', 'menubar', 'statusbar', 'mdi_child_frame'],
    'mdi_child_frame': ['toolbar', 'menubar', 'statusbar', 
                        'control', 'window', 'sizer', 'btnsizer', 
                        '!frame', '!mdi_child_frame'],
    'wizard': ['wizard_page'],
    'window': ['control', 'window', 'sizer', 'btnsizer', '!frame', '!mdi_child_frame'],
    'sizer': ['control', 'sizer', 'btnsizer', 'spacer'],
    'book': ['control', 'window', '!sizer', '!btnsizer'],
    'btnsizer': ['stdbtn'],
    'menubar': ['menu'],
    'toolbar': ['tool', 'separator'],
    'menu': ['menu', 'menu_item', 'separator'],
}
'''
Definition of compatibility of component groups as I{key}:I{group_list} pairs, where
I{key} is a parent group name and I{group_list} is a list of children group names or
group names prefixed with '!' character to exclude components having corresponding
primary group. This dictionary can be modified by component plugins directly
(some care must be taken not to redefine existing relations or breake them).
'''


class Component(object):
    '''Base class for component plugins.'''
    # Common window attributes
    windowAttributes = ['fg', 'bg', 'font', 'tooltip', 'help', 
                        'enabled', 'focused', 'hidden']
    '''Default window attributes for window-like components.'''
    genericStyles = [
        'wxSIMPLE_BORDER', 'wxSUNKEN_BORDER', 'wxDOUBLE_BORDER',
        'wxRAISED_BORDER', 'wxSTATIC_BORDER', 'wxNO_BORDER',
        'wxCLIP_CHILDREN', 'wxTRANSPARENT_WINDOW', 'wxWANTS_CHARS',
        'wxNO_FULL_REPAINT_ON_RESIZE', 'wxFULL_REPAINT_ON_RESIZE'
        ]
    '''Default generic styles.'''
    genericExStyles = [
        'wxWS_EX_VALIDATE_RECURSIVELY',
        'wxWS_EX_BLOCK_EVENTS',
#        'wxWS_EX_TRANSIENT',   # not processed by XRC (yet?)
        'wxWS_EX_PROCESS_IDLE',
        'wxWS_EX_PROCESS_UI_UPDATES'
        ]
    '''Default generic extended styles.'''
    genericEvents = [
        'EVT_WINDOW_CREATE', 'EVT_WINDOW_DESTROY',
        'EVT_MOVE', 'EVT_SIZE',
        'EVT_MOUSE_EVENTS', 'EVT_MOTION',
        'EVT_LEFT_DOWN', 'EVT_LEFT_DCLICK', 
        'EVT_MIDDLE_DOWN', 'EVT_MIDDLE_DCLICK', 
        'EVT_RIGHT_DOWN', 'EVT_RIGHT_DCLICK',  'EVT_MOUSEWHEEL',
        'EVT_ENTER_WINDOW', 'EVT_LEAVE_WINDOW', 
        'EVT_KEY_DOWN', 'EVT_KEY_UP', 'EVT_CHAR',
        'EVT_PAINT', 'EVT_ERASE_BACKGROUND',
        'EVT_CONTEXT_MENU', 'EVT_HELP',
        'EVT_SET_FOCUS', 'EVT_KILL_FOCUS', 'EVT_CHILD_FOCUS',
        'EVT_UPDATE_UI', 'EVT_IDLE',
        ]
    '''Default generic events.'''
    hasName = True                      # most elements have XRC IDs
    '''True if component has an XRC ID attribute.'''
    isTopLevel = False                  # if can be created as top level window
    '''True if component can be a top-level object in XML tree.'''
    renameDict = {}
    '''Dictionary of I{old_name}:I{new_name} for renaming some attributes
    in the Attribute Panel.'''
    hasCode = True
    '''True if component can generate code.'''
    
    def __init__(self, klass, groups, attributes, **kargs):
        '''
        Construct a new Component object. 

        @param klass: Interface element class name (e.g. C{'wxButton'}).
        @param groups: List of group names to which this component belongs.
        First group is considered to be the I{primary group}.
        @param attributes: List of XRC attribute names.

        B{Supported keyword parameters:}

        @keyword defaults: Dictionary of default attribute values for creating
        new items.
        @keyword specials: Dictionary of I{attribute_name}:I{attribute_class} pairs
         for specifying special attribute classes for some attributes, instead of
         using default Attribute class.
        @keyword params: Dictionary of pairs I{attribute_name}:I{param_class} where
         I{param_class} is a attribute interface class (one of classes in
         params.py or a custom class). If a param class is not specified, a default
         value defined by C{paramDict} dictionary is used.
        @keyword image,images: C{wx.Image} object or a list of C{wx.Image} objects
         for tree icons.
        @keyword events: List of event names for code generation panel.
        '''
        self.klass = klass
        self.groups = groups
        self.attributes = attributes
        self.styles = []
        self.exStyles = []
        self.defaults = kargs.pop('defaults', {})
        # Special Attribute classes if required
        self.specials = kargs.pop('specials', {})
        # Some default special attributes
        self.specials['font'] = FontAttribute
        self.specials['XRCED'] = CodeAttribute
        # Special Param classes if required
        self.params = kargs.pop('params', {})
        # Tree image
        if 'images' in kargs:
            self.images = kargs.pop('images')
        elif 'image' in kargs:
            self.images = [kargs.pop('image')]
        elif not 'image' in self.__dict__:
            self.images = []
        # Code generation data
        self.events = kargs.pop('events', [])

    def addStyles(self, *styles):
        '''Add more styles.'''
        self.styles.extend(styles)

    def addExStyles(self, *styles):
        '''Add more extra styles.'''
        self.exStyles.extend(styles)

    def setSpecial(self, attrName, attrClass):
        '''Set special attribute class for processing XML.

        @param attrName: Attribute name.
        @param attrClass: Attribute class.
        '''
        self.specials[attrName] = attrClass

    def setParamClass(self, attrName, paramClass):
        '''Set special attribute panel class for editing attribute value.
        
        @param attrName: Attribute name.
        @param paramClass: Param class.
        '''
        self.params[attrName] = paramClass

    def getTreeImageId(self, node):
        try:
            return self.images[0].Id
        except IndexError:
            if self.isContainer():
                return 1
            return 0

    def getTreeText(self, node):
        if node.nodeType == node.COMMENT_NODE:
            return node.data
        if node.tagName == 'object_ref':
            ref = node.getAttribute('ref')
            label = 'ref: %s' % ref
        else:
            label = node.getAttribute('subclass')
            if not label:
                label = node.getAttribute('class')
        if self.hasName:
            name = node.getAttribute('name')
            if name: label += ' "%s"' % name
        return label

    def addEvents(self, *events):
        '''Add more events.'''
        self.events.extend(events)

    # Order components having same index by group and klass
    def __cmp__(self, other):
        if self.groups < other.groups: return -1
        elif self.groups == other.groups: 
            if self.klass < other.klass: return -1
            elif self.klass == other.klass: return 0
            else: return 1
        else: return 1

    def __repr__(self):
        return "Component('%s', %s)" % (self.klass, self.attributes)

    def canHaveChild(self, component):
        '''True if the current component can have child of given type.

        This function is redefined by container classes.'''
        return False

    def canBeReplaced(self, component):
        '''True if the current component can be replaced by component.

        This function can be redefined by derived classes.'''
        return component.groups == groups

    def isContainer(self):
        '''True if component is a container (can have child nodes).'''
        return isinstance(self, Container)

    def getAttribute(self, node, attribute):
        attrClass = self.specials.get(attribute, Attribute)
        # 'object' means attribute is a text node without element tag,
        if attribute == 'object':
            return attrClass.get(node)
        elif issubclass(attrClass, AttributeAttribute):
            return attrClass.getAA(node, attribute)
        for n in node.childNodes:
            if n.nodeType == node.ELEMENT_NODE and n.tagName == attribute:
                return attrClass.get(n)
        return attrClass.get(None)

    def addAttribute(self, node, attribute, value):
        '''Add attribute element.'''
        attrClass = self.specials.get(attribute, Attribute)
        attrClass.add(node, attribute, value)

    def makeTestWin(self, res, name):
        '''Method can be overrided by derived classes to create custom test view.

        @param res: C{wx.xrc.XmlResource} object with current test resource.
        @param name: XRC ID of tested object.
        '''
        if not self.hasName: raise NotImplementedError

        testWin = view.testWin
        if self.isTopLevel:
            # Top-level window creates frame itself
            frame = None
            object = res.LoadObject(view.frame, STD_NAME, self.klass)
            object.Fit()
            testWin.size = object.GetSize()
        else:
            # Create MiniFrame to hold selected subtree
            frame = testWin.frame
            if not frame:
                frame = wx.MiniFrame(view.frame, -1, '%s: %s' % (self.klass, name), name=STD_NAME,
                                     style=wx.CAPTION|wx.CLOSE_BOX|wx.RESIZE_BORDER)
                frame.panel = wx.Panel(frame)
            object = res.LoadObject(frame.panel, STD_NAME, self.klass)
            if not object: raise NotImplementedError
            if not isinstance(object, wx.Window): raise NotImplementedError
            object.SetPosition((10,10))
            if g.conf.fitTestWin: 
                object.Fit()
                if frame:
                    frame.SetClientSize(object.GetSize()+(20,20))
                    testWin.size = frame.GetSize()
        return frame, object

    def getRect(self, obj):
        '''Return bounding box coordinates for C{obj}.'''
        # Object's rect must be relative to testWin.object
        if isinstance(obj, wx.Window):
            return [obj.GetRect()]
        elif isinstance(obj, wx.Rect): # spacer
            return [obj]
        else:
            return None

    def copyAttributes(self, srcNode, dstNode):
        '''Copy relevant attribute nodes from srcNode to dstNode.'''
        dstComp = Manager.getNodeComp(dstNode)
        if dstComp.hasName:
            dstNode.setAttribute('name', srcNode.getAttribute('name'))
        for n in srcNode.childNodes:
            if n.nodeType == n.ELEMENT_NODE:
                a = n.tagName
                # Check if attributes are compatible
                srcAttrClass = self.specials.get(a, Attribute)
                dstAttrClass = dstComp.specials.get(a, Attribute)
                if srcAttrClass is not dstAttrClass: continue
                srcParamClass = self.params.get(a, params.paramDict.get(a, params.ParamText))
                dstParamClass = dstComp.params.get(a, params.paramDict.get(a, params.ParamText))
                if srcParamClass is not dstParamClass: continue
                # Style and exstyle are not in attributes and can be treated specially
                if a == 'style':
                    styles = self.getAttribute(srcNode, a).split('|')
                    allStyles = dstComp.styles + self.genericStyles
                    dstStyles = [s for s in styles if s.strip() in allStyles]
                    if dstStyles:
                        dstComp.addAttribute(dstNode, a, '|'.join(dstStyles))
                elif a == 'exstyle':
                    styles = self.getAttribute(srcNode, a).split('|')
                    allStyles = dstComp.exStyles + self.genericExStyles
                    dstStyles = [s for s in styles if s.strip() in allStyles]
                    if dstStyles:
                        dstComp.addAttribute(dstNode, a, '|'.join(dstStyles))
                elif a in dstComp.attributes:
                    value = self.getAttribute(srcNode, a)
                    dstComp.addAttribute(dstNode, a, value)

    def copyImplicitAttributes(self, srcNode, dstNode, dstComp):
        '''Copy relevant implicit attribute nodes from srcNode to dstNode.'''
        for n in srcNode.childNodes:
            if n.nodeType == n.ELEMENT_NODE and not is_object(n):
                a = n.tagName
                if a in dstComp.implicitAttributes:
                    value = self.getAttribute(srcNode, a)
                    dstComp.addAttribute(dstNode, a, value)


class SimpleComponent(Component):
    '''Component without window attributes and styles.'''
    windowAttributes = []
    genericStyles = genericExStyles = []


class Container(Component):
    '''Base class for containers.'''
    def canHaveChild(self, component):
        # Test exclusion first
        for g in self.groups:
            if '!'+component.groups[0] in parentChildGroups.get(g, []): return False
        # Test for any possible parent-child
        groups = Set(component.groups)
        for g in self.groups:
            if groups.intersection(parentChildGroups.get(g, [])):
                return True
        return False

    def isSizer(self):
        '''If this container manages children positions and sizes.'''
        return False

    def requireImplicit(self, node):
        '''If there are implicit nodes for this particular node.'''
        return False

    def getTreeNode(self, node):
        '''Some containers may hide some internal elements.'''
        return node

    def getTreeOrImplicitNode(self, node):
        '''Return topmost child (implicit if exists).'''
        return node

    def appendChild(self, parentNode, node):
        '''Append child node. Can be overriden to create implicit nodes.'''
        parentNode.appendChild(node)

    def insertBefore(self, parentNode, node, nextNode):
        '''Insert node before nextNode. Can be overriden to create implicit nodes.'''
        parentNode.insertBefore(node, nextNode)

    def insertAfter(self, parentNode, node, prevNode):
        '''Insert node after prevNode. Can be overriden to create implicit nodes.'''
        parentNode.insertBefore(node, prevNode.nextSibling)

    def removeChild(self, parentNode, node):
        '''
        Remove node and the implicit node (if present). Return
        top-level removed child.
        '''
        return parentNode.removeChild(node)

    def copyObjects(self, srcNode, dstNode):
        # Copy child objects only for the same group
        dstComp = Manager.getNodeComp(dstNode)
        if self.groups[0] != dstComp.groups[0]: return
        children = []
        for n in filter(is_object, srcNode.childNodes):
            n = self.getTreeNode(n)
            if dstComp.canHaveChild(Manager.getNodeComp(n)):
                dstComp.appendChild(dstNode, n)

    def replaceChild(self, parentNode, newNode, oldNode):
        '''Replace oldNode by newNode keeping relevant attributes.'''
        # Keep compatible children
        oldComp = Manager.getNodeComp(oldNode)
        oldComp.copyAttributes(oldNode, newNode)
        if oldComp.isContainer():
            oldComp.copyObjects(oldNode, newNode)
        parentNode.replaceChild(newNode, oldNode)

    def getChildObject(self, node, obj, index):
        """Get index'th child of a tested interface element."""
        if isinstance(obj, wx.Window) and obj.GetSizer():
            return obj.GetSizer()
        try:
            return obj.GetChildren()[index]
        except IndexError:
            return None


class SimpleContainer(Container):
    '''Container without window attributes and styles.'''
    windowAttributes = []
    genericStyles = genericExStyles = []


class RootComponent(Container):
    '''Special root component.'''
    windowAttributes = []
    genericStyles = genericExStyles = []
    hasName = False
    hasCode = False


class SmartContainer(Container):
    '''Base class for containers with implicit nodes.'''
    implicitRenameDict = {}
    def __init__(self, klass, groups, attributes, **kargs):
        Container.__init__(self, klass, groups, attributes, **kargs)
        self.implicitKlass = kargs.pop('implicit_klass')
        self.implicitPageName = kargs.pop('implicit_page')
        self.implicitAttributes = kargs.pop('implicit_attributes')
        # This is optional
        self.implicitParams = kargs.pop('implicit_params', {})

    def getTreeNode(self, node):
        if is_element(node) and node.getAttribute('class') == self.implicitKlass:
            for n in node.childNodes: # find first object
                if is_object(n): return n
        # Maybe some children are not implicit
        return node

    def getTreeOrImplicitNode(self, node):
        '''Return topmost child (implicit if exists).'''
        if node.parentNode.getAttribute('class') == self.implicitKlass:
            return node.parentNode
        else:
            return node

    def appendChild(self, parentNode, node):
        if self.requireImplicit(node):
            elem = Model.createObjectNode(self.implicitKlass)
            elem.appendChild(node)
            parentNode.appendChild(elem)
        else:
            parentNode.appendChild(node)

    def insertBefore(self, parentNode, node, nextNode):
        if self.requireImplicit(nextNode):
            nextNode = nextNode.parentNode
        if self.requireImplicit(node):
            elem = Model.createObjectNode(self.implicitKlass)
            elem.appendChild(node)
            parentNode.insertBefore(elem, nextNode)
        else:
            parentNode.insertBefore(node, nextNode)

    def insertAfter(self, parentNode, node, prevNode):
        if self.requireImplicit(prevNode):
            nextNode = prevNode.parentNode.nextSibling
        else:
            nextNode = prevNode.nextSibling
        if self.requireImplicit(node):
            elem = Model.createObjectNode(self.implicitKlass)
            elem.appendChild(node)
            parentNode.insertBefore(elem, nextNode)
        else:
            parentNode.insertBefore(node, nextNode)

    def removeChild(self, parentNode, node):
        if self.requireImplicit(node):
            implicitNode = node.parentNode
            return parentNode.removeChild(implicitNode)
        else:
            return parentNode.removeChild(node)

    def replaceChild(self, parentNode, newNode, oldNode):
        # Do similarly to Container for object child nodes
        oldComp = Manager.getNodeComp(oldNode)
        oldComp.copyAttributes(oldNode, newNode)
        if oldComp.isContainer():
            oldComp.copyObjects(oldNode, newNode)
        # Special treatment for implicit nodes
        if self.requireImplicit(oldNode):
            implicitNode = oldNode.parentNode
            if self.requireImplicit(newNode):
                implicitNode.replaceChild(newNode, oldNode)
            else:
                parentNode.replaceChild(newNode, implicitNode)
        else:
            if self.requireImplicit(newNode):
                elem = Model.createObjectNode(self.implicitKlass)
                elem.appendChild(newNode)
                parentNode.replaceChild(elem, oldNode)
            else:
                parentNode.replaceChild(newNode, oldNode)            

    def requireImplicit(self, node):
        # SmartContainer by default requires implicit
        return True

    def setImplicitParamClass(self, attrName, paramClass):
        '''Set special Param class.'''
        self.implicitParams[attrName] = paramClass


class Sizer(SmartContainer):
    '''Sizers are not windows and have common implicit node.'''
    windowAttributes = []
    hasName = False
    genericStyles = []
    genericExStyles = []    
    renameDict = {'orient':'orientation'}
    implicitRenameDict = {'option':'proportion'}
    def __init__(self, klass, groups, attributes, **kargs):
        kargs.setdefault('implicit_klass', 'sizeritem')
        kargs.setdefault('implicit_page', 'SizerItem')
        kargs.setdefault('implicit_attributes', ['option', 'flag', 'border', 'minsize', 'ratio'])
        kargs.setdefault('implicit_params', {'option': params.ParamInt, 
                                             'minsize': params.ParamPosSize, 
                                             'ratio': params.ParamPosSize})
        SmartContainer.__init__(self, klass, groups, attributes, **kargs)

    def isSizer(self):
        return True

    def requireImplicit(self, node):
        return is_element(node) and node.getAttribute('class') != 'spacer'

    def getChildObject(self, node, obj, index):
        obj = obj.GetChildren()[index]
        if obj.IsSizer():
            return obj.GetSizer()
        elif obj.IsWindow():
            return obj.GetWindow()
        elif obj.IsSpacer():
            return obj.GetRect()
        return None                 # normally this is an error

    def getRect(self, obj):
        rects = [wx.RectPS(obj.GetPosition(), obj.GetSize())]
        for sizerItem in obj.GetChildren():
            rect = sizerItem.GetRect()
            rects.append(rect)
            # Add lines to show borders
            flag = sizerItem.GetFlag()
            border = sizerItem.GetBorder()
            if border == 0: continue
            x = (rect.GetLeft() + rect.GetRight()) / 2
            if flag & wx.TOP:
                rects.append(wx.Rect(x, rect.GetTop() - border, 0, border))
            if flag & wx.BOTTOM:
                rects.append(wx.Rect(x, rect.GetBottom() + 1, 0, border))
            y = (rect.GetTop() + rect.GetBottom()) / 2
            if flag & wx.LEFT:
                rects.append(wx.Rect(rect.GetLeft() - border, y, border, 0))
            if flag & wx.RIGHT:
                rects.append(wx.Rect(rect.GetRight() + 1, y, border, 0))
        return rects


class BoxSizer(Sizer):
    '''Sizers are not windows and have common implicit node.'''

    def __init__(self, klass, groups, attributes, **kargs):
        Sizer.__init__(self, klass, groups, attributes, **kargs)

    def getTreeImageId(self, node):
        if self.getAttribute(node, 'orient') == 'wxVERTICAL':
            return self.images[0].Id
        else:
            return self.images[1].Id

    def getRect(self, obj):
        rects = Sizer.getRect(self, obj)
        for sizerItem in obj.GetChildren():
            rect = sizerItem.GetRect()
            flag = sizerItem.GetFlag()
            if flag & wx.EXPAND:
                if obj.GetOrientation() == wx.VERTICAL:
                    y = (rect.GetTop() + rect.GetBottom()) / 2
                    rects.append(wx.Rect(rect.x, y, rect.width, 0))
                else:
                    x = (rect.GetLeft() + rect.GetRight()) / 2
                    rects.append(wx.Rect(x, rect.y, 0, rect.height))
        return rects

    def setDefaults(self, node):
        if node.nodeType == node.COMMENT_NODE or \
            node.tagName == 'object_ref': return
        if self.requireImplicit(node):
            comp = Manager.getNodeComp(node)
            sizerItem = self.getTreeOrImplicitNode(node)
            if comp.isContainer():
                for a,v in g.conf.defaultsContainer.items():
                    self.addAttribute(sizerItem, a, v)
            else:
                for a,v in g.conf.defaultsControl.items():
                    self.addAttribute(sizerItem, a, v)        

    def appendChild(self, parentNode, node):
        Sizer.appendChild(self, parentNode, node)
        self.setDefaults(node)

    def insertBefore(self, parentNode, node, nextNode):
        Sizer.insertBefore(self, parentNode, node, nextNode)
        self.setDefaults(node)

    def insertAfter(self, parentNode, node, prevNode):
        Sizer.insertAfter(self, parentNode, node, prevNode)
        self.setDefaults(node)

################################################################################
    
class _ComponentManager:
    '''Component manager used to register component plugins.'''
    def __init__(self):
        self.rootComponent = RootComponent('root', ['root'], ['encoding'], 
                                           specials={'encoding': EncodingAttribute},
                                           params={'encoding': params.ParamEncoding})
        self.components = {}
        self.ids = {}
        self.firstId = self.lastId = -1
        self.menus = {}
        self.panels = {}
        self.menuNames = ['TOP_LEVEL', 'ROOT', 'bar', 'control', 'button', 'box', 
                          'container', 'sizer', 'custom']
        self.panelNames = ['Windows', 'Panels', 'Controls', 'Sizers',  'Menus',
                           'Gizmos', 'Custom']
        self.panelImages = {}

    def init(self):
        self.firstId = self.lastId = wx.NewId()
        self.external = []      # external resources
        self.handlers = []      # registered XmlHandlers

    def register(self, component):
        '''Register component object.'''
        TRACE('register %s' % component.klass)
        self.components[component.klass] = component
        # unique wx ID for event handling
        component.id = self.lastId = wx.NewId()
        self.ids[component.id] = component

    def forget(self, klass):
        '''Remove registered component.'''
        del self.components[klass]
        for menu,iclh in self.menus.items():
            if iclh[1].klass == klass:
                self.menus[menu].remove(iclh)
        for panel,icb in self.panels.items():
            if icb[1].klass == klass:
                self.panels[panel].remove(icb)

    def getNodeComp(self, node, defaultClass='unknown'):
        # For ref nodes without class name, need to find ref element
        if node is Model.mainNode:
            return self.rootComponent
        elif node.nodeType == node.COMMENT_NODE:
            return self.components['comment']
        cls = node.getAttribute('class')
        if node.tagName == 'object_ref':
            if not cls:
                refNode = Model.findResource(node.getAttribute('ref'))
                if refNode:
                    cls = refNode.getAttribute('class')
                else:
                    cls = 'unknown'
        if defaultClass and cls not in self.components:
            cls = defaultClass
        return self.components[cls]

    def getMenuData(self, menu):
        return self.menus.get(menu, None)

    def setMenu(self, component, menu, label, help, index=999999):
        '''Set pulldown menu data.'''
        if menu not in self.menuNames: self.menuNames.append(menu)
        if menu not in self.menus: self.menus[menu] = []
        bisect.insort_left(self.menus[menu], (index, component, label, help))

    def getPanelData(self, panel):
        return self.panels.get(panel, None)

    def setTool(self, component, panel, bitmap=None, 
                pos=DEFAULT_POS, span=(1,1)):
        '''Set toolpanel data.'''
        if panel not in self.panelNames: self.panelNames.append(panel)
        if panel not in self.panels: self.panels[panel] = []
        # Auto-select bitmap if not provided
        if not bitmap:
            bmpPath = os.path.join('bitmaps', component.klass + '.png')
            if os.path.exists(bmpPath):
                bitmap = wx.Bitmap(bmpPath)
            else:
                bitmap = images.ToolDefault.GetBitmap()
        if g.conf.toolIconScale != 100:
            im = bitmap.ConvertToImage().Scale(
                bitmap.GetWidth() * g.conf.toolIconScale / 100,
                bitmap.GetHeight() * g.conf.toolIconScale / 100)
            bitmap = im.ConvertToBitmap()
        bisect.insort_left(self.panels[panel], (pos, span, component, bitmap))

    def addXmlHandler(self, h):
        '''
        Add an XML resource handler. h must be a class derived from
        XmlResourceHandler or a function loaded from a dynamic library
        using ctypes.
        '''
        self.handlers.append(h)
        
    def findById(self, id):
        return self.ids[id]

    def addXmlHandlers(self, res):
        '''Register XML handlers before creating a test window.'''
        for h in self.handlers:
            TRACE('registering Xml handler %s', h)
            if g._CFuncPtr and isinstance(h, g._CFuncPtr):
                try:
                    apply(h, ())
                except:
                    logger.exception('error calling DL func "%s"', h)
                    wx.LogError('error calling DL func "%s"' % h)
            else:               # assume a python class handler
                try:
                    res.AddHandler(apply(h, ()))
                except:
                    logger.exception('error adding XmlHandler "%s"', h)
                    wx.LogError('error adding XmlHandler "%s"' % h)

    def addExternal(self, f):
        '''Add an external resource file f to the list of preloaded
        resources.'''
        self.external.append(f)
        Model.addExternal(f)

    def preload(self, res):
        '''Preload external resources.'''
        for f in self.external:
            TRACE('Loading external resources: %s', f)
            res.Load(f)
        

# Singleton object
Manager = _ComponentManager()
'''Singleton global object of L{_ComponentManager} class.'''

c = SimpleComponent('comment', ['top_level', 'control'], ['comment'],
                    specials={'comment': CommentAttribute},
                    image=images.TreeComment.GetImage())
c.hasName = False
c.hasCode = False
c.setParamClass('comment', params.ParamMultilineText)
Manager.register(c)

