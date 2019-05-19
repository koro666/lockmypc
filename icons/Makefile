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

../server/win32/lmpc.ico: ../server/win32/lmpc-16.png ../server/win32/lmpc-32.png ../server/win32/lmpc-48.png ../server/win32/lmpc-256.png
	convert $^ $@

../server/win32/lmpc-16.png: icon-16.svg
	convert -background transparent -depth 8 $^ $@

../server/win32/lmpc-32.png: icon-32.svg
	convert -background transparent -depth 8 $^ $@

../server/win32/lmpc-48.png: icon-48.svg
	convert -background transparent -depth 8 $^ $@

../server/win32/lmpc-256.png: icon-256.svg
	convert -background transparent -depth 8 $^ $@
