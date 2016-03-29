"""
Palette for Prototyping
"""
import imp

from .ObjectPaletteBase import *

class ProtoPaletteBase(ObjectPaletteBase):
    def __init__(self):
        ObjectPaletteBase.__init__(self)
        # self.dirname should be defined in inherited class
        assert self.dirname

    def addItems(self):
        if type(protoData) == dict:
            for key in list(protoData.keys()):
                if type(protoData[key]) == dict:
                    self.add(key, parent)
                    self.addItems(protoData[key], key)
                else:
                    self.add(protoData[key], parent)

    def populate(self):
        moduleName = 'protoPaletteData'
        try:
            file, pathname, description = imp.find_module(moduleName, [self.dirname])
            module = imp.load_module(moduleName, file, pathname, description)
            self.data = module.protoData
            self.dataStruct = module.protoDataStruct
        except:
            print("protoPaletteData doesn't exist")
            return

        #self.addItems()

    def saveProtoDataStruct(self, f):
        if not f:
            return

        for key in list(self.dataStruct.keys()):
            f.write("\t'%s':'%s',\n"%(key, self.dataStruct[key]))

    def saveProtoData(self, f):
        if not f:
           return

        for key in list(self.data.keys()):
            if isinstance(self.data[key], ObjectBase):
               f.write("\t'%s':ObjectBase(name='%s', model='%s', anims=%s, actor=%s),\n"%(key, self.data[key].name, self.data[key].model, self.data[key].anims, self.data[key].actor))
            else:
               f.write("\t'%s':ObjectGen(name='%s'),\n"%(key, self.data[key].name))

    def saveToFile(self):
        try:
            f = open(self.dirname + '/protoPaletteData.py', 'w')
            f.write("from direct.leveleditor.ObjectPaletteBase import *\n\n")
            f.write("protoData = {\n")
            self.saveProtoData(f)
            f.write("}\n")
            f.write("protoDataStruct = {\n")
            self.saveProtoDataStruct(f)
            f.write("}\n")
            f.close()
        except:
            pass
