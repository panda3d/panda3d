from panda3d.core import UpdateSeq


def test_updateseq_initial():
    seq = UpdateSeq()
    assert seq == UpdateSeq.initial()

    assert seq.is_special()
    assert seq.is_initial()
    assert not seq.is_old()
    assert not seq.is_fresh()

    assert seq.seq == 0

    initial = UpdateSeq.initial()
    assert seq == initial
    assert seq >= initial
    assert seq <= initial
    assert not (seq != initial)
    assert not (seq > initial)
    assert not (seq < initial)

    fresh = UpdateSeq.fresh()
    assert not (seq == fresh)
    assert not (seq >= fresh)
    assert seq <= fresh
    assert seq != fresh
    assert not (seq > fresh)
    assert seq < fresh

    old = UpdateSeq.old()
    assert not (seq == old)
    assert not (seq >= old)
    assert not (seq > old)
    assert seq != old
    assert seq <= old
    assert seq < old


def test_updateseq_fresh():
    seq = UpdateSeq.fresh()

    assert seq.is_special()
    assert not seq.is_initial()
    assert not seq.is_old()
    assert seq.is_fresh()

    initial = UpdateSeq.initial()
    assert not (seq == initial)
    assert seq != initial
    assert seq > initial
    assert seq >= initial
    assert not (seq < initial)
    assert not (seq <= initial)

    fresh = UpdateSeq.fresh()
    assert seq == fresh
    assert seq >= fresh
    assert seq <= fresh
    assert not (seq != fresh)
    assert not (seq > fresh)
    assert not (seq < fresh)

    old = UpdateSeq.old()
    assert not (seq == old)
    assert not (seq <= old)
    assert not (seq < old)
    assert seq != old
    assert seq >= old
    assert seq > old


def test_updateseq_old():
    seq = UpdateSeq.old()

    assert seq.is_special()
    assert not seq.is_initial()
    assert seq.is_old()
    assert not seq.is_fresh()

    assert seq.seq == 1

    initial = UpdateSeq.initial()
    assert not (seq == initial)
    assert not (seq <= initial)
    assert not (seq < initial)
    assert seq != initial
    assert seq > initial
    assert seq >= initial

    fresh = UpdateSeq.fresh()
    assert not (seq == fresh)
    assert not (seq >= fresh)
    assert not (seq > fresh)
    assert seq <= fresh
    assert seq != fresh
    assert seq < fresh

    old = UpdateSeq.old()
    assert seq == old
    assert seq >= old
    assert seq <= old
    assert not (seq != old)
    assert not (seq > old)
    assert not (seq < old)
