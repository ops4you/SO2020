_CC=gcc
_LINTER=clang-tidy
_MEMCHECKER=valgrind

_STD=-std=c11
_FLTO=-flto=full
_WARN_FLAGS=-Wall -Wextra -Wdouble-promotion -Werror=pedantic -Werror=vla -pedantic-errors -Wfatal-errors
_DEBUG_FLAGS=-O0 -g
_RELEASE_FLAGS=-O2 -march=native -mtune=native

_SERVER_NAME=argus_server
_CLIENT_NAME=argus_client

_INCLUDE_DIR=include
_SRC_DIR=src
_TARGET_DIR=target
_DEBUG_DIR=$(_TARGET_DIR)/debug
_RELEASE_DIR=$(_TARGET_DIR)/release

_SERVER_SOURCES=$(shell find $(_SRC_DIR) -type f -name '*.c' ! -name client.c)
_CLIENT_SOURCES=$(shell find $(_SRC_DIR) -type f -name '*.c' ! -name server.c)
_HEADERS=$(shell find $(_INCLUDE_DIR) -name '*.h')

_SERVER_DEBUG_OBJS=$(patsubst $(_SRC_DIR)/%.c, $(_DEBUG_DIR)/%.o, $(_SERVER_SOURCES))
_SERVER_RELEASE_OBJS=$(patsubst $(_SRC_DIR)/%.c, $(_RELEASE_DIR)/%.o, $(_SERVER_SOURCES))

_CLIENT_DEBUG_OBJS=$(patsubst $(_SRC_DIR)/%.c, $(_DEBUG_DIR)/%.o, $(_CLIENT_SOURCES))
_CLIENT_RELEASE_OBJS=$(patsubst $(_SRC_DIR)/%.c, $(_RELEASE_DIR)/%.o, $(_CLIENT_SOURCES))

server: server_debug

client: client_debug

server_debug: _mkdir_debug $(_DEBUG_DIR)/$(_SERVER_NAME)

server_release: _mkdir_release $(_RELEASE_DIR)/$(_SERVER_NAME)

client_debug: _mkdir_debug $(_DEBUG_DIR)/$(_CLIENT_NAME)

client_release: _mkdir_release $(_RELEASE_DIR)/$(_CLIENT_NAME)

docs: $(_HEADERS)
	doxygen Doxyfile

memcheck_server: debug
	$(_MEMCHECKER) \
        --tool=memcheck \
        --leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
        --error-exitcode=1 \
        $(_DEBUG_DIR)/$(_SERVER_NAME)

memcheck_client: debug
	$(_MEMCHECKER) \
        --tool=memcheck \
        --leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
        --error-exitcode=1 \
        $(_DEBUG_DIR)/$(_CLIENT_NAME)

lint:
	$(_LINTER) \
        $(_SOURCES) \
        -checks="*",clang-analyzer"*",clang-analyzer-c"*" \
		-extra-arg=-fcolor-diagnostics \
        -- $(_STD) $(_WARN_FLAGS) -I$(_INCLUDE_DIR)

clean:
	rm -rf $(_TARGET_DIR)

_mkdir_debug:
	@mkdir -p $(_DEBUG_DIR)

_mkdir_release:
	@mkdir -p $(_RELEASE_DIR)

$(_DEBUG_DIR)/$(_SERVER_NAME): $(_SERVER_DEBUG_OBJS)
	$(_CC) $^ $(_STD) $(_WARN_FLAGS) $(_DEBUG_FLAGS) $(_FLTO) -o $@

$(_SERVER_DEBUG_OBJS): $(_DEBUG_DIR)/%.o : $(_SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(_CC) -c $(_STD) $(_WARN_FLAGS) $(_DEBUG_FLAGS) $(_FLTO) -I$(_INCLUDE_DIR) $< -o $@

$(_RELEASE_DIR)/$(_SERVER_NAME): $(_SERVER_RELEASE_OBJS)
	$(_CC) $^ $(_STD) $(_WARN_FLAGS) $(_RELEASE_FLAGS) $(_FLTO) -I$(_INCLUDE_DIR) -o $@

$(_SERVER_RELEASE_OBJS): $(_RELEASE_DIR)/%.o : $(_SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(_CC) -c $(_STD) $(_WARN_FLAGS) $(_RELEASE_FLAGS) $(_FLTO) -I$(_INCLUDE_DIR) $< -o $@


$(_DEBUG_DIR)/$(_CLIENT_NAME): $(_CLIENT_DEBUG_OBJS)
	$(_CC) $^ $(_STD) $(_WARN_FLAGS) $(_DEBUG_FLAGS) $(_FLTO) -o $@

$(_CLIENT_DEBUG_OBJS): $(_DEBUG_DIR)/%.o : $(_SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(_CC) -c $(_STD) $(_WARN_FLAGS) $(_DEBUG_FLAGS) $(_FLTO) -I$(_INCLUDE_DIR) $< -o $@

$(_RELEASE_DIR)/$(_CLIENT_NAME): $(_CLIENT_RELEASE_OBJS)
	$(_CC) $^ $(_STD) $(_WARN_FLAGS) $(_RELEASE_FLAGS) $(_FLTO) -I$(_INCLUDE_DIR) -o $@

$(_CLIENT_RELEASE_OBJS): $(_RELEASE_DIR)/%.o : $(_SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(_CC) -c $(_STD) $(_WARN_FLAGS) $(_RELEASE_FLAGS) $(_FLTO) -I$(_INCLUDE_DIR) $< -o $@
