CC = g++

# Windows (cygwin)
#CFLAGS = -D_WIN32 -O3 -Wall -g -mno-cygwin -DGLEW_STATIC -I/mingw/include
#WINLIBS = -lglew32 -lopengl32 -lgdi32 -lglu32 -lwinmm
#LIBS = -L/mingw/lib -L/lib/w32api  -lpng $(WINLIBS)
#LDFLAGS = -mno-cygwin

# Linux
CFLAGS = -O2 -Wall -g -I../glew/include -I./include
XLIBS = -lGL -lGLU -lX11 -lXxf86vm -lGLEW -L../glew/lib
LIBS = -L/usr/X11R6/lib  $(XLIBS) 

TARGET = lib/libGPGPU.a
OBJS = src/Engine.o src/Exception.o src/GLEngine.o src/GLTexture.o src/CgProgram.o src/PBuffer.o src/GLVertexStream.o src/GLSLProgram.o
UTIL_OBJS = src/TexturePool.o src/Util.o

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS) $(UTIL_OBJS)
	ar  rv $@ $^

clean:
	rm -f $(TARGET) src/*.o
