Direct Hardware driver and library for Mac OS X 10.4 to macOS 13.

Consists of 2 parts:
- The user space library
- The kernel space driver

This repository merely added a github workflow to automatically compile binaries for this new (13 compatible) fork.

Installing binaries from artifact (these are modified from the DirectHW/Makefile):
	sudo mkdir -p /usr/local/lib
	sudo cp -R Artifacts/DirectHW.kext <path-to-S/L/E, L/E, or EFI/OC/Kexts - see Opencore Docs for kext locations)
	sudo cp -R Artifacts/DirectHW.framework /Library/Frameworks/
	sudo cp Artifacts/libDirectHW.a /usr/local/lib/
	sudo cp Artifacts/libDirectHW.dylib /usr/local/lib/
 
	sudo chmod -R 755 <path-to-extensions>/DirectHW.kext
	sudo chmod -R 755 /Library/Frameworks/DirectHW.framework
	sudo chmod 644 /usr/local/lib/libDirectHW.a
	sudo chmod 644 /usr/local/lib/libDirectHW.dylib
	sudo chown -R root:wheel <path-to-extensions>/DirectHW.kext
	sudo chown -R root:wheel /Library/Frameworks/DirectHW.framework
	# sudo kextunload -v $(extensions)/DirectHW.kext || :
	# sudo kextload -v $(extensions)/DirectHW.kext
	# sudo touch $(extensions) || :
	# command -v kmutil > /dev/null && sudo kmutil install --volume-root / --check-rebuild || :
