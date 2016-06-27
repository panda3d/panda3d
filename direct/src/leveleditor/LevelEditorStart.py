from . import LevelEditor

if __name__ == '__main__':
    base.le = LevelEditor.LevelEditor()
    # You should define LevelEditor instance as
    # base.le so it can be reached in global scope

    base.run()
