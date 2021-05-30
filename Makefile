
DEPS = main.c data.c tigr.c graphic.c util.c cute_sound.h tigr.h
CSRCS = $(filter %.c,$(DEPS))

dumpling: $(DEPS)
	gcc $(CSRCS) -o dumpling -lGLU -lGL -lX11 -lpthread -ldl -lSDL2 -g
