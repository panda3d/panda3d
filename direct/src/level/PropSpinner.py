from IntervalGlobal import * 
import Entity

class PropSpinner(Entity.Entity):
    def __init__(self, level, entId):
        Entity.Entity.__init__(self, level, entId)
        self.initProps()

    def destroy(self):
        self.destroyProps()
        Entity.Entity.destroy(self)        

    def initProps(self):
        topNode = self.getZoneNode()
        props = topNode.findAllMatches("**/Prop_*").asList() 
        spinTracks = Parallel() 
        for prop in props: 
            name = prop.getName() 
            nameParts = name.split('_') 
            # ['Prop', 'Rotate', 'Y', '15', 'Gear2'] 
            axis = nameParts[2]
            rate = 0
            neg = (string.upper(nameParts[3][0]) == 'N')
            if neg:
                nameParts[3] = nameParts[3][1:]
            try:
                rate = int(nameParts[3])
            except:
                print 'invalid prop rotate string: %s' % name
            if neg:
                rate = -rate
            prop.setHpr(0,0,0) 
            if axis == "X": 
                hpr = Vec3(0, rate*360, 0) 
            elif axis == "Y": 
                hpr = Vec3(rate*360, 0, 0) 
            elif axis == "Z": 
                hpr = Vec3(0, 0, rate*360) 
            else: 
                print 'error', axis 
            spinTracks.append(LerpHprInterval(prop, 60, hpr)) 
        spinTracks.loop() 
        self.spinTracks = spinTracks

    def destroyProps(self):
        if hasattr(self, 'spinTracks'):
            self.spinTracks.pause()
            del self.spinTracks

    if __dev__:
        def attribChanged(self, *args):
            self.destroyProps()
            self.initProps()
