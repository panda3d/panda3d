all:
	python makepanda/makepanda.py --everything
install:
	python makepanda/installpanda.py
clean:
	rm -rf built
