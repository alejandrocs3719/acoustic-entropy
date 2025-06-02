CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -m64

SRC_DIR = src
OBJ = $(SRC_DIR)/main.o \
      $(SRC_DIR)/recolector.o \
      $(SRC_DIR)/shannon.o \
      $(SRC_DIR)/injector.o \
      $(SRC_DIR)/whitening.o

TARGET = tfg_entropy

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm -lcrypto

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)
