from direct.showbase import PythonUtil


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
