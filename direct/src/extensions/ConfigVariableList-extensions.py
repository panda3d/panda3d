
    def __hash__(self):
        raise AttributeError, "ConfigVariables are not immutable."

    def ls(self):
        from pandac.Notify import Notify
        self.write(Notify.out())

    def __cmp__(self, other):
        return list(self).__cmp__(list(other))

    def __len__(self):
        return self.getNumValues()
    
    def __getitem__(self, n):
        if n < 0 or n >= self.getNumUniqueValues():
            raise IndexError
        return self.getUniqueValue(n)
