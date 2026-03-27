PROGRAMS = ll ht

all: $(PROGRAMS)

%: %.c
	$(CC) -o $@ $<

clean:
	rm -f $(PROGRAMS)
