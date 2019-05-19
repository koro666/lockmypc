.PHONY: build rebuild touch icons

build: icons

rebuild: touch icons

touch:
	touch *.svg

icons: \
	../server/win32/lmpc.ico

../server/win32/lmpc.ico: ../server/win32/lmpc-16.png ../server/win32/lmpc-32.png ../server/win32/lmpc-48.png ../server/win32/lmpc-256.png
	convert $^ $@

../server/win32/lmpc-16.png: icon.svg
	convert -background transparent -resize 16x16 -depth 8 $^ $@

../server/win32/lmpc-32.png: icon.svg
	convert -background transparent -resize 32x32 -depth 8 $^ $@

../server/win32/lmpc-48.png: icon.svg
	convert -background transparent -resize 48x48 -depth 8 $^ $@

../server/win32/lmpc-256.png: icon.svg
	convert -background transparent -resize 256x256 -depth 8 $^ $@
