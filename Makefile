CC = clang
CFLAGS = -std=c11 -Wall -Wextra -Werror -O2
CPPFLAGS = -Iinclude           # 關鍵：告訴編譯器去哪找 .h

SRC = src/main.c \
      src/file_picker.c src/map.c src/motion.c src/algo.c src/render.c src/scan.c src/edit_map.c src/random_map.c
OBJ = $(SRC:.c=.o)
BIN = shortest_path

all: $(BIN)
$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(OBJ)

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean run rebuild
run: $(BIN)
	./$(BIN) $(ARGS)
rebuild: clean all
