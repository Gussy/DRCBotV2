/*
 *  Portions Copyright 2006,2009 David Carne and 2007,2008 Spark Fun Electronics
 *
 *
 *  This file is part of gerberDRC.
 *
 *  gerberDRC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  gerberDRC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "polygonize.h"
#define maxsteps 64
 
sp_GerbObj createPolyForLine(sp_GerbObj_Line l)
{
	double dx = l->sx - l->ex;
	double dy = l->sy - l->ey;
	double angle;

	// Angle  = angle of line
	angle = atan2(dy, dx);
	// rotate 90
	angle += M_PI/2;

	double radius = l->width / 2;
		
	//RenderPoly * obj = new RenderPoly();

	int nsteps = l->width * 1000;
	if (nsteps > maxsteps)
		nsteps = maxsteps;
	if (nsteps < 2)
		nsteps = 2;

	// No polygon data for something with 0 width
	if (l->width == 0)
		return sp_GerbObj();

	GerbObj_Poly * p = new GerbObj_Poly();

	float theta = -M_PI/2;
	float thetastep = M_PI / nsteps;
	for (int i=0; i<=nsteps; i++)
	{
		double pdx = cos(angle + 3.0 * M_PI / 2.0+theta) * radius;
		double pdy = sin(angle + 3.0 * M_PI / 2.0+theta) * radius;
		p->addPoint(Point(l->sx + pdx, l->sy + pdy));
		theta += thetastep;
	}
	theta = -M_PI/2;
	for (int i=0; i<=nsteps; i++)
	{
		double pdx = cos(angle + 3.0 * M_PI / 2.0+theta) * radius;
		double pdy = sin(angle + 3.0 * M_PI / 2.0+theta) * radius;
		p->addPoint(Point(l->ex - pdx, l->ey - pdy));
		theta += thetastep;

	}

	return sp_GerbObj(p);
}

void polygonize_vector_outp(Vector_Outp * v)
{
	#if 0
	std::set<GerbObj*>::iterator i = v->all.begin();
	int c = 0;
	for (;i!=v->all.end();)
	{
		GerbObj_Line * line = dynamic_cast<GerbObj_Line*>(*i);
		if (line)
		{	
			// First, remove the line from the map
			v->all.erase(i++);

			// Create a polygon for the line [no end caps for now]
			GerbObj_Poly * p = createPolyForLine(line);
			if (p)
				v->all.insert(p);
			delete line;

			c++;
		} else {
			++i;
		}
	}
	printf("Removed %d lines from map\n", c);
	#endif
}

void polygonize_layer(sp_gerber_object_layer layer)
{
	int count = 0;
	std::list<sp_GerbObj>::iterator it = layer->draws.begin();
	for(; it != layer->draws.end();) {
		sp_GerbObj_Line line = boost::dynamic_pointer_cast<GerbObj_Line>(*it);
		if(line) {
			// First, remove the line from the map
			layer->draws.erase(it++);

			// Create a polygon for the line [no end caps for now]
			sp_GerbObj p = createPolyForLine(line);
			if (p)
				layer->draws.push_back(p);
			line.reset();

			count++;
		} else {
			++it;
		}
	}

	DBG_MSG_PF("Removed %d lines from map\n", count);
}
