#!/usr/bin/python
import os
from optparse import OptionParser

import cairo._cairo as cairo

import gerberDRC as GD
import gerberDRC.util as GU
import math

import plot_modes

usage = "usage: %prog [options] input_dir output_file"
o = OptionParser(usage=usage)

o.add_option("-d", "--debug-level", 
	type="int", dest="debuglevel", default=1,
	help="Debug Level; 0-4, 4 being most verbose")

o.add_option("-m", "--render-mode", 
	type="str", dest="rendermode", default="REALISTIC_TOP",
	help="one of %s" % ", ".join(plot_modes.modes))

o.add_option("--bounds-artwork", 
    dest="bounds_artwork", default=False, action="store_true",
	help="calculate bounds from all artwork, using all segments and layers unless -0 or --visible-only specified.")

o.add_option("-0", "--hairlines-only", 
    dest="size0", default=False, action="store_true",
	help="use only zero width lines for calculating bounding rect\n"
		"Only applies in conjunction with --bounds-artwork")

o.add_option("--visible-only", 
    dest="bounding_from_visible_only", default=False, action="store_true",
	help="use only visible layers for calculating bounding rect. Only applies in conjunction with --bounds-artwork")
	
o.add_option("--fec", "--fallback-exc-stripped", 
    dest="fec", default="LEADING",
	help="Fallback incase the stripping of leading/trailing zeros could not be determined. Default is LEADING zeros stripped")

o.add_option("--son0", "--search-outline-nonzero", 
    dest="search_outline_nonzero", default=False, action="store_true", help="Also look at non-zero lines when determining board outline [WARNING, SLOW]")

o.add_option("-t", "--type", type="str", dest="convention", default="protel", help="Type of input files.")
(options, args) = o.parse_args()

if (len(args) != 2):
	o.error("incorrect number of arguments")
	exit(1)

if not options.rendermode in plot_modes.modes:
	o.error("MODE must be one of %s" % ", ".join(plot_modes.modes))
	exit(1)
	
path, outputfilename = args
mode = options.rendermode
convention = options.convention

GD.setDebugLevel(GD.debug_level_t(options.debuglevel))

plotmode = plot_modes.getPlotSettings(options.rendermode)


def createCairoLineCenterLinePath(obj, cr):
	cr.move_to (obj.sx, obj.sy);
	cr.line_to (obj.ex, obj.ey);
		
def renderGerberFile(rep, cr, layer, outlines):
	ps = plotmode.getPlotSettings(layer)
	
	cr.push_group()
	if not ps.drawinverted:
		cr.set_operator(cairo.OPERATOR_OVER)
	else:
		cr.set_operator(cairo.OPERATOR_OVER)
		cr.set_source_rgba(ps.ovr, ps.ovg, ps.ovb, 1)
		
		if outlines:
			for i in outlines:
				cr.move_to(i[0][0], i[0][1])
				for x,y,_ in i[1:]:
					cr.line_to(x,y)
				cr.close_path()
			cr.fill()
		else:
			cr.paint()
			
		cr.set_operator(cairo.OPERATOR_DEST_OUT)
		
	cr.set_source_rgba(ps.ovr, ps.ovg, ps.ovb, 1)

	for l in rep.layers:
		for k in l.draws:
			if isinstance(k, GD.GerbObj_Line) and (k.width == 0) and ps.strokeZeroWidthLines:
					createCairoLineCenterLinePath(k,cr)
					cr.stroke()
			else:
				GD.emitGerbObjectCairoPath(cr, k)
				
				if (ps.drawfilled):
					cr.fill()
				else:
					cr.stroke()		
			
	cr.pop_group_to_source()
	cr.paint_with_alpha(ps.alpha)
	cr.set_operator(ps.renderOperator)

path = os.path.normpath(path) + "/"

# Load all layers from the directory
layers = {}
for i in os.listdir(path):
	print "Parsing: %s" % i
	identification = GU.identifyLayer(i, convention)
	
	if not identification:
		print "Could not identify type of %s - skipping." % i
		continue
	
	fmt, layer = identification

	if (fmt == 'RS274X'):
		f = GD.parseFile(path+i)
		if (not f):
			print "Could not parse %s" % path+i
			continue 
		
		p = GD.runRS274XProgram(f)
		if (not f):
			print "Could not create polygons for %s" % path+i
			continue 
		layers[layer] = p
		
	elif (fmt == 'EXCELLON'):
		f = GD.parseExcellon(path + i, options.fec)
		
		# Fail gracefully if the excellon file could not be parsed
		if (f):
			layers[layer] = f
		
	else:
		print "Can't handle file type %s" % fmt
		continue


