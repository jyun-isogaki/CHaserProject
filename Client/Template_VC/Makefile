# Sample makefile

!include <win32.mak>

all: c201302.exe

clean:
  del *.exe *.obj *.pdb

c201302.exe: c201302.obj
  $(link) $(ldebug) $(conflags) -out:c201302.exe c201302.obj $(conlibs) ws2_32.lib

c201302.obj:
  $(cc) $(cdebug) $(cflags) $(cvars) $*.c
