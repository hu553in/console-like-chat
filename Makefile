ifeq ($(OS), Windows_NT)
	FLAGS = "-DCMAKE_SH=\"CMAKE_SH-NOTFOUND\" -G \"MinGW Makefiles\""
else
	FLAGS = ""
endif

REQ_REP_ENDPOINT=tcp://127.0.0.1:5555
PUB_SUB_ENDPOINT=tcp://127.0.0.1:5556

.PHONY: all
all: client

.PHONY: mkdir_build
mkdir_build:
	[ -d ./cmake-build-debug ] | mkdir -p cmake-build-debug

.PHONY: build
build:
	cd cmake-build-debug;$(MAKE)

.PHONY: exec_server
exec_server:
	./cmake-build-debug/blt-nng-demo server $(REQ_REP_ENDPOINT) $(PUB_SUB_ENDPOINT)

.PHONY: exec_client
exec_client:
	./cmake-build-debug/blt-nng-demo client $(REQ_REP_ENDPOINT) $(PUB_SUB_ENDPOINT)

.PHONY: clean
clean:
	rm -rf cmake-build-debug

.PHONY: reload
reload: mkdir_build
	cd cmake-build-debug;cmake $(FLAGS) ..

.PHONY: run_server
run_server: build exec_server

.PHONY: run_client
run_client: build exec_client

.PHONY: server
server: clean reload build exec_server

.PHONY: client
client: clean reload build exec_client
