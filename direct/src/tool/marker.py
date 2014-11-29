import direct.directbase.DirectStart
import random
from pandac.PandaModules import TextNode
from direct.showbase.DirectObject import DirectObject
from pandac.PandaModules import Vec4

#----------------------------- Global Variables ------------------------------

class Marker(TextNode):
    def __init__(self, tag, color, x, y,z, tag2):
        ##set all attributes to input
        self.tag = tag
        self.X = x
        self.Y = y
        self.Z = z
        self.id = tag2
        self.color = self.ColorCast(color)
        self.Display()
        
    def ColorCast(self, color):       
        #get a random number for the random case.       
        random1 = random.uniform(0,1)
        random2 = random.uniform(0,1)
        random3 = random.uniform(0,1)
            
        if color == 'RED':
            color_trans = Vec4(255, 0, 0, 1) 
        elif color == 'BLACK':
            color_trans = Vec4(0,0,0,1)
        elif color == 'ORANGE':
            color_trans = Vec4(255,165,0,1)
        elif color == 'PURPLE':
            color_trans = Vec4(128,0,128,1)
        elif color == 'GREEN':
            color_trans = Vec4(0,128,0,1)
        elif color == 'BLUE':
            color_trans = Vec4(0,0,255,1)
        elif color == 'WHITE':
            color_trans = Vec4(255,255,255,1)
        elif color == 'RANDOM':
            color_trans = Vec4(random1, random2, random3, 1)
        else:
            print '---------------------------------------'
            print '----------ERROR WITH COLOR-------------'
            print '------------Making Black---------------'
            color_trans = Vec4(0,0,0,1)
        
            
        return color_trans
    
    def Display(self):

        self.text = TextNode(self.tag)
        
        self.text.setText("Walking: " + str(self.id) + "      " 
                          + "POS: (" + str(self.X) + ", " + str(self.Y) + ")")#       TAG: " + self.tag)

        self.textNodePath = render.attachNewNode(self.text)
        self.textNodePath.setScale(.3)
        self.textNodePath.setPos(self.X,self.Y,self.Z)
        self.textNodePath.setColor(self.color)
        #print self.tag
        #print self.color
        #print '-----------------'
        self.textNodePath.setHpr(0,0,-90)
        self.textNodePath.setTwoSided(True)

        #billboarding done here
        self.textNodePath.setBillboardAxis()
        self.textNodePath.setBillboardPointWorld()
        self.textNodePath.setBillboardPointEye()      


class MarkerManager(Marker):
    id = 0.0
    markerList = []
    
    def __init__(self):
        print 'creating a MarkerManager'
    
    def Add(self, marker):
        #increment id each time we add to list.
        marker.id = self.id + 1
        
        #add marker to list
        self.markerList.append(marker)
        
        self.id = self.id + 1
    
    def PrintList(self):
        count = 0
         
        #get count
        for item in self.markerList:
            count = count + 1
        
        #prints out the count
        print "List Count: " + str(count)
        
        #print out attributes for each marker in the list
        for item in self.markerList:
            print item.tag
        
    def Delete(self, marker):
        print "deleting marker: " + str(marker.id)
        self.markerList.remove(marker)


if __name__ == '__main__':
    manager = MarkerManager()

    ## cool multicolor look
    for i in range(10):
        for j in range(10):
            test = Marker("Hello", "RANDOM", random.uniform(-10,10), random.uniform(-10,10),0,"True")
            manager.Add(test)
   
    manager.PrintList()

    run()
