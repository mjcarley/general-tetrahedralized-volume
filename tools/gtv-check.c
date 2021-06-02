/* gtv-check.c
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
  FILE *input ;
  /* , *output ; */
  GtsFile *fid ;
  gchar ch ;
  GLogLevelFlags log_level ;
  gboolean verbose ;

  verbose = FALSE ;
  log_level = G_LOG_LEVEL_MESSAGE ;
  while ( (ch = getopt(argc, argv, "hv")) != EOF ) {
    switch (ch) {
    default: 
    case 'h':
      fprintf(stderr, "Usage: %s <options> < input > output\n\n", 
	      argv[0]) ;
      fprintf(stderr, 
	      "Diagnostic information on a GTV volume\n\n") ;
      fprintf(stderr, 
	      "Options: \n"
	      "  -h print this message and exit\n"
	      "  -L# set the message logging level\n"
	      "  -v verbose, print statistics about surface\n"
	      ) ;
      return 0 ;
      break ;
    case 'L': log_level = 1 << atoi(optarg) ; break ;
    case 'v': verbose = TRUE ; break ;
    }
  }

  fprintf(stderr, "%s", GTV_STARTUP_MESSAGE) ;

  gtv_logging_init(NULL, NULL, log_level, NULL) ;

  v = gtv_volume_new(gtv_volume_class(),
		     gtv_cell_class(),
		     gtv_facet_class(),
		     gts_edge_class(),
		     gts_vertex_class()) ;

  input = stdin ;
  /* output = stdout ; */

  fid = gts_file_new(input) ;

  gtv_volume_read(v, fid) ;

  if ( verbose ) gtv_volume_print_stats(v, stderr) ;

  return 0 ;
}
