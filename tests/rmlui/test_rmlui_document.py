import pytest

rmlui = pytest.importorskip("panda3d.rmlui")

from .conftest import load


def test_title(rml_context):
    doc = load(rml_context, "<p>x</p>", head="<title>Hello</title>")
    assert doc.get_title() == "Hello"
    doc.set_title("Changed")
    assert doc.get_title() == "Changed"


def test_is_modal_default_false(rml_context):
    doc = load(rml_context, "<p>x</p>")
    assert doc.is_modal() is False


def test_show_hide(rml_context):
    # show() is called by load(); hide() must not crash and the document stays
    # loaded.
    doc = load(rml_context, "<p>x</p>")
    doc.hide()
    rml_context.update()
    assert rml_context.get_num_documents() == 1


def test_z_ordering(rml_context):
    # pull_to_front / push_to_back should not crash with multiple documents.
    a = load(rml_context, "<p>a</p>")
    b = load(rml_context, "<p>b</p>")
    a.pull_to_front()
    b.push_to_back()
    rml_context.update()
    assert rml_context.get_num_documents() == 2


def test_reload_style_sheet(rml_context):
    doc = load(rml_context, "<p>x</p>", head="<style>p { color: #fff; }</style>")
    # Should be a no-op that does not crash.
    doc.reload_style_sheet()
    rml_context.update()


def test_create_text_node(rml_context):
    doc = load(rml_context, "<div id='c'></div>")
    container = doc.get_element_by_id("c")

    text = doc.create_text_node("hello world")
    assert text is not None

    container.append_child(text)
    assert "hello world" in container.get_inner_rml()


def test_source_url(rml_context):
    doc = rml_context.load_document_from_memory(
        "<rml><head></head><body></body></rml>", "the-url")
    assert doc.get_source_url() == "the-url"


def test_close_is_idempotent(rml_context):
    doc = load(rml_context, "<p>x</p>")
    doc.close()
    rml_context.update()
    # Calling close() again, and other methods, must be safe no-ops.
    doc.close()
