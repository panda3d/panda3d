from direct.fsm import StatePush


def test_PushesStateChanges():
    psc = StatePush.PushesStateChanges(0)
    assert psc.getState() == 0
    psc.destroy()


def test_ReceivesStateChanges():
    rsc = StatePush.ReceivesStateChanges(None)
    rsc.destroy()


def test_StateVar():
    sv = StatePush.StateVar(0)
    assert sv.get() == 0
    sv.set(1)
    assert sv.get() == 1
    sv.destroy()


def test_StateChangeNode():
    sv = StatePush.StateVar(0)
    assert sv.get() == 0
    scn = StatePush.StateChangeNode(sv)
    assert scn.getState() == 0
    sv.set(1)
    assert sv.get() == 1
    assert scn.getState() == 1
    scn2 = StatePush.StateChangeNode(scn)
    assert scn2.getState() == 1
    sv.set(2)
    assert scn2.getState() == 2
    scn3 = StatePush.StateChangeNode(scn)
    assert scn3.getState() == 2
    sv.set(3)
    assert scn2.getState() == 3
    assert scn3.getState() == 3
    scn3.destroy()
    scn2.destroy()
    scn.destroy()
    sv.destroy()


def test_ReceivesMultipleStateChanges():
    rsc = StatePush.ReceivesMultipleStateChanges()
    sv = StatePush.StateVar(0)
    sv2 = StatePush.StateVar('b')
    rsc._subscribeTo(sv, 'a')
    rsc._subscribeTo(sv2, 2)
    rsc._unsubscribe('a')
    rsc.destroy()


def test_FunctionCall_1():
    l = []
    def handler1(value, l=l):
        l.append(value)
    assert not l
    sv = StatePush.StateVar(0)
    fc = StatePush.FunctionCall(handler1, sv)
    assert not l
    fc.pushCurrentState()
    assert l == [0,]
    sv.set(1)
    assert l == [0,1,]
    sv.set(2)
    assert l == [0,1,2,]
    fc.destroy()
    sv.destroy()


def test_FunctionCall_2():
    l = []
    def handler2(value, kDummy=None, kValue=None, l=l):
        l.append((value, kValue))
    assert not l
    sv = StatePush.StateVar(0)
    ksv = StatePush.StateVar('a')
    fc = StatePush.FunctionCall(handler2, sv, kValue=ksv)
    assert not l
    fc.pushCurrentState()
    assert l == [(0,'a',),]
    sv.set(1)
    assert l == [(0,'a'),(1,'a'),]
    ksv.set('b')
    assert l == [(0,'a'),(1,'a'),(1,'b'),]
    fc.destroy()
    sv.destroy()


def test_EnterExit():
    l = []
    def enter(l=l):
        l.append(1)
    def exit(l=l):
        l.append(0)
    sv = StatePush.StateVar(0)
    ee = StatePush.EnterExit(sv, enter, exit)
    sv.set(0)
    assert not l
    sv.set(1)
    assert l == [1,]
    sv.set(2)
    assert l == [1,]
    sv.set(0)
    assert l == [1,0,]
    sv.set(True)
    assert l == [1,0,1,]
    sv.set(False)
    assert l == [1,0,1,0,]
    ee.destroy()
    sv.destroy()


def test_Pulse():
    l = []
    def handler(value, l=l):
        l.append(value)
    p = StatePush.Pulse()
    fc = StatePush.FunctionCall(handler, p)
    assert not l
    fc.pushCurrentState()
    assert l == [False, ]
    p.sendPulse()
    assert l == [False, True, False, ]
    p.sendPulse()
    assert l == [False, True, False, True, False, ]
    fc.destroy()
    p.destroy()


def test_AttrSetter():
    from types import SimpleNamespace
    o = SimpleNamespace()
    svar = StatePush.StateVar(0)
    aset = StatePush.AttrSetter(svar, o, 'testAttr')
    assert hasattr(o, 'testAttr')
    assert o.testAttr == 0
    svar.set('red')
    assert o.testAttr == 'red'
    aset.destroy()
    svar.destroy()
