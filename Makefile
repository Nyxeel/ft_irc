NAME		= ircserv
CXX			= c++
CPPFLAGS	= -Wall -Werror -Wextra -MMD -MP -std=c++98
MAKEFLAGS	+= --no-print-directory

SRC_DIR		= src
OBJ_DIR		= objs

SRCS		=	main.cpp \
				Server.cpp \
				Channel.cpp \
				Client.cpp \
				Parser.cpp

SRCS_FULL	= $(addprefix $(SRC_DIR)/, $(SRCS))
OBJS		= $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))
DEPS		= $(OBJS:.o=.d)
HEADERS		= $(wildcard *.hpp *.h)
COMPILE_LOG	= $(OBJ_DIR)/.compile_log

# Farben
OK	= \033[38;5;76m
ERR	= \033[0;38;5;1m
EXE = \033[0;38;5;89m
HDR	= \033[1;36m
RST	= \033[0m

TOP_EXEC	= ┌─[ EXECUTE ]──────────────────────────────────────┐
TOP_ALL     = ┌─[ ALL ]──────────────────────────────────────────┐
TOP_PREPARE = ┌─[ PREPARE ]──────────────────────────────────────┐
TOP_COMPILE = ┌─[ COMPILE ]──────────────────────────────────────┐
TOP_LINK    = ┌─[ LINK ]─────────────────────────────────────────┐
TOP_CLEAN   = ┌─[ CLEAN ]────────────────────────────────────────┐
TOP_FCLEAN  = ┌─[ FCLEAN ]───────────────────────────────────────┐
TOP_ERROR   = ┌─[ ERROR ]────────────────────────────────────────┐
BOT         = └──────────────────────────────────────────────────┘

W = 50

.ONESHELL:
SHELL := /bin/sh
.SILENT:

all:
	# Missing-Source-Check
	missing=false
	for src in $(SRCS_FULL); do
		if [ ! -f "$$src" ]; then
			printf "\n$(ERR)%s$(RST)\n" "$(TOP_ERROR)"
			printf "$(ERR)│ %-*s │$(RST)\n" 48 "Missing source file: $$src"
			printf "$(ERR)%s$(RST)\n\n" "$(BOT)"
			missing=true
		fi
	done
	if [ "$$missing" = true ]; then exit 1; fi

	if $(MAKE) -q $(NAME); then
		printf "\n$(OK)%s$(RST)\n" "$(TOP_ALL)"
		printf "$(OK)│ %-*s │$(RST)\n" $(W) "✔ Nothing to do (already built)"
		printf "$(OK)%s$(RST)\n\n" "$(BOT)"
	else
		rm -f "$(COMPILE_LOG)"

		printf "$(HDR)"
		cat <<'ASCII'

		░██████████░██████████            ░██████░█████████    ░██████
		░██            ░██                  ░██  ░██     ░██  ░██   ░██
		░██            ░██                  ░██  ░██     ░██ ░██
		░█████████     ░██                  ░██  ░█████████  ░██
		░██            ░██                  ░██  ░██   ░██   ░██
		░██            ░██                  ░██  ░██    ░██   ░██   ░██
		░██            ░██    ░██████████ ░██████░██     ░██   ░██████

		ASCII
		printf "$(RST)"

		printf "\n$(HDR)%s$(RST)\n" "$(TOP_PREPARE)"
		if [ -d "$(OBJ_DIR)" ]; then
			printf "$(OK)│ %-*s │$(RST)\n" $(W) "📁 Using: $(OBJ_DIR)/"
		else
			if mkdir -p "$(OBJ_DIR)"; then
				printf "$(OK)│ %-*s │$(RST)\n" $(W) "📁 Created: $(OBJ_DIR)/"
			else
				printf "$(ERR)│ %-*s │$(RST)\n" $(W) "✘ Failed creating: $(OBJ_DIR)/"
				printf "$(HDR)%s$(RST)\n\n" "$(BOT)"
				exit 1
			fi
		fi
		printf "$(HDR)%s$(RST)\n" "$(BOT)"

		$(MAKE) $(NAME)
	fi

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	mkdir -p "$(@D)"
	if $(CXX) $(CPPFLAGS) -c "$<" -o "$@"; then
		printf "$(OK)│ %-*s │$(RST)\n" $(W) "✔ $@" >> "$(COMPILE_LOG)"
	else
		printf "\n$(HDR)%s$(RST)\n" "$(TOP_COMPILE)"
		printf "$(ERR)│ %-*s │$(RST)\n" $(W) "✘ $<"
		printf "$(HDR)%s$(RST)\n\n" "$(BOT)"
		exit 1
	fi

$(NAME): $(OBJS)
	printf "\n$(HDR)%s$(RST)\n" "$(TOP_COMPILE)"
	if [ -f "$(COMPILE_LOG)" ]; then
		cat "$(COMPILE_LOG)"
	else
		printf "$(OK)│ %-*s │$(RST)\n" $(W) "✔ All objects up to date"
	fi
	printf "$(HDR)%s$(RST)\n" "$(BOT)"

	printf "\n$(HDR)%s$(RST)\n" "$(TOP_LINK)"

	link_output=$$( $(CXX) $(OBJS) -o "$(NAME)" 2>&1 )
	link_status=$$?

	if [ $$link_status -eq 0 ]; then
		printf "$(OK)│ %-*s │$(RST)\n" $(W) "✔ Compiled: $(NAME)"
		printf "$(HDR)%s$(RST)\n\n" "$(BOT)"
	else
		printf "$(ERR)│ %-*s │$(RST)\n" $(W) "✘ Link failed: $(NAME)"
		printf "$(ERR)│ %-*s │$(RST)\n" 48 ""

		if echo "$$link_output" | grep -q "undefined reference"; then
			missing=$$(echo "$$link_output" | awk -F'`' '/undefined reference/ {print $$2; exit}')
			missing=$$(printf "%s" "$$missing" | sed "s/'$$//")
			printf "$(ERR)│ %-*s │$(RST)\n" 48 "Missing symbol: $$missing"
		fi

		if echo "$$link_output" | grep -q "ld returned"; then
			printf "$(ERR)│ %-*s │$(RST)\n" 48 "Linker aborted build"
		fi

		printf "$(HDR)%s$(RST)\n\n" "$(BOT)"
		exit 1
	fi

RUN_ARGS := $(filter-out run,$(MAKECMDGOALS))

run: all
	printf "\n$(EXE)%s$(RST)\n" "$(TOP_EXEC)"
	printf "$(OK)│ %-*.*s │$(RST)\n" 48 $(W) "./$(NAME) $(RUN_ARGS)"
	printf "$(EXE)%s$(RST)\n\n" "$(BOT)"
	./$(NAME) $(RUN_ARGS); \
	code=$$?; \
	printf "\n\033[3mMakefile: Program terminated with Exit code: $$code\033[0m\n"; \
	exit 0

%:
	@:

-include $(DEPS)

clean:
	printf "\n$(HDR)%s$(RST)\n" "$(TOP_CLEAN)"
	if ls $(OBJS) >/dev/null 2>&1; then
		rm -f $(OBJS) $(DEPS) "$(COMPILE_LOG)"
		printf "$(OK)│ %-*s │$(RST)\n" $(W) "✔ Removed objects from $(OBJ_DIR)/"
	else
		printf "$(ERR)│ %-*s │$(RST)\n" $(W) "⋗ No object files to clean"
	fi
	printf "$(HDR)%s$(RST)\n\n" "$(BOT)"

fclean:
	printf "\n$(HDR)%s$(RST)\n" "$(TOP_FCLEAN)"
	deleted=false
	if ls $(OBJS) >/dev/null 2>&1; then rm -f $(OBJS) $(DEPS) "$(COMPILE_LOG)"; deleted=true; fi
	if [ -f "$(NAME)" ]; then rm -f "$(NAME)"; deleted=true; fi
	if [ -d "$(OBJ_DIR)" ]; then rm -rf "$(OBJ_DIR)"; deleted=true; fi
	if [ "$$deleted" = true ]; then
		printf "$(OK)│ %-*s │$(RST)\n" $(W) "✔ Fully cleaned (objs + bin + dir)"
	else
		printf "$(ERR)│ %-*s │$(RST)\n" $(W) "⋗ Nothing to clean"
	fi
	printf "$(HDR)%s$(RST)\n\n" "$(BOT)"

re: fclean all

.PHONY: all clean fclean re run
