from panda3d import core

def test_text_assemble_null():
    # Tests that no is_whitespace() assert occurs
    assembler = core.TextAssembler(core.TextEncoder())
    assembler.set_wtext(u"\0test")
    assembler.assemble_text()
