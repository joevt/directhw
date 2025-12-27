#
# DirectHW - Kernel extension to pass through IO commands to user space
#
# Copyright © 2008-2010 coresystems GmbH <info@coresystems.de>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

all: main libs

# Note: OSTYPE-based version detection removed for consistency and reliability
# Use sw_vers-based detection below for all version comparisons
# Note: Do not append comments to makefile := assignment lines because spaces before the # become part of the assigned string.

# Get macOS version using sw_vers for reliable detection
# sw_vers -productVersion works in 10.3+ but not 10.2 and earlier
# Fallback using perl works in 10.1 and later
DETECTED_VERSION := $(shell sw_vers -productVersion 2>/dev/null | cut -d. -f1-2 || sw_vers 2>/dev/null | perl -ne 'if (/ProductVersion:\s*(\d+(\.\d+)?)/) { print $$1 }' | cut -d. -f1-2)
# Error handling: if version detection failed, abort with clear error message
ifeq ($(DETECTED_VERSION),)
$(error Could not detect macOS version. Please ensure sw_vers is available.)
endif
macos_version := $(DETECTED_VERSION)

# Version comparisons using proper macOS version detection
# Convert to integer format (major*10000 + minor*100 + patch) for proper comparison
is_sequoia_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 150000)

# Detect build machine architecture for universal binary decisions
build_arch := $(shell uname -m)
is_arm64_build := $(if $(filter arm64,$(build_arch)),1,0)
is_sonoma_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 140000)
is_ventura_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 130000)
is_monterey_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 120000)
is_big_sur_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 110000)
is_catalina_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 101500)
is_mojave_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 101400)
is_high_sierra_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 101300)
is_sierra_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 101200)
is_el_capitan_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 101100)
is_yosemite_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 101000)
is_mavericks_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 100900)
is_mountain_lion_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 100800)
is_lion_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 100700)
is_snow_leopard_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 100600)
is_leopard_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 100500)
is_tiger_or_later := $(shell expr $(shell echo "$(macos_version)" | sed 's/\./ /' | awk '{print $$1 * 10000 + $$2 * 100 + ($$3 ? $$3 : 0)}') \>= 100400)

# latestsdk := $(shell xcodebuild -showsdks | sort -r | sed -nE '/.*(-sdk macosx.*)/ { s//\1/;p;q; }')

proj := DirectHW.xcodeproj
build := build/buildlatest
extensions := /System/Library/Extensions
debugsym :=
sdk := -sdk macosx
# Default deployment target for older systems
deploy := MACOSX_DEPLOYMENT_TARGET=10.3
arch :=
# Use proper macOS version detection for build configuration
ifneq ($(is_sequoia_or_later), 0)
	proj := DirectHW.xcodeproj
	build := build/build15
	# Modern Xcode (16.4+) can still build for 10.6+ deployment
	deploy := MACOSX_DEPLOYMENT_TARGET=10.6
ifeq ($(is_arm64_build),1)
	# Apple Silicon build machine - create universal binary
	arch := -arch x86_64 -arch arm64
else
	# Intel build machine - x86_64 only
	arch := -arch x86_64
endif
else ifneq ($(is_sonoma_or_later), 0)
	proj := DirectHW.xcodeproj
	build := build/build14
	sdk := -sdk macosx
	deploy := MACOSX_DEPLOYMENT_TARGET=10.6
ifeq ($(is_arm64_build),1)
	arch := -arch x86_64 -arch arm64
else
	arch := -arch x86_64
endif
else ifneq ($(is_ventura_or_later), 0)
	proj := DirectHW.xcodeproj
	build := build/build13
	sdk := -sdk macosx
	deploy := MACOSX_DEPLOYMENT_TARGET=10.6
ifeq ($(is_arm64_build),1)
	arch := -arch x86_64 -arch arm64
else
	arch := -arch x86_64
endif
else ifneq ($(is_monterey_or_later), 0)
	proj := DirectHW.xcodeproj
	build := build/build12
	sdk := -sdk macosx
	deploy := MACOSX_DEPLOYMENT_TARGET=10.6
ifeq ($(is_arm64_build),1)
	arch := -arch x86_64 -arch arm64
else
	arch := -arch x86_64
