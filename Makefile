SRCS=src/main.c
EXEC=bin/output
INCL=src/include

FLAG=-Wall -O3
LIBS=-lSDL2 -lSDL2_image -lSDL2_ttf

all: clean run

build: clean $(EXEC)

run: $(EXEC)
	$(EXEC)

$(EXEC): $(SRCS)
	gcc $< -o $@ -I$(INCL) $(FLAG) $(LIBS)

clean:
	rm -f $(EXEC)

