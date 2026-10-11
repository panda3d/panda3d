import os
import importlib.util


class FileMgr:
    """ To handle data file """

    def __init__(self, editor=None):
        self.editor = editor

    def saveToFile(self, fileName):
        try:
            f = open(fileName, 'w')
            f.write("from panda3d.core import *\n")
            f.write("\nif hasattr(base, 'le'):\n")
            f.write("    objectMgr = base.le.objectMgr\n")
            f.write("    animMgr = base.le.animMgr\n")
            f.write("    ui = base.le.ui\n")
            f.write("    ui.sceneGraphUI.reset()\n\n")
            f.write("else:\n")
            f.write("    objectMgr = base.objectMgr\n")
            f.write("# temporary place holder for nodepath\n")
            f.write("objects = {}\n")
            f.write("animMgr.keyFramesInfo = "+str(self.editor.animMgr.keyFramesInfo)+"\n")
            f.write("animMgr.curveAnimation = "+str(self.editor.animMgr.curveAnimation)+"\n")
            saveData = self.editor.objectMgr.getSaveData()
            for data in saveData:
                f.write(data)
                f.write('\n')
            saveDataLayers = self.editor.ui.layerEditorUI.getSaveData()
            for data in saveDataLayers:
                f.write(data)
                f.write('\n')
            f.close()
            self.editor.updateStatusReadout(f'Sucessfully saved to {fileName}')
            self.editor.fNeedToSave = False
        except IOError:
            print(f'failed to save {fileName}')
            if f:
                f.close()

    def loadFromFile(self, fileName):
        dirname, moduleName = os.path.split(fileName)
        if moduleName.endswith('.py'):
            moduleName = moduleName[:-3]
        spec = importlib.util.spec_from_file_location(moduleName, fileName)
        try:
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            self.editor.updateStatusReadout(f'Sucessfully opened file {fileName}')
            self.editor.fNeedToSave = False
        except Exception:
            print(f'failed to load {fileName}')
