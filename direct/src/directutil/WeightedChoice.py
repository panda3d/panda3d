
import random

class WeightedChoice:
    def __init__(self, listOfLists, weightIndex=0):
        t=0
        for i in listOfLists:
            t+=i[weightIndex]
        self.total = t
        self.listOfLists = listOfLists
        self.weightIndex = weightIndex
   
    def choose(self):
        roll = random.randrange(self.total)
        weight = self.weightIndex
        for i in self.listOfLists:
            roll -= i[weight]
            if roll <= 0:
                return i                
