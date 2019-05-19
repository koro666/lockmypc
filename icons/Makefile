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

%.ico:
	@mkdir -p $(dir $@)
	convert -background transparent -depth 8 $^ $@

%.png:
	@mkdir -p $(dir $@)
	convert -background transparent -depth 8 $^ $@
	pngout -c6 -k0 -q -force $@

../server/win32/lmpc.ico: icon-16.svg icon-32.svg icon-48.svg icon-256.svg
