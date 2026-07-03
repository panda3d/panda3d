import pytest

rmlui = pytest.importorskip("panda3d.rmlui")

from .conftest import load


def test_context_dimensions_and_name(rml_region, rml_context):
    assert rml_context.get_name() == rml_region.get_context().get_name()
    assert rml_context.get_width() > 0
    assert rml_context.get_height() > 0


def test_load_document_from_memory(rml_context):
    doc = rml_context.load_document_from_memory(
        "<rml><head></head><body><p id='x'>hello</p></body></rml>")
    assert doc is not None
    assert doc.get_element_by_id("x") is not None


def test_load_document_from_memory_source_url(rml_context):
    doc = rml_context.load_document_from_memory(
        "<rml><head></head><body></body></rml>", "my-source")
    assert doc.get_source_url() == "my-source"

    doc2 = rml_context.load_document_from_memory(
        "<rml><head></head><body></body></rml>")
    assert doc2.get_source_url() == "[from memory]"


def test_num_documents(rml_context):
    assert rml_context.get_num_documents() == 0
    load(rml_context, "<p>a</p>")
    assert rml_context.get_num_documents() == 1
    load(rml_context, "<p>b</p>")
    assert rml_context.get_num_documents() == 2


def test_unload_all_documents(rml_context):
    load(rml_context, "<p>a</p>")
    load(rml_context, "<p>b</p>")
    assert rml_context.get_num_documents() == 2

    rml_context.unload_all_documents()
    assert rml_context.get_num_documents() == 0


def test_unload_single_document(rml_context):
    doc = load(rml_context, "<p>a</p>")
    load(rml_context, "<p>b</p>")
    assert rml_context.get_num_documents() == 2

    rml_context.unload_document(doc)
    rml_context.update()
    assert rml_context.get_num_documents() == 1


def test_get_element_at_point_hits_and_misses(rml_context):
    # A block element pinned to the top-left corner with a known size.
    load(
        rml_context,
        "<div id='d' style='position:absolute;left:0;top:0;"
        "width:40px;height:40px;'></div>",
    )
    rml_context.update()

    # A point inside the div resolves to it...
    hit = rml_context.get_element_at_point(20, 20)
    assert hit is not None
    assert hit.get_id() == "d"

    # ...and a point well outside it does not.
    miss = rml_context.get_element_at_point(500, 500)
    assert miss is None or miss.get_id() != "d"


def test_hover_default_none(rml_context):
    load(rml_context, "<p>a</p>")
    # With no mouse interaction no element is hovered.
    assert rml_context.get_hover_element() is None
