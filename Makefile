OPT = -O2 -g
DEFS =
CFLAGS=$(OPT) $(DEFS)

SLOG_FILES = slogavl slogkbt slog.dat
UUID_FILES = uuidavl uuidkbt uuid.dat

all: $(SLOG_FILES) $(UUID_FILES)

slogavl: slogavl.o tavl.o
	$(CC) -o $@ $^

slogkbt: slogkbt.o
	$(CC) -o $@ $^

slog.dat: slog.dat.gz
	gunzip < slog.dat.gz > slog.dat

uuidavl: uuidavl.c avl.o
	$(CC) -o $@ $^ -luuid

uuidkbt: uuidkbt.o
	$(CC) -o $@ $^ -luuid

uuidgen: uuidgen.o
	$(CC) -o $@ $^ -luuid

uuid.dat: uuidgen
	./uuidgen > uuid.dat
