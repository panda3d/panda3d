"""RandomNumGen module: contains the RandomNumGen class"""

__all__ = ['randHash', 'RandomNumGen']

from direct.directnotify import DirectNotifyGlobal
from panda3d.core import Mersenne

def randHash(num):
    """ this returns a random 16-bit integer, given a seed integer.
    It will always return the same output given the same input.
    This is useful for repeatably mapping numbers with predictable
    bit patterns (i.e. doIds or zoneIds) to numbers with random bit patterns
    """
    rng = RandomNumGen(num)
    return rng.randint(0, (1<<16) - 1)

class RandomNumGen:
    notify = \
      DirectNotifyGlobal.directNotify.newCategory("RandomNumGen")

    def __init__(self, seed):
        """seed must be an integer or another RandomNumGen"""
        if isinstance(seed, RandomNumGen):
            # seed this rng with the other rng
            rng = seed
            seed = rng.randint(0, 1 << 16)

        self.notify.debug("seed: " + str(seed))
        seed = int(seed)
        rng = Mersenne(seed)
        self.__rng = rng

    def __rand(self, N):
        """returns integer in [0..N)"""
        """
        # using modulus biases the numbers a little bit
        # the bias is worse for larger values of N
        return self.__rng.getUint31() % N
        """

        # this technique produces an even distribution.
        # random.py would solve this problem like so:
        # where:
        #   M=randomly generated number
        #   O=1 greater than the maximum value of M
        # return int(float(M)*(float(N)/float(O))) # M*(N/O)
        #
        # that generally works fine, except that it relies
        # on floating-point numbers, which are not guaranteed
        # to produce identical results on different machines.
        #
        # for our purposes, we need an entirely-integer approach,
        # since integer operations *are* guaranteed to produce
        # identical results on different machines.
        #
        # SO, we take the equation M*(N/O) and change the order of
        # operations to (M*N)/O.
        #
        # this requires that we have the ability to hold the result of
        # M*N. Luckily, Python has support for large integers. One
        # alternative would be to limit the RNG to a 16-bit range,
        # in which case we could do this math down in C++; but 16 bits
        # really doesn't provide a large enough range (0..65535).
        # Finally, since our O happens to be a power of two (0x80000000),
        # we can replace the divide with a shift.
        # boo-ya

        # the maximum for N ought to be 0x80000000, but Python treats
        # that as a negative number.
        assert N >= 0
        assert N <= 0x7fffffff

        return int((self.__rng.getUint31() * N) >> 31)

    def choice(self, seq):
        """returns a random element from seq"""
        return seq[self.__rand(len(seq))]

    def shuffle(self, x):
        """randomly shuffles x in-place"""
        for i in range(len(x) - 1, 0, -1):
            # pick an element in x[:i+1] with which to exchange x[i]
            j = int(self.__rand(i+1))
            x[i], x[j] = x[j], x[i]

    def randrange(self, start, stop=None, step=1):
        """randrange([start,] stop[, step])
        same as choice(range(start, stop[, step])) without construction
        of a list"""
        ## this was lifted from Python2.2's random.py
        # This code is a bit messy to make it fast for the
        # common case while still doing adequate error checking
        istart = int(start)
        if istart != start:
            raise ValueError("non-integer arg 1 for randrange()")
        if stop is None:
            if istart > 0:
                return self.__rand(istart)
            raise ValueError("empty range for randrange()")
        istop = int(stop)
        if istop != stop:
            raise ValueError("non-integer stop for randrange()")
        if step == 1:
            if istart < istop:
                return istart + self.__rand(istop - istart)
            raise ValueError("empty range for randrange()")
        istep = int(step)
        if istep != step:
            raise ValueError("non-integer step for randrange()")
        if istep > 0:
            n = (istop - istart + istep - 1) / istep
        elif istep < 0:
            n = (istop - istart + istep + 1) / istep
        else:
            raise ValueError("zero step for randrange()")

        if n <= 0:
            raise ValueError("empty range for randrange()")
        return istart + istep*int(self.__rand(n))

    def randint(self, a, b):
        """returns integer in [a, b]"""
        assert a <= b
        range = b-a+1
        r = self.__rand(range)
        return a+r

    # since floats are involved, I would recommend not trusting
    # this function for important decision points where remote
    # synchronicity is critical
    def random(self):
        """returns random float in [0.0, 1.0)"""
        return float(self.__rng.getUint31()) / float(1 << 31)