endif
else ifneq ($(is_big_sur_or_later), 0)
	proj := DirectHW.xcodeproj
	build := build/build11
	sdk := -sdk macosx
	deploy := MACOSX_DEPLOYMENT_TARGET=10.6
ifeq ($(is_arm64_build),1)
	arch := -arch x86_64 -arch arm64
else
	arch := -arch x86_64
endif
else ifneq ($(is_catalina_or_later), 0)
	proj := DirectHW.xcodeproj
	build := build/build10.15
	sdk := -sdk macosx
	# Catalina (10.15+) dropped 32-bit support completely
	arch := -arch x86_64
else ifneq ($(is_mojave_or_later), 0)
	proj := DirectHW.xcodeproj
	build := build/build10.14
	sdk := -sdk macosx10.14
	arch := -arch i386 -arch x86_64
else ifneq ($(is_high_sierra_or_later), 0)
	proj := DirectHW.xcodeproj
	build := build/build10.13
	sdk := -sdk macosx10.13
	arch := -arch i386 -arch x86_64
else ifneq ($(is_sierra_or_later), 0)
	proj := DirectHW10.6.xcodeproj
	build := build/build10.12
	sdk :=
	deploy :=
else ifneq ($(is_el_capitan_or_later), 0)
	proj := DirectHW10.6.xcodeproj
	build := build/build10.11
	sdk :=
	deploy :=
else ifneq ($(is_yosemite_or_later), 0)
	proj := DirectHW10.6.xcodeproj
	build := build/build10.10
	sdk :=
	deploy :=
else ifneq ($(is_mavericks_or_later), 0)
	proj := DirectHW10.6.xcodeproj
	build := build/build10.9
	sdk :=
	deploy :=
	extensions := /Library/Extensions
else ifneq ($(is_mountain_lion_or_later), 0)
	proj := DirectHW10.6.xcodeproj
	build := build/build10.8
	sdk :=
	deploy :=
else ifneq ($(is_lion_or_later), 0)
	proj := DirectHW10.6.xcodeproj
	build := build/build10.7
	sdk :=
	deploy :=
else ifneq ($(is_snow_leopard_or_later), 0)
	proj := DirectHW10.6.xcodeproj
	build := build/build10.6
	sdk :=
	deploy :=
else ifneq ($(is_leopard_or_later), 0)
	proj := DirectHW10.5.xcodeproj
	build := build/build10.5
	debugsym := -g
	sdk :=
	deploy :=
else ifneq ($(is_tiger_or_later), 0)
	proj := DirectHW10.4.xcodeproj
	build := build/build10.4
	sdk :=
	deploy :=
endif

main:
	@echo "=== Attempting Xcode build ===" && \
	BUILD_LOG="$(build)/xcode_build.log" && \
	mkdir -p $(build) && \
	if xcodebuild -alltargets -project $(proj) $(sdk) $(deploy) $(arch) SYMROOT=$(build) 2> "$$BUILD_LOG"; then \
		echo "✅ Xcode build succeeded"; \
	else \
		echo "❌ Xcode build failed, falling back to Command Line Tools"; \
		if [ -f "$$BUILD_LOG" ] && [ -s "$$BUILD_LOG" ]; then \
			echo "Xcode build errors:"; \
			cat "$$BUILD_LOG"; \
		fi; \
		$(MAKE) libs-fallback; \
	fi

libs: DirectHW.c DirectHW.h
	mkdir -p $(build)/Release
ifneq ($(is_big_sur_or_later)$(is_monterey_or_later)$(is_ventura_or_later)$(is_sonoma_or_later)$(is_sequoia_or_later),0)
ifeq ($(is_arm64_build),1)
	# Apple Silicon build machine - create universal binary
	$(CC) -arch x86_64 -arch arm64 DirectHW.c -dynamiclib -framework IOKit -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -o $(build)/Release/libDirectHW.dylib $(debugsym)
else
	# Intel build machine - x86_64 only for Apple Silicon era macOS
	$(CC) -arch x86_64 DirectHW.c -dynamiclib -framework IOKit -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -o $(build)/Release/libDirectHW.dylib $(debugsym)
endif
else ifneq ($(is_catalina_or_later),0)
	# Catalina (10.15+) - 64-bit only
	$(CC) -arch x86_64 DirectHW.c -dynamiclib -framework IOKit -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -o $(build)/Release/libDirectHW.dylib $(debugsym)
