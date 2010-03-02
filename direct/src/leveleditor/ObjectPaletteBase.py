import copy
import ObjectGlobals as OG

class ObjectGen:
    """ Base class for obj definitions """
    def __init__(self, name=''):
       self.name = name

class ObjectBase(ObjectGen):
    """ Base class for obj definitions """
    def __init__(self, name='', createFunction = None, model = None, models= [], anims = [], animNames = [], properties={},
                 movable = True, actor = False):
        ObjectGen.__init__(self, name)
        self.createFunction = createFunction
        self.model = model
        self.models = models[:]
        self.anims = anims[:]
        self.animNames = animNames[:]
        self.properties = copy.deepcopy(properties)
        self.movable = movable
        self.actor = actor

class ObjectPaletteBase:
    """
    Base class for objectPalette

    You should write your own ObjectPalette class inheriting this.
    Refer ObjectPalette.py for example.
    """

    def __init__(self):
        self.rootName = '_root'
        self.data = {}
        self.dataStruct = {}
        self.dataKeys = []
        self.populate()

    def insertItem(self, item, parentName):
        """
        You can insert item to obj palette tree.

        'item' is the object to be inserted, it can be either a group or obj.
        'parentName' is the name of parent under where this item will be inserted.
        """
        if type(self.data) != dict:
           return None

        if parentName is None:
           parentName = self.rootName

        self.dataStruct[item.name] = parentName
        self.data[item.name] = item
        self.dataKeys.append(item.name)

    def add(self, item, parentName = None):
        if type(item) == str:
           self.insertItem(ObjectGen(name = item), parentName)
        else:
           self.insertItem(item, parentName)

    def deleteStruct(self, name, deleteItems):
        try:
           item = self.data.pop(name)
           for key in self.dataStruct.keys():
               if self.dataStruct[key] == name:
                  node = self.deleteStruct(key, deleteItems)
                  if node is not None:
                     deleteItems[key] = node
           return item
        except:
           return None
        return None

    def delete(self, name):
        try:
           deleteItems = {}
           node = self.deleteStruct(name, deleteItems)
           if node is not None:
              deleteItems[name] = node 
           for key in deleteItems.keys():
               item = self.dataStruct.pop(key)
        except:
           return
        return

    def findItem(self, name):
        try:
            item = self.data[name]
        except:
            return None
        return item

    def rename(self, oldName, newName):
        #import pdb;set_trace()
        if oldName == newName:
           return False
        if newName == "":
           return False
        try:
            for key in self.dataStruct.keys():
                if self.dataStruct[key] == oldName:
                   self.dataStruct[key] = newName

            self.dataStruct[newName] = self.dataStruct.pop(oldName)
            item = self.data.pop(oldName)
            item.name = newName
            self.data[newName] = item
        except:
            return False
        return True

    def populate(self):
        # You should implement this in subclass
        raise NotImplementedError('populate() must be implemented in ObjectPalette.py')
