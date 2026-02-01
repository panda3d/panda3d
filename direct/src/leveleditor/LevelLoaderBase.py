import importlib.util


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

        try:
            spec = importlib.util.spec_from_file_location(fileName, filePath)
            if spec is None or spec.loader is None:
                raise ImportError

            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            return True
        except Exception:
            print(f'failed to load {fileName}')
            return None
