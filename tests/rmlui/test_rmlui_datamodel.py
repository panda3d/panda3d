import gc
import pytest

rmlui = pytest.importorskip("panda3d.rmlui")

from .conftest import load


def test_create_and_get_data_model(rml_context):
    dm = rml_context.create_data_model("m")
    assert dm is not None
    assert dm.is_valid()

    # Re-creating the same name fails.
    assert rml_context.create_data_model("m") is None

    # get_data_model returns a handle to the existing model.
    again = rml_context.get_data_model("m")
    assert again is not None
    assert again.is_valid()


def test_remove_data_model(rml_context):
    rml_context.create_data_model("m")
    assert rml_context.remove_data_model("m")
    assert not rml_context.remove_data_model("m")  # already gone


def test_bind_func_drives_template(rml_context):
    dm = rml_context.create_data_model("m")
    state = {"name": "alice"}
    assert dm.bind_func("name", lambda: state["name"])

    doc = load(rml_context,
               "<p id='out' data-model='m'>{{name}}</p>")
    out = doc.get_element_by_id("out")
    assert "alice" in out.get_inner_rml()

    # Change the backing value and mark dirty; the DOM re-evaluates.
    state["name"] = "bob"
    dm.dirty_variable("name")
    rml_context.update()
    assert "bob" in out.get_inner_rml()


def test_bind_func_rebind_fails(rml_context):
    dm = rml_context.create_data_model("m")
    assert dm.bind_func("x", lambda: 1)
    # Binding the same name twice fails.
    assert not dm.bind_func("x", lambda: 2)


def test_bind_list_for_data_for(rml_context):
    dm = rml_context.create_data_model("m")
    items = ["a", "b", "c"]
    assert dm.bind_list("items", lambda: items)

    doc = load(rml_context,
               "<div id='out' data-model='m'>"
               "<span data-for='i : items'>{{i}}</span></div>")
    out = doc.get_element_by_id("out")
    # RmlUi renders one <span> per item (plus a hidden data-for template node),
    # so assert on the rendered content rather than the raw child count.
    rml = out.get_inner_rml()
    assert "<span>a</span>" in rml
    assert "<span>b</span>" in rml
    assert "<span>c</span>" in rml
    assert "<span>d</span>" not in rml

    items.append("d")
    dm.dirty_variable("items")
    rml_context.update()
    assert "<span>d</span>" in out.get_inner_rml()


def test_bind_list_size_is_live(rml_context):
    """`{{ items.size }}` reads the live list length via the getter, even
    without a data-for view on the same variable (regression: it used to read
    a cache that only data-for populated, so a size-only binding was always 0)."""
    dm = rml_context.create_data_model("m")
    items = ["a", "b"]
    assert dm.bind_list("items", lambda: items)

    doc = load(rml_context,
               "<p id='out' data-model='m'>{{ items.size }}</p>")
    out = doc.get_element_by_id("out")
    assert "2" in out.get_inner_rml()

    items.append("c")
    dm.dirty_variable("items")
    rml_context.update()
    assert "3" in out.get_inner_rml()


def test_bind_dict_list_per_field(rml_context):
    dm = rml_context.create_data_model("m")
    rows = [
        {"name": "alice", "score": 100, "visible": True},
        {"name": "bob", "score": 50, "visible": True},
        {"name": "carol", "score": 0, "visible": False},
    ]
    assert dm.bind_dict_list("rows", lambda: rows)

    doc = load(rml_context,
               "<div id='out' data-model='m'>"
               "<p data-for='r : rows' data-if='r.visible'>{{r.name}}={{r.score}}</p>"
               "</div>")
    out = doc.get_element_by_id("out")
    rml = out.get_inner_rml()
    # Per-field access resolves r.name / r.score.
    assert "alice=100" in rml
    assert "bob=50" in rml
    # data-if hides the carol row (RmlUi sets display:none rather than removing it).
    assert "display: none" in rml

    # Mutate the backing list + dirty -> new row appears.
    rows.append({"name": "dave", "score": 75, "visible": True})
    dm.dirty_variable("rows")
    rml_context.update()
    assert "dave=75" in out.get_inner_rml()


def test_bind_dict_list_rejects_non_dict_rows(rml_context):
    dm = rml_context.create_data_model("m")
    # Rows that aren't dicts are logged and produce empty rows, not a crash.
    assert dm.bind_dict_list("rows", lambda: ["not-a-dict", 42])

    doc = load(rml_context,
               "<div id='out' data-model='m'>"
               "<p data-for='r : rows'>{{r.name}}</p></div>")
    # No field values render, but the context did not crash.
    assert doc.get_element_by_id("out") is not None


def test_bind_list_rejects_non_sequence(rml_context):
    dm = rml_context.create_data_model("m")
    assert dm.bind_list("items", lambda: 42)  # bind succeeds

    # When evaluated, a non-list getter yields an empty list (logged warning),
    # not a crash — so no item <span>s are rendered (only the hidden template).
    doc = load(rml_context,
               "<div id='out' data-model='m'>"
               "<span data-for='i : items'>{{i}}</span></div>")
    out = doc.get_element_by_id("out")
    assert "rmlui-inner-rml" in out.get_inner_rml()  # only the template remains
    assert ">4<" not in out.get_inner_rml()           # no item rendered


def test_bind_func_getter_ref_survives_gc(rml_context):
    """The data model holds its own reference to the getter; dropping the
    Python reference and collecting must not break evaluation."""
    dm = rml_context.create_data_model("m")
    state = {"v": "x"}

    def getter():
        return state["v"]

    dm.bind_func("v", getter)
    del getter
    gc.collect()
    gc.collect()

    doc = load(rml_context, "<p id='out' data-model='m'>{{v}}</p>")
    out = doc.get_element_by_id("out")
    assert "x" in out.get_inner_rml()


def test_bind_event_callback(rml_context):
    dm = rml_context.create_data_model("m")

    fired = []
    assert dm.bind_event_callback("ping", lambda: fired.append(1))

    doc = load(rml_context,
               "<button id='b' data-model='m' data-event-click='ping'>go</button>")
    rml_context.update()
    doc.get_element_by_id("b").click()
    rml_context.update()

    assert fired == [1]


def test_dirty_all(rml_context):
    dm = rml_context.create_data_model("m")
    state = {"a": "1", "b": "2"}
    dm.bind_func("a", lambda: state["a"])
    dm.bind_func("b", lambda: state["b"])

    doc = load(rml_context,
               "<div id='out' data-model='m'>{{a}}-{{b}}</div>")
    out = doc.get_element_by_id("out")
    assert "1-2" in out.get_inner_rml()

    state["a"] = "9"
    state["b"] = "8"
    dm.dirty_all()
    rml_context.update()
    assert "9-8" in out.get_inner_rml()
