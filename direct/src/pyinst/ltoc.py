import os, sys, UserList
import finder, tocfilter, resource

class lTOC(UserList.UserList):
    """ A class for managing lists of resources.
        Should be a UserList subclass. Doh. 
        Like a list, but has merge(other) and filter() methods """
    def __init__(self, reslist=None, filters=None):
        UserList.UserList.__init__(self, reslist)
        self.filters = []
        if filters is not None:
            self.filters = filters[:]
    def prepend(self, res):
        self.resources.insert(0, res)
    def merge(self, other):
        ' merge in another ltoc, discarding dups and preserving order '
        tmp = {}
        for res in self.data:
            tmp[res.name] = 0
        for res in other:
            if tmp.get(res.name, 1):
                self.data.append(res)
                tmp[res.name] = 0
    def filter(self):
        ' invoke all filters '
        for i in range(len(self.data)):
            res = self.data[i]
            if res:
                for f in self.filters:
                    if f.matches(res):
                        self.data[i] = None
                        break
        self.data = filter(None, self.data)
        return self
    def unique(self):
        ' remove all duplicate entries, preserving order '
        new = self.__class__()
        new.merge(self)
        self.data = new.data
    def toList(self):
        ' return self as a list of (name, path, typ) '
        tmp = []
        for res in self.data:
            tmp.append((res.name, res.path, res.typ))
        return tmp
    def addFilter(self, filter):
        if type(filter) == type(''):
            self.filters.append(finder.makeresource(filter).asFilter())
        else:
            if type(filter) == type(self):
                if isinstance(filter, tocfilter._Filter):
                    self.filters.append(filter)
                elif isinstance(filter, resource.resource):
                    self.filters.append(filter.asFilter())
                else:
                    raise ValueError, "can't make filter from %s", repr(filter)
            else:
                raise ValueError, "can't make filter from %s", repr(filter)
        print " added filter", `self.filters[-1]`             
            
   
if __name__ == '__main__':
    sys.path.insert(0, '.')
    import finder
    import pprint
    s = finder.scriptresource('finder.py', './finder.py')
    ##    pyltoc = lTOC(s.modules)
    ##    l1 = pyltoc.toList()
    ##    print "Raw py ltoc:", pprint.pprint(l1)
    ##    f1 = ModFilter(['dospath', 'macpath', 'posixpath'])
    ##    l2 = lTOC(s.modules).filter(f1).toList()
    ##    print "Filter out dospath, macpath, posixpath:", pprint.pprint(l2)
    ##    f2 = DirFilter(['.'])
    ##    l3 = lTOC(s.modules).filter(f2).toList()
    ##    print "Filter out current dir:", pprint.pprint(l3)
    ##    f3 = StdLibFilter()
    ##    l4 = lTOC(s.modules).filter(f3).toList()
    ##    print "Filter out stdlib:", pprint.pprint(l4)
    ##    #print "Filter out current dir and stdlib:", lTOC(s.modules).filter(f2, f3).toList()
    binltoc = lTOC(s.binaries)
    print "Raw bin ltoc:", pprint.pprint(binltoc.toList())
    binltoc.addFilter('c:/winnt/system32')
    pprint.pprint(binltoc.filter().toList())
    
    
