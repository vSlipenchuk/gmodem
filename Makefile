all: pa-e1550

pa-e1550: pa-e1550.c
		$(CC) pa-e1550.c -lpulse -o pa