
    def __str__(self):
        return self.getStringValue()

    def __hash__(self):
        raise AttributeError, "ConfigVariables are not immutable."

    def ls(self):
        from pandac.Notify import Notify
        self.write(Notify.out())

    def __int__(self):
        return int(self.getValue())

    def __long__(self):
        return long(self.getValue())

    def __float__(self):
        return float(self.getValue())

    def __nonzero__(self):
        return bool(self.getValue())

    def __oct__(self):
        return oct(self.getValue())

    def __hex__(self):
        return hex(self.getValue())

    def __cmp__(self, other):
        return self.getValue().__cmp__(other)

    def __neg__(self):
        return -self.getValue()

    def __coerce__(self, other):
        return (self.getValue(), other)

    def __len__(self):
        return self.getNumWords()
    
    def __getitem__(self, n):
        if n < 0 or n >= self.getNumWords():
            raise IndexError
        return self.getWord(n)

    def __setitem__(self, n, value):
        if n < 0 or n > self.getNumWords():
            raise IndexError
        self.setWord(n, value)
