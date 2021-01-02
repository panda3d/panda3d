def test_pta_float_compare():
    from panda3d.core import PTA_float, CPTA_float

    # Two null PTAs
    assert PTA_float() == PTA_float()
    assert not (PTA_float() != PTA_float())

    # Two non-null PTAs
    assert PTA_float([1]) != PTA_float([1])
    assert not (PTA_float([1]) == PTA_float([1]))

    # A copy of a PTA
    pta = PTA_float([1])
    assert pta == PTA_float(pta)
    assert not (pta != PTA_float(pta))

    # A const copy of a PTA
    pta = PTA_float([1])
    cpta = CPTA_float(pta)
    assert pta == cpta
    assert not (pta != cpta)


def test_pta_float_pickle():
    from panda3d.core import PTA_float
    from direct.stdpy.pickle import dumps, loads, HIGHEST_PROTOCOL

    null_pta = PTA_float()

    empty_pta = PTA_float([])

    data_pta = PTA_float([1.0, 2.0, 3.0])
    data = data_pta.get_data()

    for proto in range(1, HIGHEST_PROTOCOL + 1):
        null_pta2 = loads(dumps(null_pta, proto))
        assert null_pta2.is_null()
        assert len(null_pta2) == 0

        empty_pta2 = loads(dumps(empty_pta, proto))
        assert not empty_pta2.is_null()
        assert len(empty_pta2) == 0

        data_pta2 = loads(dumps(data_pta, proto))
        assert tuple(data_pta2) == (1.0, 2.0, 3.0)
        assert data_pta2.get_data() == data_pta.get_data()


def test_cpta_float_pickle():
    from panda3d.core import PTA_float, CPTA_float
    from direct.stdpy.pickle import dumps, loads, HIGHEST_PROTOCOL

    null_pta = CPTA_float(PTA_float())

    empty_pta = CPTA_float([])

    data_pta = CPTA_float([1.0, 2.0, 3.0])
    data = data_pta.get_data()

    for proto in range(1, HIGHEST_PROTOCOL + 1):
        null_pta2 = loads(dumps(null_pta, proto))
        assert null_pta2.is_null()
        assert len(null_pta2) == 0

        empty_pta2 = loads(dumps(empty_pta, proto))
        assert not empty_pta2.is_null()
        assert len(empty_pta2) == 0

        data_pta2 = loads(dumps(data_pta, proto))
        assert tuple(data_pta2) == (1.0, 2.0, 3.0)
        assert data_pta2.get_data() == data_pta.get_data()
