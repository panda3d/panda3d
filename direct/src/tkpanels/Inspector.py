### Inspectors allow you to visually browse through the members of various python objects.
### To open an inspector, import this module, and execute inspector.inspect(anObject)
### I start IDLE with this command line: idle.py -c "from inspector import inspect"
### so that I can just type: inspect(anObject) any time.

import string
from Tkinter import *
from TkGlobal import *


### public API

def inspect(anObject):
    inspector = inspectorFor(anObject)
    inspectorWindow = InspectorWindow(inspector)
    inspectorWindow.open()

### private

def inspectorFor(anObject):
    typeName = string.capitalize(type(anObject).__name__) + 'Type'
    if _InspectorMap.has_key(typeName):
        inspectorName = _InspectorMap[typeName]
    else:
        print "Can't find an inspector for " + typeName
        inspectorName = 'Inspector'
    inspector = eval(inspectorName + '(anObject)')
    return inspector


### initializing

def initializeInspectorMap():
    global _InspectorMap
    notFinishedTypes = ['BufferType',  'EllipsisType',  'FrameType', 'TracebackType', 'XRangeType']

    _InspectorMap = {
        'Builtin_function_or_methodType' : 'FunctionInspector',
        'BuiltinFunctionType' : 'FunctionInspector',
        'BuiltinMethodType' : 'FunctionInspector',
        'ClassType' : 'ClassInspector',
        'CodeType' : 'CodeInspector',
        'ComplexType' : 'Inspector',
        'DictionaryType' : 'DictionaryInspector',
        'DictType' : 'DictionaryInspector',
        'FileType' : 'Inspector',
        'FloatType' : 'Inspector', 
        'FunctionType' : 'FunctionInspector',
        'Instance methodType' : 'InstanceMethodInspector',
        'InstanceType' : 'InstanceInspector',
        'IntType' : 'Inspector',
        'LambdaType' : 'Inspector',
        'ListType' : 'SequenceInspector',
        'LongType' : 'Inspector',
        'MethodType' : 'FunctionInspector',
        'ModuleType' : 'ModuleInspector',
        'NoneType' : 'Inspector',
        'SliceType' : 'SliceInspector',
        'StringType' : 'SequenceInspector',
        'TupleType' : 'SequenceInspector',
        'TypeType' : 'Inspector',
         'UnboundMethodType' : 'FunctionInspector'}

    for each in notFinishedTypes:
        _InspectorMap[each] = 'Inspector'

    
### Classes

class Inspector:
    def __init__(self, anObject):
        self.object = anObject
        self.initializePartsList()
        self.initializePartNames()

    def __str__(self):
        return __name__ + '(' + str(self.object) + ')'

    def initializePartsList(self):
        self._partsList = []
        keys = self.namedParts()
        keys.sort()
        for each in keys:
            self._partsList.append(each)
            #if not callable(eval('self.object.' + each)):
            #    self._partsList.append(each)  

    def initializePartNames(self):
        self._partNames = ['up'] + map(lambda each: str(each), self._partsList)

    def title(self):
        "Subclasses may override."
        return string.capitalize(self.objectType().__name__)
        
    def namedParts(self):
        return dir(self.object)

    def stringForPartNumber(self, partNumber):
        object = self.partNumber(partNumber)
        doc = None
        if callable(object):
            try:
                doc = object.__doc__
            except:
                pass
        if doc:
            return (str(object) + '\n' + str(doc))
        else:
            return str(object)

    def partNumber(self, partNumber):
        if partNumber == 0:
            return self.object
        else:
            part = self.privatePartNumber(partNumber)
            return eval('self.object.' + part)

    def inspectorFor(self, part):
        return inspectorFor(part)

    def privatePartNumber(self, partNumber):
        return self._partsList[partNumber - 1]        

    def partNames(self):
        return self._partNames
    
    def objectType(self):
        return type(self.object)

###
    
class ModuleInspector(Inspector):
    def namedParts(self):
        return ['__dict__']

class ClassInspector(Inspector):
    def namedParts(self):
        return ['__bases__'] + self.object.__dict__.keys()

    def title(self):
        return self.object.__name__ + ' Class'

class InstanceInspector(Inspector):
    def title(self):
        return self.object.__class__.__name__
    def namedParts(self):
        return ['__class__'] + dir(self.object)

###
    
class FunctionInspector(Inspector):
    def title(self):
        return self.object.__name__ + "()"

class InstanceMethodInspector(Inspector):
    def title(self):
        return str(self.object.im_class) + "." + self.object.__name__ + "()"

class CodeInspector(Inspector):
    def title(self):
        return str(self.object)

###

class ComplexInspector(Inspector):
    def namedParts(self):
        return ['real', 'imag']

###

class DictionaryInspector(Inspector):

    def initializePartsList(self):
        Inspector.initializePartsList(self)
        keys = self.object.keys()
        keys.sort()
        for each in keys:
            self._partsList.append(each)

    def partNumber(self, partNumber):
        if partNumber == 0:
            return self.object
        key = self.privatePartNumber(partNumber)
        return self.object[key]

