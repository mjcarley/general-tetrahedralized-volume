/* gtv2msh.c
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

gint main(gint argc, gchar **argv)

{
  GtvVolume *v ;
  GtsFile *f ;
  FILE *input, *output ;
  gchar ch ;
  GLogLevelFlags log_level ;

  input = stdin ; output = stdout ;
  log_level = G_LOG_LEVEL_MESSAGE ;

  while ( (ch = getopt(argc, argv, "hL:")) != EOF ) {
    switch (ch) {
    default: 
    case 'h':
      fprintf(stderr, "Usage: %s <options> < input > output\n\n", 
	      argv[0]) ;
      fprintf(stderr, 
	      "Read a gtv volume and output it as a gmsh .msh file\n\n"
	      "Options: \n"
	      "  -L# set the message logging level\n") ;
      return 0 ;
      break ;
      case 'L': log_level = 1 << atoi(optarg) ; break ;
    }
  }  

  fprintf(stderr, "%s", GTV_STARTUP_MESSAGE) ;

  gtv_logging_init(NULL, NULL, log_level, NULL) ;

  v = gtv_volume_new(gtv_volume_class(),
		     gtv_cell_class(),
		     gtv_facet_class(),
		     gts_edge_class(),
		     gts_vertex_class()) ;
  f = gts_file_new(input) ;
  
  gtv_volume_read(v, f) ;

  gtv_volume_write_gmsh(v, output) ;

  return 0 ;
}
