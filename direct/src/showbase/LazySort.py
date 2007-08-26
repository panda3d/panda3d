from itertools import tee

def itersorted(iterable, cmp = cmp, key = lambda x: x, reverse = False):
    """
    This function returns a generator object that yields sorted items from
    'iterable'.

    It implements a form of lazy sorting that's most useful in two cases:
    1) When you only need the first few values in the sorted data.
    2) When you want to amortize the cost of the sort over the time
       you use the data.

    It is to be considered a 'stable sort', where values with equivalent
    sorting criteria maintain their relative order as it is in the input
    data set.

    'cmp' MUST return values in [-1,0,1]. Otherwise, behavior is
    undefined, and will most likely be very incorrect.
    """

    # Notes:
    # Understanding the concepts of 'left' and 'right' here is important.
    # 'left' values are those that are yielded earlier in the sort. So
    # each subsequent value yielded is 'to the right' of the previous one.
    # A stack is used to maintain sets of values who share the same key
    # value.  Each layer corresponds to one key.  During the traversals of
    # the input data, values are added to each layer in such a way that
    # they maintain their relative position (to others in the same layer)
    # from the original data.  This ensures a 'stable sort'.
    
    # Create our working structures
    stack = []      # holds a stack of 'layers'.
                    # 'left' value layers are above 'right' ones.
    layer = ()      # A 3-tuple of the form:
                    # (key, data iterator, [values])
    init = True     # Is set to true for the first pass through
                    # the data.
    if reverse:     # Use this to easily switch the direction of the sort.
        rev = -1
    else:
        rev = 1

    # Create the base iterator that will track our
    # main progress through the data.
    a = ((key(x),x) for x in iterable)
        
    # Begin the main loop
    while 1:
        # If the stack is empty, we must now seed it.
        # Advance the base iterator until we find a value 'to the right' of
        # anything we've yielded so far. (All values 'to the left' have
        # already been yielded)
        if not stack:
            # pull next value off the base iterator
            k,val = a.next()

            # If init, get the first value and stop.
            # Otherwise, find the first value 'to the right'
            # of the most recently yielded value.
            while (not init) and (cmp(k,lLimit) != rev):
                k,val = a.next()
                pass

            # Place the found value as the initial stack value
            # (and store its iteration progress as well).
            a,b = tee(a)
            stack.append([k, b, [val]])
            pass


        # We now iterate through the data, starting where the value
        # at the top of the stack left off.
        layer = stack[-1]
        b = layer[1]
        for k,val in b:
            # If the next data element is 'to the left' of (or equal to)
            # the top off the stack and 'to the right' of the last element
            # yielded, add it to the stack.
            if cmp(k,layer[0]) != rev and (init or cmp(k,lLimit) == rev):
                # If it's 'to the left' of the current stack value,
                # make a new layer and add it to the top of the stack.
                # Otherwise, it's equivalent so we'll just append it
                # to the values in the top layer of the stack.
                if cmp(k,layer[0]) == -rev:
                    b,layer[1] = tee(b)
                    stack.append([k, b, []])
                    layer = stack[-1]
                    pass
                layer[2].append(val)
                pass
            pass
        # Remove the initialization condition to enable lLimit checking.
        init = False

        # Whatever values that are on the top stack at this point are
        # the 'left-most' we've found that we haven't yet yielded. Yield
        # them in the order that we discovered them in the source data.
        # Define lLimit as the right-most limit for values that have not
        # yet been yielded.  This will allow us to ignore these values
        # on future iterations.
        lLimit, b, vals = stack.pop()
        for val in vals:
            yield val
            pass

if __debug__:
    def P(i):
        for x in reversed(i):
            print x

    def test():
        import random
        from itertools import islice

        control = sorted(data, key = lambda x: x[0])
        variable = itersorted(data, key = lambda x: x[0])
        
        print control[:10] == [x for x in islice(variable,10)]
        print data
        print control

        variable = itersorted(data, key = lambda x: x[0])
        print [x for x in islice(variable,10)]

    from unittest import TestCase, main
    from random import shuffle
    from itertools import islice
    
    class LazySortTest(TestCase):
        """
        Run these tests with:
        > python LazySort.py
        """
        
        TESTLEN = 10
        RANGELEN = max(TESTLEN, 10)

        a = range(RANGELEN/2)*2
        b = range(RANGELEN/2)*2
        shuffle(a)
        shuffle(b)
        DATA = zip(a,b)
        shuffle(DATA)
        del a
        del b
        
        def testRange(self):
            control = sorted(self.DATA)
            variable = itersorted(self.DATA)
            self.assertEqual(control[:10], [x for x in islice(variable, self.TESTLEN)])
            
        def testRangeCompare(self):
            control = sorted(self.DATA, cmp = lambda a,b: -cmp(a,b))
            variable = itersorted(self.DATA, cmp = lambda a,b: -cmp(a,b))
            self.assertEqual(control[:10], [x for x in islice(variable, self.TESTLEN)])

        def testRangeKey(self):
            control = sorted(self.DATA, key = lambda x: x[0])
            variable = itersorted(self.DATA, key = lambda x: x[0])
            self.assertEqual(control[:10], [x for x in islice(variable, self.TESTLEN)])
        
        def testRangeReverse(self):
            control = sorted(self.DATA, reverse = True)
            variable = itersorted(self.DATA, reverse = True)
            self.assertEqual(control[:10], [x for x in islice(variable, self.TESTLEN)])

        def testRangeCompareKey(self):
            control = sorted(self.DATA, cmp = lambda a,b: -cmp(a,b),
                             key = lambda x: x[0])
            variable = itersorted(self.DATA, cmp = lambda a,b: -cmp(a,b),
                                  key = lambda x: x[0])
            self.assertEqual(control[:10], [x for x in islice(variable, self.TESTLEN)])
        
        def testRangeCompareReverse(self):
            control = sorted(self.DATA, cmp = lambda a,b: -cmp(a,b),
                             reverse = True)
            variable = itersorted(self.DATA, cmp = lambda a,b: -cmp(a,b),
                                  reverse = True)
            self.assertEqual(control[:10], [x for x in islice(variable, self.TESTLEN)])
            
        def testRangeKeyReverse(self):
            control = sorted(self.DATA, key = lambda x: x[0], reverse = True)
            variable = itersorted(self.DATA, key = lambda x: x[0], reverse = True)
            self.assertEqual(control[:10], [x for x in islice(variable, self.TESTLEN)])
            
            control = sorted(self.DATA, key = lambda x: x[1], reverse = True)
            variable = itersorted(self.DATA, key = lambda x: x[1], reverse = True)
            self.assertEqual(control[:10], [x for x in islice(variable, self.TESTLEN)])

        def testRangeCompareKeyReverse(self):
            control = sorted(self.DATA, cmp = lambda a,b: -cmp(a,b),
                             key = lambda x: x[0],
                             reverse = True)
            variable = itersorted(self.DATA, cmp = lambda a,b: -cmp(a,b),
                                  key = lambda x: x[0],
                                  reverse = True)
            self.assertEqual(control[:10], [x for x in islice(variable, self.TESTLEN)])
        

    if __name__ == '__main__':
        main() # unittest.main
    
