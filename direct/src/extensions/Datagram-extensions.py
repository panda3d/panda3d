
    def putArg(self, arg, subatomicType, divisor=1):
        # Import the type numbers
        import DCSubatomicType
        if (divisor == 1):
            if subatomicType == DCSubatomicType.STInt8:
                self.addInt8(int(arg))
            elif subatomicType == DCSubatomicType.STInt16:
                self.addInt16(int(arg))
            elif subatomicType == DCSubatomicType.STInt32:
                self.addInt32(int(arg))
            elif subatomicType == DCSubatomicType.STInt64:
                self.addInt64(int(arg))
            elif subatomicType == DCSubatomicType.STUint8:
                self.addUint8(int(arg))
            elif subatomicType == DCSubatomicType.STUint16:
                self.addUint16(int(arg))
            elif subatomicType == DCSubatomicType.STUint32:
                self.addUint32(int(arg))
            elif subatomicType == DCSubatomicType.STUint64:
                self.addUint64(int(arg))
            elif subatomicType == DCSubatomicType.STFloat64:
                self.addFloat64(arg)
            elif subatomicType == DCSubatomicType.STString:
                self.addString(arg)
            elif subatomicType == DCSubatomicType.STBlob:
                self.addString(arg)
            elif hasattr(DCSubatomicType, "STBlob32") and subatomicType == DCSubatomicType.STBlob32:
                self.addString32(arg)
            elif subatomicType == DCSubatomicType.STInt8array:
                self.addUint16(len(arg))
                for i in arg:
                    self.addInt8(int(i))
            elif subatomicType == DCSubatomicType.STInt16array:
                self.addUint16(len(arg) << 1)
                for i in arg:
                    self.addInt16(int(i))
            elif subatomicType == DCSubatomicType.STInt32array:
                self.addUint16(len(arg) << 2)
                for i in arg:
                    self.addInt32(int(i))
            elif subatomicType == DCSubatomicType.STUint8array:
                self.addUint16(len(arg))
                for i in arg:
                    self.addUint8(int(i))
            elif subatomicType == DCSubatomicType.STUint16array:
                self.addUint16(len(arg) << 1)
                for i in arg:
                    self.addUint16(int(i))
            elif subatomicType == DCSubatomicType.STUint32array:
                self.addUint16(len(arg) << 2)
                for i in arg:
                    self.addUint32(int(i))
            elif subatomicType == DCSubatomicType.STUint32uint8array:
                self.addUint16(len(arg) * 5)
                for i in arg:
                    self.addUint32(int(i[0]))
                    self.addUint8(int(i[1]))
            else:
                raise Exception("Error: No such type as: " + str(subatomicType))
        else:
            if subatomicType == DCSubatomicType.STInt8:
                self.addInt8(int(round(arg*divisor)))
            elif subatomicType == DCSubatomicType.STInt16:
                self.addInt16(int(round(arg*divisor)))
            elif subatomicType == DCSubatomicType.STInt32:
                self.addInt32(int(round(arg*divisor)))
            elif subatomicType == DCSubatomicType.STInt64:
                self.addInt64(int(round(arg*divisor)))
            elif subatomicType == DCSubatomicType.STUint8:
                self.addUint8(int(round(arg*divisor)))
            elif subatomicType == DCSubatomicType.STUint16:
                self.addUint16(int(round(arg*divisor)))
            elif subatomicType == DCSubatomicType.STUint32:
                self.addUint32(int(round(arg*divisor)))
            elif subatomicType == DCSubatomicType.STUint64:
                self.addUint64(int(round(arg*divisor)))
            elif subatomicType == DCSubatomicType.STInt8array:
                self.addUint16(len(arg))
                for i in arg:
                    self.addInt8(int(round(i*divisor)))
            elif subatomicType == DCSubatomicType.STInt16array:
                self.addUint16(len(arg) << 1)
                for i in arg:
                    self.addInt16(int(round(i*divisor)))
            elif subatomicType == DCSubatomicType.STInt32array:
                self.addUint16(len(arg) << 2)
                for i in arg:
                    self.addInt32(int(round(i*divisor)))
            elif subatomicType == DCSubatomicType.STUint8array:
                self.addUint16(len(arg))
                for i in arg:
                    self.addUint8(int(round(i*divisor)))
            elif subatomicType == DCSubatomicType.STUint16array:
                self.addUint16(len(arg) << 1)
                for i in arg:
                    self.addUint16(int(round(i*divisor)))
            elif subatomicType == DCSubatomicType.STUint32array:
                self.addUint16(len(arg) << 2)
                for i in arg:
                    self.addUint32(int(round(i*divisor)))
            elif subatomicType == DCSubatomicType.STUint32uint8array:
                self.addUint16(len(arg) * 5)
                for i in arg:
                    self.addUint32(int(round(i[0]*divisor)))
                    self.addUint8(int(round(i[1]*divisor)))
            else:
                raise Exception("Error: type does not accept divisor: " + str(subatomicType))
        return None


