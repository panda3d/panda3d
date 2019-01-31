import os
import imp


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
            self.editor.updateStatusReadout('Sucessfully saved to %s'%fileName)
            self.editor.fNeedToSave = False
        except IOError:
            print('failed to save %s'%fileName)
            if f:
                f.close()

    def loadFromFile(self, fileName):
        dirname, moduleName = os.path.split(fileName)
        if moduleName.endswith('.py'):
            moduleName = moduleName[:-3]
        file, pathname, description = imp.find_module(moduleName, [dirname])
        try:
            module = imp.load_module(moduleName, file, pathname, description)
            self.editor.updateStatusReadout('Sucessfully opened file %s'%fileName)
            self.editor.fNeedToSave = False
        except:
            print('failed to load %s'%fileName)
