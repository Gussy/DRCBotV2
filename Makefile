SRCS=src/gerber_parse.cpp src/wrap/gerber_parse_wrap.cpp src/wrap/aperture_wrap.cpp \
	src/util.cpp src/fileio.cpp src/macro_parser.cpp src/macro_vm.cpp \
	src/gerb_script_util.cpp src/util_type.cpp src/gerbobj_line.cpp src/gerbobj_poly.cpp \
	src/gcode_interp.cpp src/wrap/gerber_utils_wrap.cpp src/wrap/gcode_interp_wrap.cpp \
	src/polygon_ops.cpp src/wrap/polygon_wrap.cpp src/polygonize.cpp
	
OBJS=$(patsubst %.cpp,build/%.o, $(SRCS) )

OS = $(shell uname)

CPPFLAGS = -fPIC `python-config --includes` -g -Isrc/ -Wall -Werror -Wno-unused -O2

ifeq ($(OS),Darwin) 
	CPPFLAGS += -Wnewline-eof
endif

LDFLAGS = -lboost_python `python-config --ldflags`

_gerber_utils.so: $(OBJS)
	@echo "LD   $@"
	g++ $^ --shared -o $@ $(LDFLAGS)
	
build/%.o: %.cpp
	@echo "CXX	$<"
	@mkdir -p $(@D)
	@g++ $(CPPFLAGS) -c -o $@ $<
	
.PHONY: clean
clean:
	rm -rf build _gerber_utils.so
