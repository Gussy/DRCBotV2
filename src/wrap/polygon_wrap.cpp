#include <boost/python.hpp>
#include "wrap_fns.h"
#include "polygon_ops.h"

void polygon_wrap(void)
{
	using namespace boost::python;
	
	def("booleanOR", boolean_or);
}

