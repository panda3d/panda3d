from panda3d.core import GeomVertexArrayFormat, GeomVertexFormat, Geom


def test_format_arrays():
    array1 = GeomVertexArrayFormat("vertex", 3, Geom.NT_float32, Geom.C_point)
    array2 = GeomVertexArrayFormat("normal", 3, Geom.NT_float32, Geom.C_normal)
    array3 = GeomVertexArrayFormat("color", 4, Geom.NT_float32, Geom.C_color)
    array4 = GeomVertexArrayFormat("texcoord", 2, Geom.NT_float32, Geom.C_texcoord)

    # Verify initial refcounts
    assert array1.get_ref_count() == 1
    assert array2.get_ref_count() == 1
    assert array3.get_ref_count() == 1
    assert array4.get_ref_count() == 1

    format = GeomVertexFormat()

    def expect_arrays(*args):
        assert format.get_num_arrays() == len(args)
        assert len(format.arrays) == len(args)
        assert tuple(format.arrays) == args
        arrays = format.get_arrays()
        assert tuple(arrays) == args

        assert array1.get_ref_count() == 1 + arrays.count(array1) * 2
        assert array2.get_ref_count() == 1 + arrays.count(array2) * 2
        assert array3.get_ref_count() == 1 + arrays.count(array3) * 2
        assert array4.get_ref_count() == 1 + arrays.count(array4) * 2

    # Verify empty state
    expect_arrays()

    # Append to end
    format.add_array(array1)
    expect_arrays(array1,)
    format.add_array(array2)
    expect_arrays(array1, array2)
    format.add_array(array3)
    expect_arrays(array1, array2, array3)
    format.add_array(array4)
    expect_arrays(array1, array2, array3, array4)

    # Verify other accessors
    assert format.get_num_arrays() == 4
    assert len(format.arrays) == 4
    assert tuple(format.get_arrays()) == (array1, array2, array3, array4)

    # Remove from beginning
    format.remove_array(0)
    expect_arrays(array2, array3, array4)
    format.remove_array(0)
    expect_arrays(array3, array4)
    format.remove_array(0)
    expect_arrays(array4,)
    format.remove_array(0)
    expect_arrays()

    # Insert at end
    format.insert_array(0, array1)
    expect_arrays(array1,)
    format.insert_array(1, array2)
    expect_arrays(array1, array2)
    format.insert_array(2, array3)
    expect_arrays(array1, array2, array3)
    format.insert_array(3, array4)
    expect_arrays(array1, array2, array3, array4)

    # Remove from end
    format.remove_array(3)
    expect_arrays(array1, array2, array3)
    format.remove_array(2)
    expect_arrays(array1, array2)
    format.remove_array(1)
    expect_arrays(array1,)
    format.remove_array(0)
    expect_arrays()

    # Insert at beginning
    format.insert_array(0, array4)
    expect_arrays(array4,)
    format.insert_array(0, array3)
    expect_arrays(array3, array4)
    format.insert_array(0, array2)
    expect_arrays(array2, array3, array4)
    format.insert_array(0, array1)
    expect_arrays(array1, array2, array3, array4)

    # Remove from middle and insert back in middle
    format.remove_array(2)
    expect_arrays(array1, array2, array4)
    format.insert_array(2, array3)
    expect_arrays(array1, array2, array3, array4)
    format.remove_array(1)
    expect_arrays(array1, array3, array4)
    format.remove_array(1)
    expect_arrays(array1, array4)
    format.insert_array(1, array2)
    expect_arrays(array1, array2, array4)
    format.insert_array(2, array3)
    expect_arrays(array1, array2, array3, array4)

    # Clear
    format.clear_arrays()
    expect_arrays()

    # Add to end but with very high index
    format.insert_array(0x7fffffff, array1)
    expect_arrays(array1,)
    format.insert_array(0x7fffffff, array2)
    expect_arrays(array1, array2)
    format.insert_array(0x7fffffff, array3)
    expect_arrays(array1, array2, array3)
    format.insert_array(0x7fffffff, array4)
    expect_arrays(array1, array2, array3, array4)
