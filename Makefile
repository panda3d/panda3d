all:
	python makepanda/makepanda.py --everything --target all
panda3d:
	python makepanda/makepanda.py --everything --target panda3d
plugins:
	python makepanda/makepanda.py --everything --target plugins
installer:
	python makepanda/makepanda.py --everything --target installer
install:
	python makepanda/installpanda.py
clean:
	rm -rf built

.PHONY: all install clean
