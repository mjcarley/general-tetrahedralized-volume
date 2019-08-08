/* gtv-test.c
 * 
 * Copyright (C) 2006 Michael Carley
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /*HAVE_CONFIG_H*/

#ifdef HAVE_GETOPT_H
#  include <getopt.h>
#endif /* HAVE_GETOPT_H */

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include <glib.h>
#include <gts.h>

#include <gtv.h>
#include "gtv-private.h"

gint main(gint argc, gchar **argv)

{
  GtsVertex *v1, *v2, *v3, *v4, *v5, *v6, *v7, *v8, *v9 ;
  GtsEdge *e1, *e2, *e3, *e4, *e5, *e6, *e7, *e8, *e9 ;
  GtvFacet *f1, *f2, *f3, *f4 ;
  GtvCell *c ;
  GtvVolume *v ;
  gdouble len ;
  gchar ch ;
  GLogLevelFlags log_level ;

  v = gtv_volume_new(gtv_volume_class(),
		     gtv_cell_class(),
		     gtv_facet_class(),
		     gts_edge_class(),
		     gts_vertex_class()) ;
  len = 1.0 ; log_level = G_LOG_LEVEL_MESSAGE ;

  while ( (ch = getopt(argc, argv, "hl:L:")) != EOF ) {
    switch (ch) {
    default: 
    case 'h':
      fprintf(stderr, "Usage: %s <options> > output\n\n", 
	      argv[0]) ;
      fprintf(stderr, "Generate a tetrahedralized cube \n\n") ;
      fprintf(stderr, 
	      "Options: \n"
	      "  -h print this message and exit\n"
	      "  -l# set the coordinate of the cube corners\n"
	      "  -L# set the message logging level\n"
	      ) ;
      return 0 ;
      break ;
    case 'l': len = atof(optarg) ; break ;
    case 'L': log_level = 1 << atoi(optarg) ; break ;
    }
  }

  fprintf(stderr, "%s", GTV_STARTUP_MESSAGE) ;
  gtv_logging_init(NULL, NULL, log_level, NULL) ;

  v1 = gts_vertex_new(gts_vertex_class(), -len, -len, -len) ;
  v2 = gts_vertex_new(gts_vertex_class(), len, -len, -len) ;
  v3 = gts_vertex_new(gts_vertex_class(), len, len, -len) ;
  v4 = gts_vertex_new(gts_vertex_class(), -len, len, -len) ;
  v5 = gts_vertex_new(gts_vertex_class(), -len, -len, len) ;
  v6 = gts_vertex_new(gts_vertex_class(), len, -len, len) ;
  v7 = gts_vertex_new(gts_vertex_class(), len, len, len) ;
  v8 = gts_vertex_new(gts_vertex_class(), -len, len, len) ;
  v9 = gts_vertex_new(gts_vertex_class(), 0, 0, 0) ;

  e1 = gts_edge_new(gts_edge_class(), v1, v2) ;
  e2 = gts_edge_new(gts_edge_class(), v2, v3) ;
  e3 = gts_edge_new(gts_edge_class(), v3, v1) ;
  e4 = gts_edge_new(gts_edge_class(), v3, v4) ;
  e5 = gts_edge_new(gts_edge_class(), v4, v1) ;
  e6 = gts_edge_new(gts_edge_class(), v1, v9) ;
  e7 = gts_edge_new(gts_edge_class(), v2, v9) ;
  e8 = gts_edge_new(gts_edge_class(), v3, v9) ;
  e9 = gts_edge_new(gts_edge_class(), v4, v9) ;

  f1 = gtv_facet_new(gtv_facet_class(), e2, e1, e3) ;
  f2 = gtv_facet_new(gtv_facet_class(), e1, e7, e6) ;
  f3 = gtv_facet_new(gtv_facet_class(), e2, e8, e7) ;
  f4 = gtv_facet_new(gtv_facet_class(), e3, e8, e6) ;

  c = gtv_cell_new(gtv_cell_class(), f1, f2, f3, f4) ;
  gtv_volume_add_cell(v, c) ;

  f1 = gtv_facet_new(gtv_facet_class(), e3, e4, e5) ;
  f2 = gtv_facet_new(gtv_facet_class(), e4, e8, e9) ;
  f3 = gtv_facet_new(gtv_facet_class(), e5, e6, e9) ;

  c = gtv_cell_new(gtv_cell_class(), f1, f2, f3, f4) ;
  gtv_volume_add_cell(v, c) ;

  gtv_volume_write(v, stdout) ;

  return 0 ;
}
