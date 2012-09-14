#include "polygon_ops.h"

using namespace gtl;

sp_gerber_object_layer boolean_or(sp_gerber_object_layer layer)
{
	#if 1
	// Boolean OR
	DBG_MSG_PF("Running boolean OR operation.");
	BoostPolygonSet polygon_set;
	polygon_set |= layerToPolygonSet(layer);
	polygonSetIntoLayer(polygon_set, layer);
	#endif

	#if 0
	// Generics testing
	using namespace gtl;
	typedef std::vector<RenderPoly*> BoostPolygonSet;

	DBG_MSG_PF("draws: %lu", layer->draws.size());

	BoostPolygonSet set, tmpset;
	std::list<sp_GerbObj>::iterator it = layer->draws.begin();
	for(; it != layer->draws.end(); ++it) {
		//DBG_MSG_PF("\tgerbojbs: %lu", (*it)->getPolyData()->segs.size());
		//BoostPolygon poly;
		//gtl::set_points(poly, (*it)->getPolyData()->segs.begin(), (*it)->getPolyData()->segs.end());
		tmpset.clear();
		tmpset.push_back((*it)->getPolyData());
		set |= tmpset;
	}

	// Layers isolated into nets
	DBG_MSG_PF("sets: %lu", set.size());
	#endif

	return layer;
}

BoostPolygonSet layerToPolygonSet(sp_gerber_object_layer layer)
{
	DBG_MSG_PF("draws: %lu", layer->draws.size());

	polygonize_layer(layer);

	// Convert to Boost Polygons
	BoostPolygon polygon;
	BoostPolygonSet polygon_set;
	std::list<sp_GerbObj>::iterator it = layer->draws.begin();
	for(; it != layer->draws.end(); ++it) {
		gtl::set_points(polygon, (*it)->getPolyData()->segs.begin(), (*it)->getPolyData()->segs.end());
		polygon_set.push_back(polygon);
	}
	return polygon_set;
}

void polygonSetIntoLayer(BoostPolygonSet polygon_set, sp_gerber_object_layer layer)
{
	// Layer isolated into nets
	DBG_MSG_PF("sets: %lu", polygon_set.size());

	std::list<sp_GerbObj> output;

	// Convert back to Gerber Objects
	// This is sketchy and slow, ideally the boost::polygon generics should be used to avoid conversions...
	layer->draws.clear();
	std::vector<BoostPolygon>::iterator jt = polygon_set.begin();
	for(; jt != polygon_set.end(); ++jt) {
		GerbObj_Poly * gp = new GerbObj_Poly();
		BoostPolygon::iterator_type kt = (*jt).begin();
		for(; kt != (*jt).end(); ++kt) {
			gp->addPoint(Point((double)gtl::x(*kt), (double)gtl::y(*kt)));
		}
		layer->draws.push_back(sp_GerbObj(gp));
	}
}
