SRCS="../src/gerber_parse.cpp"
SRCS+=" ../src/util.cpp ../src/fileio.cpp ../src/macro_parser.cpp ../src/macro_vm.cpp"
SRCS+=" ../src/gerb_script_util.cpp ../src/util_type.cpp ../src/gerbobj_line.cpp ../src/gerbobj_poly.cpp"
SRCS+=" ../src/gcode_interp.cpp ../src/groupize.cpp ../src/partitioning.cpp ../src/polymath.cpp ../src/inpoly.cpp"
SRCS+=" ../src/drc.cpp"

# Clean
rm drcbot

# Build
g++ -g -DINT_ASSERT main.cpp $SRCS -o drcbot