else ifneq ($(is_mojave_or_later)$(is_high_sierra_or_later),0)
	# Mojave (10.14) and High Sierra (10.13) - last versions with 32-bit support
	$(CC) -arch i386 -arch x86_64 DirectHW.c -dynamiclib -framework IOKit -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -o $(build)/Release/libDirectHW.dylib $(debugsym)
else
	# Older systems - 64-bit only (no ARM64 support)
	$(CC) -arch x86_64 DirectHW.c -dynamiclib -framework IOKit -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -o $(build)/Release/libDirectHW.dylib $(debugsym)
endif
	#$(CC) -static -c DirectHW.c -o $(build)/Release/libDirectHW.a

libs-fallback: DirectHW.c DirectHW.h
	@echo "=== Building with Command Line Tools (fallback) ==="
	mkdir -p $(build)/Release
ifneq ($(is_big_sur_or_later)$(is_monterey_or_later)$(is_ventura_or_later)$(is_sonoma_or_later)$(is_sequoia_or_later),0)
ifeq ($(is_arm64_build),1)
	# Apple Silicon build machine - create universal binary
	$(CC) -arch x86_64 -arch arm64 DirectHW.c -dynamiclib -framework IOKit -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -o $(build)/Release/libDirectHW.dylib $(debugsym)
else
	# Intel build machine - x86_64 only for Apple Silicon era macOS
	$(CC) -arch x86_64 DirectHW.c -dynamiclib -framework IOKit -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -o $(build)/Release/libDirectHW.dylib $(debugsym)
endif
else ifneq ($(is_catalina_or_later),0)
	# Catalina (10.15+) - 64-bit only
	$(CC) -arch x86_64 DirectHW.c -dynamiclib -framework IOKit -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -o $(build)/Release/libDirectHW.dylib $(debugsym)
else ifneq ($(is_mojave_or_later)$(is_high_sierra_or_later),0)
	# Mojave (10.14) and High Sierra (10.13) - last versions with 32-bit support
	$(CC) -arch i386 -arch x86_64 DirectHW.c -dynamiclib -framework IOKit -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -o $(build)/Release/libDirectHW.dylib $(debugsym)
else
	# Older systems - 64-bit only (no ARM64 support)
	$(CC) -arch x86_64 DirectHW.c -dynamiclib -framework IOKit -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -o $(build)/Release/libDirectHW.dylib $(debugsym)
endif
	#$(CC) -static -c DirectHW.c -o $(build)/Release/libDirectHW.a

install:
	sudo mkdir -p /usr/local/lib
	[[ -e $(extensions)/DirectHW.kext ]] && sudo rm -R $(extensions)/DirectHW.kext || :
	sudo cp -p -R $(build)/Release/DirectHW.kext $(extensions)/
	[[ -e /Library/Frameworks/DirectHW.framework ]] && sudo rm -R /Library/Frameworks/DirectHW.framework || :
	sudo cp -p -R $(build)/Release/DirectHW.framework /Library/Frameworks/
	sudo cp -p $(build)/Release/libDirectHW.a /usr/local/lib/
	sudo cp -p $(build)/Release/libDirectHW.dylib /usr/local/lib/
	sudo chmod -R 755 $(extensions)/DirectHW.kext
	sudo chmod -R 755 /Library/Frameworks/DirectHW.framework
	sudo chmod 644 /usr/local/lib/libDirectHW.a
	sudo chmod 644 /usr/local/lib/libDirectHW.dylib
	sudo chown -R root:wheel $(extensions)/DirectHW.kext
	sudo chown -R root:wheel /Library/Frameworks/DirectHW.framework
	sudo kextunload -v $(extensions)/DirectHW.kext || :
	sudo kextload -v $(extensions)/DirectHW.kext
	sudo touch $(extensions) || :
	# Use kmutil for macOS 10.13+ (High Sierra and later), kextcache for older versions
	if [ "$(is_high_sierra_or_later)" != "0" ]; then \
		sudo kmutil install --volume-root / --check-rebuild || : ; \
	else \
		sudo kextcache -system-prelinked-kernel || sudo kextcache -system-cache || : ; \
	fi

clean:
	rm -rf $(build)
