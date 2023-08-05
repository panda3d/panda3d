from direct.showbase import PythonUtil
import pytest


def test_queue():
    q = PythonUtil.Queue()
    assert q.isEmpty()
    q.clear()
    assert q.isEmpty()
    q.push(10)
    assert not q.isEmpty()
    q.push(20)
    assert not q.isEmpty()
    assert len(q) == 2
    assert q.front() == 10
    assert q.back() == 20
    assert q.top() == 10
    assert q.top() == 10
    assert q.pop() == 10
    assert len(q) == 1
    assert not q.isEmpty()
    assert q.pop() == 20
    assert len(q) == 0
    assert q.isEmpty()


def test_flywheel():
    f = PythonUtil.flywheel(['a','b','c','d'], countList=[11,20,3,4])
    obj2count = {}
    for obj in f:
        obj2count.setdefault(obj, 0)
        obj2count[obj] += 1
    assert obj2count['a'] == 11
    assert obj2count['b'] == 20
    assert obj2count['c'] == 3
    assert obj2count['d'] == 4

    f = PythonUtil.flywheel([1,2,3,4], countFunc=lambda x: x*2)
    obj2count = {}
    for obj in f:
        obj2count.setdefault(obj, 0)
        obj2count[obj] += 1
    assert obj2count[1] == 2
    assert obj2count[2] == 4
    assert obj2count[3] == 6
    assert obj2count[4] == 8

    f = PythonUtil.flywheel([1,2,3,4], countFunc=lambda x: x, scale = 3)
    obj2count = {}
    for obj in f:
        obj2count.setdefault(obj, 0)
        obj2count[obj] += 1
    assert obj2count[1] == 1 * 3
    assert obj2count[2] == 2 * 3
    assert obj2count[3] == 3 * 3
    assert obj2count[4] == 4 * 3


def test_formatTimeCompact():
    ftc = PythonUtil.formatTimeCompact
    assert ftc(0) == '0s'
    assert ftc(1) == '1s'
    assert ftc(60) == '1m0s'
    assert ftc(64) == '1m4s'
    assert ftc(60*60) == '1h0m0s'
    assert ftc(24*60*60) == '1d0h0m0s'
    assert ftc(24*60*60 + 2*60*60 + 34*60 + 12) == '1d2h34m12s'


def test_formatTimeExact():
    fte = PythonUtil.formatTimeExact
    assert fte(0) == '0s'
    assert fte(1) == '1s'
    assert fte(2) == '2s'
    assert fte(61) == '1m1s'
    assert fte(60) == '1m'
    assert fte(60*60) == '1h'
    assert fte(24*60*60) == '1d'
    assert fte((24*60*60) + (2 * 60)) == '1d0h2m'
    del fte


def test_AlphabetCounter():
    tempList = []
    ac = PythonUtil.AlphabetCounter()
    for i in range(26*3):
        tempList.append(ac.next())
    assert tempList == [ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                        'AA','AB','AC','AD','AE','AF','AG','AH','AI','AJ','AK','AL','AM','AN','AO','AP','AQ','AR','AS','AT','AU','AV','AW','AX','AY','AZ',
                        'BA','BB','BC','BD','BE','BF','BG','BH','BI','BJ','BK','BL','BM','BN','BO','BP','BQ','BR','BS','BT','BU','BV','BW','BX','BY','BZ',]
    ac = PythonUtil.AlphabetCounter()
    num  = 26 # A-Z
    num += (26*26) # AA-ZZ
    num += 26 # AAZ
    num += 1 # ABA
    num += 2 # ABC
    for i in range(num):
        x = ac.next()
    assert x == 'ABC'


def test_unescape_html_string():
    assert PythonUtil.unescapeHtmlString('asdf') == 'asdf'
    assert PythonUtil.unescapeHtmlString('as+df') == 'as df'
    assert PythonUtil.unescapeHtmlString('as%32df') == 'as2df'
    assert PythonUtil.unescapeHtmlString('asdf%32') == 'asdf2'


def test_priority_callbacks():
    l = []
    def a(l=l):
        l.append('a')
    def b(l=l):
        l.append('b')
    def c(l=l):
        l.append('c')

    pc = PythonUtil.PriorityCallbacks()
    pc.add(a)
    pc()
    assert l == ['a']

    del l[:]
    bItem = pc.add(b)
    pc()
    assert 'a' in l
    assert 'b' in l
    assert len(l) == 2

    del l[:]
    pc.remove(bItem)
    pc()
    assert l == ['a']

    del l[:]
    pc.add(c, 2)
    bItem = pc.add(b, 10)
    pc()
    assert l == ['a', 'c', 'b']

    del l[:]
    pc.remove(bItem)
    pc()
    assert l == ['a', 'c']

    del l[:]
    pc.clear()
    pc()
    assert len(l) == 0

def test_weighted_choice():
    # Test PythonUtil.weightedChoice() with no valid list.
    with pytest.raises(IndexError):
        PythonUtil.weightedChoice([])

    # Create a sample choice list.
    # This contains a few tuples containing only a weight
    # and an arbitrary item.
    choicelist = [(3, 'item1'), (1, 'item2'), (7, 'item3')]

    # These are the items that we expect.
    items = ['item1', 'item2', 'item3']

    # Test PythonUtil.weightedChoice() with our choice list.
    item = PythonUtil.weightedChoice(choicelist)

    # Assert that what we got was at least an available item.
    assert item in items

    # Create yet another sample choice list, but with a couple more items.
    choicelist = [(2, 'item1'), (25, 'item2'), (14, 'item3'), (5, 'item4'),
                  (7, 'item5'), (3, 'item6'), (6, 'item7'), (50, 'item8')]

    # Set the items that we expect again.
    items = ['item1', 'item2', 'item3', 'item4', 'item5', 'item6', 'item7', 'item8']

    # The sum of all of the weights is 112.
    weightsum = 2 + 25 + 14 + 5 + 7 + 3 + 6 + 50

    # Test PythonUtil.weightedChoice() with the sum.
    item = PythonUtil.weightedChoice(choicelist, sum=weightsum)

    # Assert that we got a valid item (most of the time this should be 'item8').
    assert item in items

    # Test PythonUtil.weightedChoice(), but with an invalid sum.
    item = PythonUtil.weightedChoice(choicelist, sum=1)

    # Assert that we got 'item1'.
    assert item == items[0]

    # Create a bogus random function.
    rnd = lambda: 0.5

    # Test PythonUtil.weightedChoice() with the bogus function.
    item = PythonUtil.weightedChoice(choicelist, rng=rnd, sum=weightsum)

    # Assert that we got 'item6'.
    # We expect 'item6' because 0.5 multiplied by 112 is 56.0.
    # When subtracting that number by each weight, it will reach 0
    # by the time it hits 'item6' in the iteration.
    assert item == items[5]


def test_serial():
    gen = PythonUtil.SerialNumGen()
    assert gen.next() == 0
    assert next(gen) == 1
    assert next(gen) == 2
    assert gen.next() == 3


def test_alphabet_counter():
    counter = PythonUtil.AlphabetCounter()
    assert next(counter) == 'A'
    assert counter.next() == 'B'
    assert counter.next() == 'C'
    assert next(counter) == 'D'

    for i in range(26 - 4):
        next(counter)

    assert next(counter) == 'AA'
    assert next(counter) == 'AB'
