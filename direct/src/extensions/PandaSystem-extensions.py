
    def getSystems(self):
        l = []
        for i in range(self.getNumSystems()):
            l.append(self.getSystem(l))
        return l
