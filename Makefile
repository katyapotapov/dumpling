
dumpling: main.c data.c tigr.c graphic.c
	gcc $^ -o dumpling -lGLU -lGL -lX11
