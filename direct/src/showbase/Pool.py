"""

Pool is a collection of python objects that you can checkin and
checkout. This is useful for a cache of objects that are expensive to load
and can be reused over and over, like splashes on cannonballs, or
bulletholes on walls. The pool is unsorted. Items do not have to be unique
or be the same type.

Internally the pool is implemented with 2 lists, free items and used items.

p = Pool([1,2,3,4,5])
x = p.checkout()
p.checkin(x)

"""


from direct.directnotify import DirectNotifyGlobal

class Pool:

    notify = DirectNotifyGlobal.directNotify.newCategory("Pool")
    
    def __init__(self, free=[]):
        self.__free = free
        self.__used = []

    def add(self, item):
        """
        Add an item to the free list.
        """
        self.__free.append(item)

    def remove(self, item):
        """
        Remove an item. Error is flagged if the item is not in the pool.
        """
        if item in self.__free:
            self.__free.remove(item)
        if item in self.__used:
            self.__used.remove(item)
        self.notify.error("item not in pool")

    def checkout(self):
        """
        Get an arbitrary item from the pool.
        """
        if not self.__free:
            self.notify.error("no items are free")
        item = self.__free.pop()
        self.__used.append(item)
        return item

    def checkin(self, item):
        """
        Put back a checked out item.
        Error if the item is not checked out.
        """
        if item not in self.__used:
            self.notify.error("item is not checked out")
        self.__used.remove(item)
        self.__free.append(item)

    def reset(self):
        """
        Resets the pool so all items are free.
        """
        self.__free.extend(self.__used)
        self.__used = []

    def hasFree(self):
        """
        Returns true if there is at least one free item.
        """
        return (len(self.__free) != 0)

    def isFree(self, item):
        """
        Returns true if this item is free for check out.
        """
        return (item in self.__free)

    def isUsed(self, item):
        """
        Returns true if this item has already been checked out.
        """
        return (item in self.__used)
    
    def __repr__(self):        
        return "free = %s\nused = %s" % (self.__free, self.__used)


