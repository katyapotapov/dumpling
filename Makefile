
dumpling: main.c data.c tigr.c
	gcc $^ -o dumpling -lGLU -lGL -lX11
