    
    def __getitem__(self, n):
        if n < 0 or n >= self.getNumWords():
            raise IndexError
        return self.getWord(n)
