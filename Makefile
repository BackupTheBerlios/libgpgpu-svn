CC = g++

# Windows (cygwin)
#CXXFLAGS = -D_WIN32 -O3 -Wall -g -mno-cygwin -DGLEW_STATIC -I/mingw/include
#WINLIBS = -lglew32 -lopengl32 -lgdi32 -lglu32 -lwinmm
#LIBS = -L/mingw/lib -L/lib/w32api  -lpng $(WINLIBS)
#LDFLAGS = -mno-cygwin

# Linux
CXXFLAGS = -O2 -Wall -g -I../glew/include -IGPGPU/include
XLIBS = -lGL -lGLU -lX11 -lCg -lCgGL -lXxf86vm -lpthread -lGPGPU -lGLEW -Lglew/lib
LIBS = -L/usr/X11R6/lib -lpng  $(XLIBS) -lz -LGPGPU/lib

TARGETS = tst tst2 tst3 dowdgpu GLSLtst particle

all: $(TARGETS)

tst: tst.o GenUtil.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)
tst2: tst2.o GenUtil.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)
tst3: tst3.o GenUtil.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)
GLSLtst: GLSLtst.o GenUtil.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)

bitonicgpu: bitonicgpu.o GenUtil.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)
bitonicgpu2: bitonicgpu2.o GenUtil.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)

dowdgpu: dowdgpu.o GenUtil.o DowdSortingNetwork.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)
dowdgpuati: dowdgpuati.o GenUtil.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)
    
bitonic: bitonic.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)
bitonic2: bitonic2.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)
bitonic3: bitonic3.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)
bitonic4: bitonic4.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)
bitonic5: bitonic5.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)
bitonic6: bitonic6.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)
bitonicgen: bitonicgen.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)

particle: particle.o GenUtil.o DowdSortingNetwork.o
	$(CC) $(LDFLAGS)   -o $@ $^ $(LIBS)


clean:
	rm -f $(TARGETS) *.o

distclean: clean
	find -name \*~ -exec rm {} \;
	(cd glew; make clean)
	(cd GPGPU; make clean)

deps:
	(cd glew; make lib/libGLEW.a )
	(cd GPGPU; make)
