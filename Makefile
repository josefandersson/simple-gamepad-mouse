BUILDFN=sgm
CFLAGS=-pthread -Wall -O2 -o $(BUILDFN)

clean:
	rm $(BUILDFN)

build:
	gcc main.c $(CFLAGS)