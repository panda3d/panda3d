all:
	python makepanda/makepanda.py --everything
installer:
	python makepanda/makepanda.py --everything --installer
install:
	python makepanda/installpanda.py
clean:
	rm -rf built

.PHONY: all install clean
