import imp

class LevelLoaderBase:
    """
    Base calss for LevelLoader

    which you will use to load level editor data in your game.
    Refer LevelLoader.py for example.
    """
    def __init__(self):
        self.defaultPath = None # this should be set in your LevelLoader.py
        self.initLoader()

    def initLoader(self):
        # You should implement this in subclass
        raise NotImplementedError('populate() must be implemented in your LevelLoader.py')

    def cleanUp(self):
        # When you don't need to load any more data, you can call clean up
        del base.objectPalette
        del base.protoPalette
        del base.objectHandler
        del base.objectMgr

    def loadFromFile(self, fileName, filePath=None):
        if filePath is None:
            filePath = self.defaultPath

        if fileName.endswith('.py'):
            fileName = fileName[:-3]
        file, pathname, description = imp.find_module(fileName, [filePath])
        try:
            module = imp.load_module(fileName, file, pathname, description)
            return True
        except:
            print('failed to load %s'%fileName)
            return None
