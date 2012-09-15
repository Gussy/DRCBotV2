#include <boost/python.hpp>
#include "wrap_fns.h"
#include "polygon_ops.h"

void polygon_wrap(void)
{
	using namespace boost::python;
	
	// polygonize.cpp
	def("polygonizeLayer", polygonize_layer);
	def("polygonizeLayers", polygonize_vector_outp);

	// polygon_ops.cpp
	def("booleanOR", boolean_or);
}