render_order = plotmode.getRenderOrder()

# Calculate the outline path used for drawing the board substrate and calculating
# board coordinates
outline_paths = []
if "MILLING" in layers:
	t = layers["MILLING"]
	outline_line_list = []
	for layer in t.layers:
		for draw in layer.draws:
			if isinstance(draw, GD.GerbObj_Line):
				outline_line_list.append(draw)
	outline_paths = GU.buildCyclePathsForLineSegments(outline_line_list)
elif "OUTLINE" in layers:
	t = layers["OUTLINE"]
	outline_line_list = []
	for layer in t.layers:
		for draw in layer.draws:
			if isinstance(draw, GD.GerbObj_Line):
				outline_line_list.append(draw)
	outline_paths = GU.buildCyclePathsForLineSegments(outline_line_list)
elif "COPPER_TOP" in layers:
	t = layers["COPPER_TOP"]
	outline_line_list = []
	for t_l in t.layers:
		if options.search_outline_nonzero:
			outline_line_list += [k for k in t_l.draws if isinstance(k, GD.GerbObj_Line)]
		else:
			outline_line_list += [k for k in t_l.draws if isinstance(k, GD.GerbObj_Line) and k.width == 0]
	outline_paths = GU.buildCyclePathsForLineSegments(outline_line_list)

# Calculate a bounding rectangle using either all visible objects, or only zero-width lines
check_layers = [v for k,v in layers.items() if not k.startswith("DRILL")]
if (options.bounding_from_visible_only):
	check_layers = [v for k,v in layers.items() if k in render_order and not k.startswith("DRILL")]
artwork_bounds = GU.calculateBoundingRectFromPCBLayers(check_layers, options.size0)

# Calculate the board area, for use in setting up the image / image transform
if outline_paths and not options.bounds_artwork:
	srcrect = GU.calculateBoundingRectFromOutlines(outline_paths)

	srcrect_sane = True
	copper_layers = [v for k,v in layers.iteritems() if k.startswith("COPPER")]
	copper_bounds = GU.calculateBoundingRectFromPCBLayers(copper_layers, False) # looks for non-zero width in copper, verify this is the correct action

	# Check if we have artwork outside the source rectangle
	if  srcrect.getStartPoint().x > copper_bounds.getStartPoint().x or \
		srcrect.getStartPoint().y > copper_bounds.getStartPoint().y or \
		srcrect.getEndPoint().x < copper_bounds.getEndPoint().x or \
		srcrect.getEndPoint().y < copper_bounds.getEndPoint().y:
		srcrect_sane = False 

	if not srcrect_sane:
		print "Warning, copper bounds extend outside of outline-calculated-rectangle," +\
				"falling back to default 'rectangular' board based on bounds-rect"
		srcrect = copper_bounds
		outline_paths = [[
				(srcrect.getStartPoint().x, srcrect.getStartPoint().y, 0),
				(srcrect.getStartPoint().x, srcrect.getEndPoint().y, 0),
				(srcrect.getEndPoint().x, srcrect.getEndPoint().y, 0),
				(srcrect.getEndPoint().x, srcrect.getStartPoint().y, 0)
			]]
		
if not outline_paths or options.bounds_artwork:
	# No outline path found, or bounds were supposed to be found from all artwork
	# so use all artwork except for unscaled drill layer and determine coords
	srcrect = artwork_bounds

# Try to guess-fit the drill layer
if "DRILL" in layers:
	layers["DRILL"].scaleToFit((srcrect.getEndPoint().x, srcrect.getEndPoint().y))

# Calculate the image size and transform
(width, height), transform = GD.prepareCairoTransform(1024, srcrect, pad = 50, trim_to_ratio = True,
		**plotmode.getTransformArgs())

# Prepare a surface to render onto
surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, width, height)
cr = cairo.Context(surface)

# Apply the transform to the context
transform(cr)

# Setup the background
plotmode.renderBackground(cr, outline_paths)

for i in reversed(render_order):
	try:
		t = layers[i];
	except KeyError:
		continue
		
	print "Rendering %s" % i
	renderGerberFile(t, cr, i, outline_paths)

if "DRILL" in layers:
	print "Rendering Drills"
	cr.set_operator(cairo.OPERATOR_OVER)
	cr.set_source_rgba(0,0,0,1);
	for tool,x,y in layers["DRILL"].hits:
		radius = layers["DRILL"].rack.rack[tool]
		cr.arc(x,y,radius/2,0,math.pi * 2)
		cr.close_path()
		cr.fill()
		
surface.write_to_png(open(outputfilename, 'w'))

