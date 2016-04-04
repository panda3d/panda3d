"""
Palette for Prototyping
"""
import os
import imp


class ProtoObjs:
    def __init__(self, name):
        self.dirname = os.path.dirname(__file__)
        self.name = name;
        self.filename = "/%s.py"%(name)
        self.data = {}

    def populate(self):
        moduleName = self.name
        try:
            file, pathname, description = imp.find_module(moduleName, [self.dirname])
            module = imp.load_module(moduleName, file, pathname, description)
            self.data = module.protoData
        except:
            print("%s doesn't exist"%(self.name))
            return

    def saveProtoData(self, f):
        if not f:
           return

        for key in self.data.keys():
            f.write("\t'%s':'%s',\n"%(key, self.data[key]))

    def saveToFile(self):
        try:
            f = open(self.dirname + self.filename, 'w')
            f.write("protoData = {\n")
            self.saveProtoData(f)
            f.write("}\n")
            f.close()
        except:
            pass
