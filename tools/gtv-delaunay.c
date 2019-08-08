/* gtv-delaunay.c
 * 
 * Copyright (C) 2007, 2008 Michael Carley
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
#include <stdlib.h>
#include <math.h>

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

/*this is similar to Stephane Popinet's code in `delaunay'*/

static guint read_vertices(GPtrArray *v, FILE *f)

{
  guint line = 1 ; 
  guint nv, ne, nf, nt, i;
  gdouble x, y, z;

  g_return_val_if_fail(v != NULL, 1) ;
  g_return_val_if_fail(f != NULL, 1) ;

  if ( fscanf(f, "%u %u %u %u", &nv, &ne, &nf, &nt) != 4 )
    return line ;
  line ++ ;

  g_ptr_array_set_size (v, nv);
  for ( i = 0 ; i < nv ; i ++ ) {
    if (fscanf (f, "%lg %lg %lg", &x, &y, &z) != 3)
      return line ;
    line ++ ;

    g_ptr_array_index (v, i) = 
      gts_vertex_new (gts_vertex_class (), x, y, z);
  }
  
  return 0 ;
}

gint main(gint argc, gchar **argv)

{
  GtvVolume *v ;
  GtvCell *c ;
  GtsVertex *p = NULL, *v1, *v2, *v3, *v4 ;
  gint i ;
  GSList *j, *cells ;
  GPtrArray *vertices ;
  gdouble len ;
  FILE *input, *output ;
  GtsFile *fid ;
  gboolean remove_hull, check_delaunay, read_volume, 
    write_volume, write_times ;
  gboolean delete_last_vertex ;
  gchar *tetgen_stub = NULL ;
  gchar ch ;
  GLogLevelFlags log_level ;
  GTimer *timer ;
  guint line ;

  len = 4.0 ; log_level = G_LOG_LEVEL_MESSAGE ;
  remove_hull = FALSE ; check_delaunay = FALSE ;
  write_volume = TRUE ; read_volume = FALSE ; write_times = FALSE ;
  delete_last_vertex = FALSE ;
  while ( (ch = getopt(argc, argv, "cdDhl:L:rt:Tw")) != EOF ) {
    switch (ch) {
    default: 
    case 'h':
      fprintf(stderr, "Usage: %s <options> < input > output\n\n", 
	      argv[0]) ;
      fprintf(stderr, 
	      "Generate a Delaunay tetrahedralization of a "
	      "list of points\n\n") ;
      fprintf(stderr, 
	      "Options: \n"
	      "  -c check that the tetrahedralization is Delaunay\n"
	      "  -d check an existing tetrahedralized volume is Delaunay\n"
	      "  -h print this message and exit\n"
	      "  -l# set the coordinate for the enclosing tetrahedron\n"
	      "  -L# set the message logging level\n"
	      "  -r remove the enclosing tetrahedron vertices at the end\n"
	      "  -t (file) write a set of tetgen files to (file).node and "
	      "(file).ele\n"
	      "  -T write timing data to stderr\n"
	      "  -w do not write the GTV volume\n"
	      ) ;
      return 0 ;
      break ;
    case 'c': check_delaunay = TRUE ; break ;
    case 'd': read_volume = check_delaunay = TRUE ; break ;
    case 'l': len = atof(optarg) ; break ;
    case 'L': log_level = 1 << atoi(optarg) ; break ;
    case 'r': remove_hull = TRUE ; break ;
    case 't': tetgen_stub = g_strdup(optarg) ; break ;
    case 'T': write_times = TRUE ; break ;
    case 'w': write_volume = FALSE ; break ;
      /*undocumented options used for testing*/
    case 'D': delete_last_vertex = TRUE ; break ;
    }
  }

  fprintf(stderr, "%s", GTV_STARTUP_MESSAGE) ;

  if ( write_times ) { timer = g_timer_new() ; g_timer_start(timer) ; }
  else timer = NULL ;

  gtv_logging_init(NULL, NULL, log_level, NULL) ;

  v = gtv_volume_new(gtv_volume_class(),
		     gtv_cell_class(),
		     gtv_facet_class(),
		     gts_edge_class(),
		     gts_vertex_class()) ;

  input = stdin ; output = stdout ;

  if ( write_times ) 
    fprintf(stderr, "%s: reading input data: t=%lgs\n", 
	    argv[0], g_timer_elapsed(timer, NULL)) ;
  if ( read_volume ) {
    fid = gts_file_new(input) ;
    gtv_volume_read(v, fid) ;
  } else {
    vertices = g_ptr_array_new() ;
    
    line = read_vertices(vertices, input) ;
    if ( line > 0 ) {
      fprintf(stderr, "%s: error in input at line %u\n", argv[0], line) ;
      return 1 ;
    }

    c = GTV_CELL(gtv_tetrahedron_large((GtvTetrahedronClass *)
				       gtv_cell_class(),
				       gtv_facet_class(),
				       gts_edge_class(),
				       gts_vertex_class(),
				       len)) ;
    gtv_volume_add_cell(v, c) ;

    gtv_tetrahedron_vertices(GTV_TETRAHEDRON(c), &v1, &v2, &v3, &v4) ;

    if ( write_times ) 
      fprintf(stderr, "%s: beginning tetrahedralization: t=%lgs\n", 
	      argv[0], g_timer_elapsed(timer, NULL)) ;
    for ( i = 0 ; i < vertices->len ; i ++ ) {
      p = GTS_VERTEX(g_ptr_array_index(vertices, i)) ;
      if ( gtv_delaunay_add_vertex(v, p, NULL) == GTV_VERTEX_NOT_IN_VOLUME )
	fprintf(stderr,
		"vertex (%lg,%lg,%lg) not inside convex hull\n",
		GTS_POINT(p)->x, GTS_POINT(p)->y, GTS_POINT(p)->z) ;
    }

    if ( write_times ) 
      fprintf(stderr, "%s: tetrahedralization finished: t=%lgs\n", 
	      argv[0], g_timer_elapsed(timer, NULL)) ;

/*     if ( delete_last_vertex ) gtv_delaunay_remove_vertex(v, p) ; */

    if ( remove_hull ) {
      cells = gtv_vertex_cells(v1, v) ;
      for ( j = cells ; j != NULL ; j = j->next )
	gtv_volume_remove_cell(v, GTV_CELL(j->data)) ;

      cells = gtv_vertex_cells(v2, v) ;
      for ( j = cells ; j != NULL ; j = j->next )
	gtv_volume_remove_cell(v, GTV_CELL(j->data)) ;

      cells = gtv_vertex_cells(v3, v) ;
      for ( j = cells ; j != NULL ; j = j->next )
	gtv_volume_remove_cell(v, GTV_CELL(j->data)) ;

      cells = gtv_vertex_cells(v4, v) ;
      for ( j = cells ; j != NULL ; j = j->next )
	gtv_volume_remove_cell(v, GTV_CELL(j->data)) ;
    }
  }

  if ( check_delaunay ) {
    c = gtv_delaunay_check(v) ;
    if ( c != NULL ) {
      fprintf(stderr, "Volume violates Delaunay condition, cell:\n") ;
      gtv_tetrahedron_vertices(GTV_TETRAHEDRON(c), &v1, &v2, &v3, &v4) ;
      fprintf(stderr, "(%lg,%lg,%lg) (%lg,%lg,%lg) "
	      "(%lg,%lg,%lg) (%lg,%lg,%lg)\n",
	      GTS_POINT(v1)->x, GTS_POINT(v1)->y, GTS_POINT(v1)->z,
	      GTS_POINT(v2)->x, GTS_POINT(v2)->y, GTS_POINT(v2)->z,
	      GTS_POINT(v3)->x, GTS_POINT(v3)->y, GTS_POINT(v3)->z,
	      GTS_POINT(v4)->x, GTS_POINT(v4)->y, GTS_POINT(v4)->z) ;
      v1 = gtv_tetrahedron_is_encroached(GTV_TETRAHEDRON(c)) ;
      fprintf(stderr, "is encroached by vertex (%lg,%lg,%lg)\n", 
	      GTS_POINT(v1)->x, GTS_POINT(v1)->y, GTS_POINT(v1)->z) ;
    }
    else
      fprintf(stderr, "Volume is valid Delaunay tetrahedralization\n") ;
  }

  if ( tetgen_stub != NULL ) gtv_volume_write_tetgen(v, tetgen_stub) ;    

  if ( write_volume ) gtv_volume_write(v, output) ;

  return 0 ;
}
