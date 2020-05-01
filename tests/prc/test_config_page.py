from panda3d import core

def test_load_unload_page():
    var = core.ConfigVariableInt("test-var", 1)
    assert var.value == 1

    page = core.load_prc_file_data("test_load_unload_page", "test-var 2")
    assert page
    assert var.value == 2

    assert core.unload_prc_file(page)
    assert var.value == 1

def test_ini_syntax():
    page = core.load_prc_file_data("test_ini_syntax", "test-var=true\ntest-var2 = foo")
    assert page

    for decl in page.declarations:
        print(decl.variable)

    var = core.ConfigVariableBool("test-var", False)
    assert var.value == True

    var = core.ConfigVariableString("test-var2", "")
    assert var.value == "foo"
