LIB_OBJECTS=ovc.o jpeg.o
LIB_OUTPUT=img_hengst.a
EXE_FILES=main.cpp
EXE_OUTPUT=hengster
COMPILER_FLAGS = -w -g 
LINKER_FLAGS = -Limg_hengst  `pkg-config --cflags --libs opencv` -lstdc++
GCC=g++

all:	lib exe

lib: $(LIB_OBJECTS)
	ar rcs $(LIB_OUTPUT) $(LIB_OBJECTS)

ovc.o:	ovc.cpp ovc.hpp
	$(CC) -c ovc.cpp  $(COMPILER_FLAGS)

jpeg.o:	jpeg.cpp jpeg.hpp
	$(CC) -c jpeg.cpp  $(COMPILER_FLAGS)

exe:
	$(CC) $(EXE_FILES) $(LIB_OUTPUT) $(COMPILER_FLAGS) -o $(EXE_OUTPUT) $(LINKER_FLAGS)

clean:
	rm *.o $(LIB_OUTPUT) $(EXE_OUTPUT) 