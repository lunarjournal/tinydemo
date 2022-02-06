CC=gcc
CFLAGS=-s -lSDL2 -lm -Qn -Os -ffast-math -nostartfiles -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables -Wl,--gc-sections -Wl,--build-id=none 
DEPS=./subs/file/*.c base.c ./subs/audio/sid/csid.c  boot.s 
LINKER_TAPE=x86_64.ld
OUT=base.bin
default:
	cp TEMP.MOD B 
	gzip B
	$(CC) -T $(LINKER_TAPE) $(DEPS) $(CFLAGS) -o $(OUT)
	./sstrip base.bin
	rm B.gz
	hexedit $(OUT)