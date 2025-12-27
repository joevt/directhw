CC = cc
CFLAGS = -arch arm64 -arch x86_64 -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -mmacosx-version-min=10.3 -O2 -Wall

all: AdiumApplescriptRunner

AdiumApplescriptRunner: applescript_runner.m
	$(CC) $(CFLAGS) -framework Foundation -framework OSAKit -o $@ $<

clean:
	rm -f AdiumApplescriptRunner

.PHONY: all clean
