import serial

class BufferedReader(serial.Serial):
    def __init__(self):
        self.buffer = []
    
    def read(self, size):
        while len(self.buffer) < size:
            self.buffer.append(serial.Serial.read(self, 1))     #block
            self.buffer.extend(list(serial.Serial.read(self, self.inWaiting())))  #and get what's there
        a, self.buffer = self.buffer[:size], self.buffer[size:] #take the requested amount
        return ''.join(a)
    
    def pushback(self, what):
        self.buffer = list(what) + self.buffer