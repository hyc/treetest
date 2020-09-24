OPT = -O2 -g
CFLAGS=$(OPT)

all: slogavl slogkbt slog.dat

slogavl: slogavl.o tavl.o
	$(CC) -o $@ $^

slogkbt: slogkbt.o
	$(CC) -o $@ $^

slog.dat: slog.dat.gz
	gunzip < slog.dat.gz > slog.dat
