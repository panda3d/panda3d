from direct.leveleditor import LevelEditor

if __name__ == '__main__':
    from direct.showbase.ShowBase import ShowBase
    base = ShowBase()

    base.le = LevelEditor.LevelEditor()  # type: ignore[attr-defined]
    # You should define LevelEditor instance as
    # base.le so it can be reached in global scope

    base.run()
