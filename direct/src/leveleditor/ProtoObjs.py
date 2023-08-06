"""
Palette for Prototyping
"""
import os
import imp


class ProtoObjs:
    def __init__(self, name):
        self.dirname = os.path.dirname(__file__)
        self.name = name
        self.filename = f"/{name}.py"
        self.data = {}

    def populate(self):
        moduleName = self.name
        try:
            file, pathname, description = imp.find_module(moduleName, [self.dirname])
            module = imp.load_module(moduleName, file, pathname, description)
            self.data = module.protoData
        except Exception:
            print(f"{self.name} doesn't exist")
            return

    def saveProtoData(self, f):
        if not f:
            return

        for key, value in self.data.items():
            f.write(f"\t'{key}':'{value}',\n")

    def saveToFile(self):
        try:
            f = open(self.dirname + self.filename, 'w')
            f.write("protoData = {\n")
            self.saveProtoData(f)
            f.write("}\n")
            f.close()
        except Exception:
            pass
