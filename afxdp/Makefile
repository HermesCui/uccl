ABS_ROOT_PATH := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
ABS_REPO_ROOT := $(shell realpath $(ABS_ROOT_PATH)/..)

KERNEL = $(shell uname -r)
INC = -I./lib/install/include \
      -I$(ABS_REPO_ROOT)/include \
      -I$(ABS_ROOT_PATH)/lib/libbpf/src
LIBS = ./lib/install/lib/libxdp.a ./lib/install/lib/libbpf.a -lglog -lgflags -lgtest -lz -lelf
LIBS_SHARED = -Wl,-rpath=$(ABS_ROOT_PATH)/./lib/xdp-tools/lib/libxdp/ \
	   		  -Wl,-rpath=$(ABS_ROOT_PATH)/./lib/libbpf/src/ \
	   		  -L./lib/xdp-tools/lib/libxdp -lxdp -L./lib/libbpf/src -lbpf \
	   		  -lglog -lgflags -lgtest -lz -lelf
override CXXFLAGS += -O3 -g -std=c++17 -Wno-pointer-arith -Wno-interference-size -fPIC
CLANGFLAGS = -O3 -g
DEPS = *.h
PLUGIN_SO = libnccl-net.so

lib_src = $(wildcard *.cc)
lib_src := $(filter-out %_main.cc,$(lib_src))
lib_src := $(filter-out %_test.cc,$(lib_src))
lib_src := $(filter-out %_plugin.cc,$(lib_src))
lib_obj = $(lib_src:.cc=.o)

main_src = $(wildcard *_main.cc)
main_src += $(wildcard *_test.cc)
main_bin = $(main_src:.cc=)

ebpf_obj = ebpf_afxdp_test.o ebpf_transport.o ebpf_transport_pktloss.o $(PLUGIN_SO)

.PHONY: build
build: $(main_bin) $(ebpf_obj)

ebpf_afxdp_test.o: ebpf_afxdp_test.c $(DEPS)
	clang $(INC) -target bpf -c ebpf_afxdp_test.c -o ebpf_afxdp_test.o $(CLANGFLAGS)

ebpf_transport.o: ebpf_transport.c $(DEPS)
	clang $(INC) -target bpf -c ebpf_transport.c -o ebpf_transport.o $(CLANGFLAGS)

ebpf_transport_pktloss.o: ebpf_transport_pktloss.c $(DEPS)
	clang $(INC) -target bpf -c ebpf_transport_pktloss.c -o ebpf_transport_pktloss.o $(CLANGFLAGS)

afxdp_daemon_main: afxdp_daemon_main.cc ebpf_transport.o $(DEPS) $(lib_obj)
	g++ afxdp_daemon_main.cc -o afxdp_daemon_main $(lib_obj) $(INC) $(LIBS) $(CXXFLAGS)

transport_test: transport_test.cc ebpf_transport.o $(DEPS) $(lib_obj)
	g++ transport_test.cc -o transport_test $(lib_obj) $(INC) $(LIBS) $(CXXFLAGS)

util_afxdp_test: util_afxdp_test.cc ebpf_afxdp_test.o $(DEPS) $(lib_obj)
	g++ util_afxdp_test.cc -o util_afxdp_test $(lib_obj) $(INC) $(LIBS) $(CXXFLAGS)

rss_hash_test: rss_hash_test.cc $(DEPS) $(lib_obj)
	g++ rss_hash_test.cc -o rss_hash_test $(lib_obj) $(INC) $(LIBS) $(CXXFLAGS)

timely_test: timely_test.cc $(DEPS) $(lib_obj)
	g++ timely_test.cc -o timely_test $(lib_obj) $(INC) $(LIBS) $(CXXFLAGS)

timing_wheel_test: timing_wheel_test.cc $(DEPS) $(lib_obj)
	g++ timing_wheel_test.cc -o timing_wheel_test $(lib_obj) $(INC) $(LIBS) $(CXXFLAGS)

util_lrpc_test: util_lrpc_test.cc $(DEPS) $(lib_obj)
	g++ util_lrpc_test.cc -o util_lrpc_test $(lib_obj) $(INC) $(LIBS) $(CXXFLAGS)

%.o: %.cc $(DEPS)
	g++ -c $< -o $@ $(INC) $(CXXFLAGS)

NCCL_HOME:=../thirdparty/nccl
CUDA_HOME:=/usr/local/cuda
NCCL_INC:= -I$(NCCL_HOME)/build/include -I$(NCCL_HOME)/src/include -I$(CUDA_HOME)/include

$(PLUGIN_SO): nccl_plugin.cc ebpf_transport.o $(DEPS) $(lib_obj)
	g++ $(NCCL_INC) -fPIC -shared -o $@ -Wl,-soname,$(PLUGIN_SO) nccl_plugin.cc $(lib_obj) $(INC) $(LIBS_SHARED) $(CXXFLAGS)

.PHONY: clean
clean:
	rm -f *.o $(main_bin) $(PLUGIN_SO)

lib: lib_config
	echo lib; $(MAKE) -C lib
	echo lib/xdp-tools; $(MAKE) -C lib/xdp-tools

lib_config: configure
	./configure
	cd lib/xdp-tools; ./configure

lib_clean:
	echo lib/xdp-tools; $(MAKE) -C lib/xdp-tools clean
	echo lib; $(MAKE) -C lib clean
	echo common; $(MAKE) -C common clean
