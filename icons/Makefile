.PHONY: build rebuild touch icons

build: icons

rebuild: touch icons

touch:
	touch *.svg

icons: \
	../client/android/src/main/res/drawable-ldpi/icon.png \
	../client/android/src/main/res/drawable-mdpi/icon.png \
	../client/android/src/main/res/drawable-hdpi/icon.png \
	../client/android/src/main/res/drawable-xhdpi/icon.png \
	../client/android/src/main/res/drawable-xxhdpi/icon.png \
	../client/android/src/main/res/drawable-xxxhdpi/icon.png \
	../client/android/src/main/res/drawable-ldpi/host.png \
	../client/android/src/main/res/drawable-mdpi/host.png \
	../client/android/src/main/res/drawable-hdpi/host.png \
	../client/android/src/main/res/drawable-xhdpi/host.png \
	../client/android/src/main/res/drawable-xxhdpi/host.png \
	../client/android/src/main/res/drawable-xxxhdpi/host.png \
	../server/win32/lmpc.ico

icon-16.svg: icon.svg
icon-32.svg: icon.svg
icon-36.svg: icon.svg
icon-48.svg: icon.svg
icon-72.svg: icon.svg
icon-96.svg: icon.svg
icon-144.svg: icon.svg
icon-192.svg: icon.svg
icon-256.svg: icon.svg

host-36.svg: host.svg
host-48.svg: host.svg
host-72.svg: host.svg
host-96.svg: host.svg
host-144.svg: host.svg
host-192.svg: host.svg

%.svg:
	@touch $@

%.ico:
	@mkdir -p $(dir $@)
	convert -background transparent -depth 8 $^ $@

%.png:
	@mkdir -p $(dir $@)
	convert -background transparent -depth 8 $^ $@
	pngout -c6 -k0 -q -force $@

../client/android/src/main/res/drawable-ldpi/icon.png: icon-36.svg
../client/android/src/main/res/drawable-mdpi/icon.png: icon-48.svg
../client/android/src/main/res/drawable-hdpi/icon.png: icon-72.svg
../client/android/src/main/res/drawable-xhdpi/icon.png: icon-96.svg
../client/android/src/main/res/drawable-xxhdpi/icon.png: icon-144.svg
../client/android/src/main/res/drawable-xxxhdpi/icon.png: icon-192.svg

../client/android/src/main/res/drawable-ldpi/host.png: host-36.svg
../client/android/src/main/res/drawable-mdpi/host.png: host-48.svg
../client/android/src/main/res/drawable-hdpi/host.png: host-72.svg
../client/android/src/main/res/drawable-xhdpi/host.png: host-96.svg
../client/android/src/main/res/drawable-xxhdpi/host.png: host-144.svg
../client/android/src/main/res/drawable-xxxhdpi/host.png: host-192.svg

../server/win32/lmpc.ico: icon-16.svg icon-32.svg icon-48.svg icon-256.svg
