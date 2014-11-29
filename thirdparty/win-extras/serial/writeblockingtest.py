import serial
import time

def test():
    ser = serial.Serial(0, 300)
    print time.time()
    ser.write('*' * 120)
    print time.time()
    ser.close()

if __name__ == '__main__':
    test()