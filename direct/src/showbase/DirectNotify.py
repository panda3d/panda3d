
"""DirectNotify module: this module contains the DirectNotify class"""

import Notifier
import Logger

class DirectNotify:
    """DirectNotify class: this class contains methods for creating
    mulitple notify categories via a dictionary of Notifiers."""
    
    def __init__(self):
        """__init__(self)
        DirectNotify class keeps a dictionary of Notfiers"""
        self.__categories = { }
        # create a default log file
        self.logger = Logger.Logger()

    def __str__(self):
        """__str__(self)
        Print handling routine"""
        return "DirectNotify categories: %s" % (self.__categories)

    #getters and setters
    def getCategories(self):
        """getCategories(self)
        Return list of category dictionary keys"""
        return (self.__categories.keys())

    def getCategory(self, categoryName):
        """getCategory(self, string)
        Return the category with given name if present, None otherwise"""
        return(self.__categories.get(categoryName, None))

    def addCategory(self, categoryName, category):
        """addCategory(self, Notifier)
        Add a given Notifier with given name to the category dictionary
        and return 0 if not present, else return 0. """
        if (self.__categories.has_key(categoryName)):
            print "Warning: DirectNotify: category '%s' already exists" % \
                  (categoryName)
            return(0)
        else:
            self.__categories[categoryName] = category
            return(1)

    def newCategory(self, categoryName):
        """newCategory(self, string)
        Make a new notify category named categoryName. Return new category
        if no such category exists, else return existing category"""
        if (not self.__categories.has_key(categoryName)):
            self.__categories[categoryName] = Notifier.Notifier(categoryName)
        else:
            print "Warning: DirectNotify: category '%s' already exists" % \
                  (categoryName)
        return(self.getCategory(categoryName))

           

#global DirectNotify for public access
directNotify = DirectNotify()





