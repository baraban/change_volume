
LDFLAGS=-lasound -lm
SRC=change_volume.c
EXE=change_volume

all:
	$(CC) -o $(EXE) $(SRC) $(LDFLAGS)

clean:
	rm $(EXE)
