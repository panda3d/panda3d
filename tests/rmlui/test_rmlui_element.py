import pytest

rmlui = pytest.importorskip("panda3d.rmlui")

from .conftest import load


def test_inner_rml(rml_context):
    doc = load(rml_context, "<div id='d'>before</div>")
    el = doc.get_element_by_id("d")
    assert el.get_inner_rml() == "before"
    el.set_inner_rml("after")
    assert el.get_inner_rml() == "after"


def test_attributes(rml_context):
    doc = load(rml_context, "<div id='d' data-x='1'></div>")
    el = doc.get_element_by_id("d")

    assert el.has_attribute("data-x")
    assert el.get_attribute("data-x") == "1"
    assert el.get_attribute("missing", "fallback") == "fallback"

    el.set_attribute("data-y", "2")
    assert el.get_attribute("data-y") == "2"

    el.remove_attribute("data-x")
    assert not el.has_attribute("data-x")


def test_classes(rml_context):
    doc = load(rml_context, "<div id='d'></div>")
    el = doc.get_element_by_id("d")

    assert not el.is_class_set("active")
    el.set_class("active", True)
    assert el.is_class_set("active")
    el.set_class("active", False)
    assert not el.is_class_set("active")


def test_id_property(rml_context):
    doc = load(rml_context, "<div id='theid'></div>")
    el = doc.get_element_by_id("theid")
    assert el.get_id() == "theid"


def test_children_and_traversal(rml_context):
    doc = load(rml_context,
               "<div id='parent'><span class='c'>a</span><span class='c'>b</span></div>")
    parent = doc.get_element_by_id("parent")

    assert parent.get_num_children() == 2
    first = parent.get_child(0)
    assert first is not None
    assert first.get_inner_rml() == "a"

    # query_selector finds the first matching descendant.
    found = parent.query_selector(".c")
    assert found is not None
    assert found.get_inner_rml() == "a"

    # get_parent_node walks back up.
    back = first.get_parent_node()
    assert back is not None
    assert back.get_id() == "parent"


def test_create_append_remove(rml_context):
    doc = load(rml_context, "<div id='container'></div>")
    container = doc.get_element_by_id("container")
    assert container.get_num_children() == 0

    child = doc.create_element("div")
    assert child is not None
    child.set_inner_rml("dynamic")

    appended = container.append_child(child)
    assert appended is not None
    assert container.get_num_children() == 1
    assert appended.get_inner_rml() == "dynamic"

    container.remove_child(appended)
    rml_context.update()
    assert container.get_num_children() == 0


def test_set_and_remove_property(rml_context):
    # A bare <div> defaults to display:inline (no UA stylesheet for in-memory
    # docs), so force block layout to make width measurable.
    doc = load(rml_context,
               "<div id='d' style='display:block;width:30px;height:30px;'></div>")
    el = doc.get_element_by_id("d")

    assert el.set_property("width", "80px")
    rml_context.update()
    assert el.get_offset_width() == pytest.approx(80, abs=1)

    # A malformed value is rejected (returns False) rather than raising.
    assert not el.set_property("width", "not-a-length")

    el.remove_property("width")
    rml_context.update()


def test_geometry_queries(rml_context):
    doc = load(rml_context,
               "<div id='d' style='display:block;width:40px;height:25px;'></div>")
    el = doc.get_element_by_id("d")
    rml_context.update()

    assert el.get_offset_width() == pytest.approx(40, abs=1)
    assert el.get_offset_height() == pytest.approx(25, abs=1)

    rel = el.get_relative_offset()
    abs_off = el.get_absolute_offset()
    assert rel.x >= 0 and rel.y >= 0
    assert abs_off.x >= 0 and abs_off.y >= 0


def test_pseudo_class_query(rml_context):
    doc = load(rml_context, "<div id='d'></div>")
    el = doc.get_element_by_id("d")
    # With no mouse over it, :hover is not set; this just exercises the query.
    assert el.is_pseudo_class_set("hover") is False


def test_scroll_into_view_does_not_crash(rml_context):
    doc = load(rml_context,
               "<div style='display:block;height:50px;overflow:auto;'>"
               "<div id='target' style='display:block;height:500px;'></div></div>")
    target = doc.get_element_by_id("target")
    target.scroll_into_view(True)
    target.scroll_into_view(False)
    rml_context.update()


def test_form_control_value(rml_context):
    doc = load(rml_context,
               "<input id='inp' type='text' value='start'/>",
               head="")
    el = doc.get_element_by_id("inp")
    assert el.get_value() == "start"
    el.set_value("changed")
    assert el.get_value() == "changed"
