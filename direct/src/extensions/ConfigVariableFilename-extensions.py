
    def __str__(self):
        return self.cStr()

    def __len__(self):
        return self.length()
    
    def __getitem__(self, n):
        return self.cStr().__getitem__(n)
