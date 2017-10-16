GCC=g++
#LINK2=-lGLEW -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi
LINK=-lGLEW -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lXinerama -lXcursor
LINKSOIL=-lSOIL -limage_helper -limage_DXT -lstb_image_aug
FREETYPE_LINK=-lfreetype
FREETYPE_PATH=-I/usr/include/freetype2/

# this: `-ldl -lXinerama -lXcursor`
# might also need to be added, if getting undefined references to 'dlclose'

FLAGS=-std=c++11

main : main.cpp
	$(GCC) $(FLAGS) $< -o $@ $(LINK) $(LINKSOIL) $(FREETYPE_PATH) $(FREETYPE_LINK)

test : clean main
	./main

clean:
	rm -rf *.o main
