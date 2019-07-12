CFLAGS = -O2 -Wall -Werror

all: sort_records sort_recptrs sort_recinfos

sort_records: records.o sort_records.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

sort_recptrs: records.o sort_recptrs.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

sort_recinfos: records.o sort_recinfos.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f sort_records sort_recptrs sort_recinfos *.o

.PHONY: all clean

