
    def __len__(self):
        return self.getValue().__len__()
    
    def __getitem__(self, n):
        return self.getValue().__getitem__(n)
    
    def __setitem__(self, n):
        value = self.getValue()
        value.__setitem__(n)
        self.assign(value)
