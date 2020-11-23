BUILDFN=sgm
CFLAGS=-pthread -Wall -O2 -L/usr/X11R6/lib -lX11 -o $(BUILDFN)

clean:
	rm $(BUILDFN)

build:
	gcc main.c $(CFLAGS)