# Compilador
CC = gcc

# Diretórios
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Flags de compilação
CFLAGS = -Wall -Wextra -I$(INC_DIR)

# Nome do executável
TARGET = M.A.G

# Arquivos fonte
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Bibliotecas necessárias
LIBS = -lncurses

# Criar diretório de objetos
$(shell mkdir -p $(OBJ_DIR))

# Regra principal
all: $(TARGET)

# Compilação do programa
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

# Regra para arquivos objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpar arquivos compilados
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Executar o programa
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run