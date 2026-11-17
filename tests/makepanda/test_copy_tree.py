from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "makepanda"))
import makepandacore as core


def test_windows_copy_tree_handles_deep_paths_and_empty_directories(tmp_path, monkeypatch):
    monkeypatch.setattr(core, "GetHost", lambda: "windows")
    source = tmp_path / "SDK with spaces"
    source.mkdir()
    (source / "empty").mkdir()
    (source / "binary").write_bytes(bytes(range(256)))
    destination = tmp_path
    while len(str(destination)) < 300:
        destination /= "nested-" * 10
    destination /= "python"
    destination.parent.mkdir(parents=True)
    assert len(str(destination)) > 260
    core.CopyTree(str(destination), str(source))
    assert (destination / "empty").is_dir()
    assert (destination / "binary").read_bytes() == bytes(range(256))


def test_windows_copy_tree_still_omits_vcs_and_excluded_paths(tmp_path, monkeypatch):
    monkeypatch.setattr(core, "GetHost", lambda: "windows")
    source = tmp_path / "source"
    (source / ".git").mkdir(parents=True)
    (source / ".git" / "config").write_text("vcs")
    (source / "excluded").write_text("omit")
    (source / "keep").write_text("preserve")
    destination = tmp_path / "output"
    core.CopyTree(str(destination), str(source), exclude=("excluded",))
    assert sorted(path.name for path in destination.iterdir()) == ["keep"]
    assert (destination / "keep").read_text() == "preserve"
