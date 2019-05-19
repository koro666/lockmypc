.PHONY: build rebuild touch icons

build: icons

rebuild: touch icons

touch:
	touch *.svg

icons: \
	../server/win32/lmpc.ico

icon-16.svg: icon.svg
icon-32.svg: icon.svg
icon-48.svg: icon.svg
icon-256.svg: icon.svg

../server/win32/lmpc.ico: icon-16.svg icon-32.svg icon-48.svg icon-256.svg
	convert -background transparent -depth 8 $^ $@
