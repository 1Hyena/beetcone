NAME    = beetcone
CC      = gcc
PROF    = -O3
C_FLAGS = -Wall -Werror -Wextra -pedantic-errors -Wconversion
C_FLAGS+= -Wno-unused-parameter -fmax-errors=5 -std=gnu23
L_FLAGS = -lm -lcrypto
SRC_DIR = src
OBJ_DIR = obj
DEFINES =

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
O_FILES   := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

OUT = ./$(NAME)

all:
	@$(MAKE) make_dynamic -s

debug:
	@$(MAKE) make_debug -s

make_dynamic: $(O_FILES)
	@printf "\033[1;33mMaking \033[37m   ...."
	$(CC) -o $(OUT) $(O_FILES) $(L_FLAGS)
	@printf "\033[1;32m Optimized %s done!\033[0m\n" $(NAME)

make_debug: PROF = -O0 -g -rdynamic
make_debug: DEFINES += -DARION_DEBUG
make_debug: $(O_FILES)
	@printf "\033[1;33mMaking \033[37m   ...."
	$(CC) -o $(OUT) $(O_FILES) $(L_FLAGS)
	@printf "\033[1;32m Debug %s done!\033[0m\n" $(NAME)

PRINT_FMT1 = "\033[1m\033[31mCompiling \033[37m....\033[34m %-48s"
PRINT_FMT2 = "    \033[33m%6s\033[31m lines\033[0m \n"
PRINT_FMT  = $(PRINT_FMT1)$(PRINT_FMT2)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@printf $(PRINT_FMT) $*.c "`wc -l $(SRC_DIR)/$*.c | cut -f1 -d' '`"
	@$(CC) $< $(C_FLAGS) $(PROF) $(DEFINES) -c -o $@

clean:
	@printf "\033[1;36mCleaning \033[37m ...."
	@rm -f $(O_FILES) $(OUT)
	@printf "\033[1;37m Binaries of $(NAME) cleaned!\033[0m\n"
