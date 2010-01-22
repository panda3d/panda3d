"""
Palette for Prototyping
"""
import os
import imp
import types

from ObjectPaletteBase import *

class ProtoPalette(ObjectPaletteBase):
    def __init__(self):
        ObjectPaletteBase.__init__(self)

    def addItems(self, protoData, parent=None):
        if type(protoData) == types.DictType:
            for key in protoData.keys():
                if type(protoData[key]) == types.DictType:
                    self.add(key, parent)
                    self.addItems(protoData[key], key)
                else:
                    self.add(protoData[key], parent)
                    
    def populate(self):
        dirname = os.path.dirname(__file__)
        moduleName = 'protoPaletteData'
        try:
            file, pathname, description = imp.find_module(moduleName, [dirname])
            module = imp.load_module(moduleName, file, pathname, description)
        except:
            print "protoPaletteData doesn't exist"        
            return

        self.addItems(module.protoData)

    def saveProtoData(self, f, protoData, depth):
        tab = ' '*4*depth
        if not f:
            return

        for key in protoData.keys():
            f.write("%s'%s' : "%(tab, key))
            if type(protoData[key]) == types.DictType:
                f.write("{\n")
                self.saveProtoData(f, protoData[key], depth + 1)
                f.write("%s},\n"%tab)
            else:
                f.write("ObjectBase(name='%s', model='%s', anims=%s, actor=True),\n"%(protoData[key].name, protoData[key].model, protoData[key].anims))

    def saveToFile(self):
        try:
            f = open(os.path.dirname(__file__) + '/protoPaletteData.py', 'w')
            f.write("from direct.leveleditor.ObjectPaletteBase import *\n\n")
            f.write("protoData = {\n")
            self.saveProtoData(f, self.data, 1)
            f.write("}\n")
            f.close()
        except:
            pass