class SequenceInspector(Inspector):
    def initializePartsList(self):
        Inspector.initializePartsList(self)
        for each in range(len(self.object)):
            self._partsList.append(each)

    def partNumber(self, partNumber):
        if partNumber == 0:
            return self.object
        index = self.privatePartNumber(partNumber)
        return self.object[index]
    
class SliceInspector(Inspector):
    def namedParts(self):
        return ['start', 'stop', 'step']


### Initialization
initializeInspectorMap()

class InspectorWindow:
    def __init__(self, inspector):
        self.inspectors = [inspector]

    def topInspector(self):
        return self.inspectors[len(self.inspectors) - 1]

    def inspectedObject(self):
        return self.topInspector().object

    def open(self):
        self.top= Toplevel(tkroot)
        self.createViews()
        self.update()

    #Private - view construction
    def createViews(self):
        self.createMenus()
        self.createListWidget()
        self.createTextWidget()
        # self.top.resizable(0, 0)

    def setTitle(self):
        self.top.title('Inspecting: ' + self.topInspector().title())

    def createListWidget(self):
        frame = Frame(self.top)
        frame.pack(side=LEFT, fill=BOTH, expand=1)
        # frame.grid(row=0, column=0, sticky=N+W+S+E)
        scrollbar = Scrollbar(frame, orient=VERTICAL)
        listWidget = self.listWidget = Listbox(frame, yscrollcommand=scrollbar.set)
        scrollbar.config(command = listWidget.yview)
        scrollbar.pack(side=RIGHT, fill=Y)
        listWidget.pack(side=LEFT, fill=BOTH, expand=1)
        listWidget.bind('<ButtonRelease-1>',  self.listSelectionChanged)
        listWidget.bind("<Double-Button-1>", self.popOrDive)

    def createTextWidget(self):
        self.textWidget = Text(self.top)
        self.textWidget.pack(side=RIGHT, fill=BOTH, expand=1)
        # self.textWidget.grid(row=0, column=1, columnspan=2, sticky=N+W+S+E)

    def createMenus(self):
        self.menuBar = Menu(self.top)
        self.top.config(menu=self.menuBar)
        inspectMenu = Menu(self.menuBar)
        self.menuBar.add_cascade(label='Inspect', menu=inspectMenu)
        inspectMenu.add_command(label='Pop', command=self.pop)
        inspectMenu.add_command(label='Dive', command=self.dive)
        inspectMenu.add_command(label='Inspect', command=self.inspect)
        helpMenu = Menu(self.menuBar)
        self.menuBar.add_cascade(label='Help', menu=helpMenu)
        helpMenu.add_command(label='Instructions', command=self.showHelp)

    def fillList(self):
        self.listWidget.delete(0, END)
        for each in self.topInspector().partNames():
            self.listWidget.insert(END, each)
        self.listWidget.select_clear(0)

    # Event Handling
    def listSelectionChanged(self, event):
        partNumber = self.selectedIndex()
        if partNumber == None:
            partNumber = 0
        string = self.topInspector().stringForPartNumber(partNumber)
        self.textWidget.delete('1.0', END)
        self.textWidget.insert(END, string)

    def popOrDive(self, event):
        '''The list has been double-clicked. If the selection is 'self' then pop,
        otherwise dive into the selected part'''
        if self.selectedIndex() == 0:
            self.pop()
        else:
            self.dive()

    # Menu Events
    def inspect(self):
        inspector = self.inspectorForSelectedPart()
        if inspector == None:
            return
        InspectorWindow(inspector).open()        
        
    def pop(self):
        if len(self.inspectors) > 1:
            self.inspectors = self.inspectors[ : -1]
            self.update()

    def dive(self):
        inspector = self.inspectorForSelectedPart()
        if inspector == None:
            return
        self.inspectors.append(inspector)
        self.update()

    def update(self):
        self.setTitle()
        self.fillList()
        self.listSelectionChanged(None)

    def showHelp(self):
        help = Toplevel(tkroot)
        help.title("Inspector Help")
        frame = Frame(help)
        frame.pack()
        text = Label(frame, text="Double click an instance variable to dive down\nDouble click self to pop back up")
        text.pack()

    #Private
    def selectedIndex(self):
        indicies = map(int, self.listWidget.curselection())
        if len(indicies) == 0:
            return None
        partNumber = indicies[0]
        return partNumber

    def inspectorForSelectedPart(self):
        partNumber = self.selectedIndex()
        if partNumber == None:
            return None
        part = self.topInspector().partNumber(partNumber)

        # If this is a node path, pop up a scene graph explorer
        from PandaModules import *
        import FSM
        if isinstance(part, NodePath):
            # part.place()
            top = Toplevel(tkroot)
            import SceneGraphExplorer
            sge = SceneGraphExplorer.SceneGraphExplorer(top, nodePath = part,
                                                        scrolledCanvas_hull_width = 200,
                                                        scrolledCanvas_hull_height = 400)
            sge.pack(fill = BOTH, expand = 0)
        elif isinstance(part, FSM.FSM):
            import FSMInspector
            fsmi = FSMInspector.FSMInspector(part)

        return self.topInspector().inspectorFor(part)

