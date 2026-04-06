
import random

# DCR: I added a weightedChoice() function to PythonUtil that supports
# floating-point weights and is intended for one-shot choices. It
# has an optional 'sum' argument that you can pass in if you know the
# sum of the weights and want to make repeated choices.

class WeightedChoice:
    def __init__(self, listOfLists, weightIndex=0):
        t=0
        for i in listOfLists:
            t+=i[weightIndex]
        self.total = t
        self.listOfLists = listOfLists
        self.weightIndex = weightIndex

    def choose(self, rng=random):
        roll = rng.randrange(self.total)
        weight = self.weightIndex
        for i in self.listOfLists:
            roll -= i[weight]
            if roll <= 0:
                return i
