import copy
import ObjectGlobals as OG

class ObjectBase:
    """ Base class for obj definitions """
    
    def __init__(self, name='', createFunction = None, model = None, models= [], anims = [], animNames = [], properties={},
                 movable = True, actor = False):
        self.name = name
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
        self.data = {}
        self.populate()

    def insertItem(self, item, parentName, data, obj=None):
        """
        You can insert item to obj palette tree.

        'item' is the name to be inserted, it can be either a group or obj.
        If item is a group 'obj' will be None.
        'parentName' is the name of parent under where this item will be inserted.
        'data' is used in recursive searching.
        
        """

        if type(data) != dict:
            return

        if parentName is None:
            # when adding a group to the root
            assert item not in data.keys()

            if obj is None:
                data[item] = {}
            else:
                data[item] = obj
            return

        if parentName in data.keys():
            if obj is None:
                data[parentName][item] = {}
            else:
                data[parentName][item] = obj
        else:
            for key in data.keys():
                self.insertItem(item, parentName, data[key], obj)
                
    def add(self, item, parentName = None):
        if type(item) == str:
            self.insertItem(item, parentName, self.data, None)
        else:
            self.insertItem(item.name, parentName, self.data, item)

    def findItem(self, name, data=None):
        if data is None:
            data = self.data

        if type(data) != dict:
            return None

        if name in data.keys():
            return data[name]
        else:
            for key in data.keys():
                result = self.findItem(name, data[key])
                if result:
                    return result
            return None
        
    def populate(self):
        # You should implement this in subclass
        raise NotImplementedError('populate() must be implemented in ObjectPalette.py')
