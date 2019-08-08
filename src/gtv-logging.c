/* GTV - Library for the manipulation of tetrahedralized volumes
 * Copyright (C) 2007, 2008 Michael Carley
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <math.h>

#include <gts.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#include "gtv.h"
#include "gtv-private.h"

/**
 * @defgroup logging Logging functions
 * @{
 * 
 */

void gtv_logging_func(const gchar *log_domain,
		      GLogLevelFlags log_level,
		      const gchar *message,
		      gpointer data[]) ;
const gchar *gtv_logging_string(GLogLevelFlags level) ;

const gchar *gtv_logging_string(GLogLevelFlags level)

{
  const gchar *strings[] = {"RECURSION", 
			    "FATAL",
			    "ERROR",
			    "CRITICAL",
			    "WARNING",
			    "MESSAGE",
			    "INFO",
			    "DEBUG"} ;

/*   if ( G_LOG_FLAG_RECURSION & level) return strings[0] ; */
/*   if ( G_LOG_FLAG_FATAL & level) return strings[1] ;  */
  if ( G_LOG_LEVEL_ERROR & level) return strings[2] ; 
  if ( G_LOG_LEVEL_CRITICAL & level) return strings[3] ; 
  if ( G_LOG_LEVEL_WARNING & level) return strings[4] ; 
  if ( G_LOG_LEVEL_MESSAGE & level) return strings[5] ; 
  if ( G_LOG_LEVEL_INFO & level) return strings[6] ; 
  if ( G_LOG_LEVEL_DEBUG & level) return strings[7] ; 

  g_assert_not_reached() ;

  return NULL ;
}

void gtv_logging_func(const gchar *log_domain,
		      GLogLevelFlags log_level,
		      const gchar *message,
		      gpointer data[])

{
  FILE *f = (FILE *)data[GTV_LOGGING_DATA_FID] ;
  gchar *p = (gchar *)data[GTV_LOGGING_DATA_PREFIX] ;
  GLogLevelFlags level = *(GLogLevelFlags *)data[GTV_LOGGING_DATA_LEVEL] ;
  gint (*exit_func)(void) = data[GTV_LOGGING_DATA_EXIT_FUNC] ;

  if ( log_level > level ) return ;

  fprintf(f, "%s%s-%s: %s\n", p, 
	  G_LOG_DOMAIN, gtv_logging_string(log_level),
	  message) ;

  if ( log_level <= G_LOG_LEVEL_ERROR ) {
    if ( exit_func != NULL ) exit_func() ;
  }

  return ;
}

/** 
 * Initialize GTV logging
 * 
 * @param f file stream for messages
 * @param p string to prepend to messages
 * @param log_level maximum logging level to handle (see g_log)
 * @param exit_func function to call if exiting on an error
 * 
 * @return GTV_SUCCESS on success
 */

gint gtv_logging_init(FILE *f, gchar *p, 
		      GLogLevelFlags log_level,
		      gpointer exit_func)

{
  static gpointer data[GTV_LOGGING_DATA_WIDTH] ;
  static GLogLevelFlags level ;

  if ( f != NULL ) data[GTV_LOGGING_DATA_FID] = f ;
  else data[GTV_LOGGING_DATA_FID] = stderr ;    

  if ( p != NULL ) data[GTV_LOGGING_DATA_PREFIX] = g_strdup(p) ;
  else data[GTV_LOGGING_DATA_PREFIX] = g_strdup("") ;

  level = log_level ;
  data[GTV_LOGGING_DATA_LEVEL] = &level ;    

  if ( level == G_LOG_LEVEL_DEBUG ) 
    g_warning("%s: setting a logging level of G_LOG_LEVEL_DEBUG " 
	      "will generate a lot of output", __FUNCTION__) ;
    
  g_log_set_handler (G_LOG_DOMAIN, 
		     G_LOG_FLAG_RECURSION |
		     G_LOG_FLAG_FATAL |   
		     G_LOG_LEVEL_ERROR |
		     G_LOG_LEVEL_CRITICAL |
		     G_LOG_LEVEL_WARNING |
		     G_LOG_LEVEL_MESSAGE |
		     G_LOG_LEVEL_INFO |
		     G_LOG_LEVEL_DEBUG,
		     (GLogFunc)gtv_logging_func, data);

  return GTV_SUCCESS ;
}

/**
 * @}
 * 
 */

