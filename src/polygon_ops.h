#ifndef _POLYGON_OPS_H_
#define _POLYGON_OPS_H_

#include "main.h"
#include "gcode_interp.h"
#include "gerbobj_poly.h"
#include "polygonize.h"
#include <boost/polygon/polygon.hpp>

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;
typedef gtl::polygon_data<int> BoostPolygon;
typedef std::vector<BoostPolygon> BoostPolygonSet;

sp_gerber_object_layer boolean_or(sp_gerber_object_layer layer);

BoostPolygonSet layerToPolygonSet(sp_gerber_object_layer layer);
void polygonSetIntoLayer(BoostPolygonSet polygon_set, sp_gerber_object_layer layer);

namespace boost {
	namespace polygon {
		// point_line
		template <>
		struct geometry_concept<point_line*> { typedef point_concept type; };

		template <>
		struct point_traits<point_line*> {
			typedef int coordinate_type;
			static inline coordinate_type get(const point_line * point, orientation_2d orient) {
				if(orient == HORIZONTAL)
					return (coordinate_type)point->x;
				return (coordinate_type)point->y;
			}
		};

		template <>
		struct point_mutable_traits<point_line*> {
			static inline void set(point_line * point, orientation_2d orient, double value) {
				if(orient == HORIZONTAL)
					point->x = value;
				else
				point->y = value;
			}
			static inline point_line * construct(double x_value, double y_value) {
				point_line *retval;
				retval->x = x_value;
				retval->y = y_value;
				return retval;
			}
		};

		#if 0
		// Polygon
		template <>
		struct geometry_concept<RenderPoly*> { typedef polygon_concept type; };

		template <>
		struct polygon_traits<RenderPoly*> {
			typedef int coordinate_type;
			typedef std::vector<struct point_line*>::const_iterator iterator_type;
			typedef struct point_line* point_type;

			// Get the begin iterator
			static inline iterator_type begin_points(const RenderPoly* t) {
				return t->segs.begin();
			}

			// Get the end iterator
			static inline iterator_type end_points(const RenderPoly* t) {
				return t->segs.end();
			}

			// Get the number of sides of the polygon
			static inline std::size_t size(const RenderPoly* t) {
				return t->segs.size();
			}

			// Get the winding direction of the polygon
			static inline winding_direction winding(RenderPoly* t) {
				return unknown_winding;
			}
		};

		template <>
		struct polygon_mutable_traits<RenderPoly*> {
			//expects stl style iterators
			template <typename iT>
			static inline RenderPoly* set_points(RenderPoly* t, iT input_begin, iT input_end) {
				t->segs.clear();
				/*for(; input_begin != input_end; ++input_begin) {
					point_line* p = dynamic_cast<point_line*>(*input_begin);
					t->segs.push_back(p);
				}*/
				t->segs.insert(t->segs.end(), input_begin.segs.begin(), input_end.segs.end());
				return t;
			}
		};
		#endif
	}
}
#endif
