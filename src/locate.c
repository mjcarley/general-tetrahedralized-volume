/* GTV - Library for the manipulation of tetrahedralized volumes
 *
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
#include <stdlib.h>

#include <gts.h>

#include "predicates.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#include "gtv.h"
#include "gtv-private.h"

#define NEIGHBOURS_THROUGH_FACET(_t,_p,_f) \
((_t != _p) && gtv_tetrahedron_has_facet(GTV_TETRAHEDRON(_p), _f))

#define OUTSIDE_FACET(_v1,_v2,_v3,_v4,_D) \
(( _D > 0.0 ? \
	   (gts_point_orientation_3d(_v1, _v2, _v3, _v4) < 0.0) : \
           (gts_point_orientation_3d(_v1, _v2, _v3, _v4) > 0.0) ))

/**
 * @defgroup locate Point location in a volume
 * @{
 * 
 */

/*this comes from Stephane Popinet's GTS code*/

#if GLIB_CHECK_VERSION(2,4,0)

static gboolean cell_distance(gpointer key, gpointer value,
			      gpointer data[])

{
  GtvCell **t = (GtvCell **)(data[0]) ;
  gint ns = *(gint *)data[1] ;
  gint *i = (gint *)data[2] ;
  gdouble *Rmin = (gdouble *)data[3] ;
  GtsPoint *p = (GtsPoint *)data[4] ;
  GtsPoint *p1 ;
  gdouble R2 ;

  p1 = GTS_POINT (GTS_SEGMENT (GTS_TRIANGLE 
			       (GTV_TETRAHEDRON(key)->f1)->e1)->v1);
  if ( *i == 0 ) {
    *t = GTV_CELL(key) ; *i = 1 ; 
    *Rmin = gts_point_distance2(p, p1) ;
    return FALSE ;
  }
  (*i) ++ ;
  
  R2 = gts_point_distance2(p, p1) ;

  if ( R2 < *Rmin ) {
    *t = GTV_CELL(key) ; *Rmin = R2 ;
  }
  
  return (*i >= ns) ;
}

static GtvCell *random_closest_cell(GHashTable *h, GtsPoint *p) 

{
  gpointer data[8] ;
  gint i, ns, nt ;
  gdouble R2 ;
  GtvCell *t ;

  nt = g_hash_table_size(h);
  if ( nt == 0 ) return NULL ;
  ns = exp(log((gdouble) nt)/3.0) ; i = 0 ; R2 = G_MAXDOUBLE ;

  data[0] = &t ; data[1] = &ns ; data[2] = &i ; data[3] = &R2 ;
  data[4] = p ;
  
  g_hash_table_find(h, (GHRFunc)cell_distance, data) ;

  return t ;
}

#else /*if GLIB_CHECK_VERSION(2,4,0)*/
typedef struct _GHashNode      GHashNode;

struct _GHashNode
{
  gpointer key;
  gpointer value;
  GHashNode *next;
};

#if GLIB_CHECK_VERSION(1,2,8)
struct _GHashTable
{
  gint size;
  gint nnodes;
  guint frozen;
  GHashNode **nodes;
  GHashFunc hash_func;
  GCompareFunc key_compare_func;
};
#else  /* not 1.2.8 */
#  if GLIB_CHECK_VERSION(1,3,0)
struct _GHashTable
{
  gint size;
  gint nnodes;
  GHashNode **nodes;
  GHashFunc hash_func;
  GEqualFunc key_equal_func;
};
#  else  /* not 1.3.0 */
#    error "Hack needs glib version 1.2.8 or 1.3.0"
#  endif /* not 1.3.0 */
#endif /* not 1.2.8 */

static GtvCell *random_closest_cell(GHashTable *h, GtsPoint *p)

{
  GtvCell *closest, *test ;
  gdouble R, Rmin = G_MAXDOUBLE ;
  GHashNode *node ;
  gint i, ns, n, nt ;

  nt = g_hash_table_size(h);
  if ( nt == 0 ) return NULL ;
  ns = exp(log((gdouble) nt)/3.) ;

  closest = NULL ;
  for ( (i = 0), (n = 0) ; i < h->size && n < ns ; i ++ ) {
    for ( node = h->nodes[i] ; node && n < ns ; node = node->next ) {
      test = GTV_CELL(node->key) ;
      R = gts_point_distance2(p,
			      GTS_POINT(GTS_SEGMENT(GTS_TRIANGLE(GTV_TETRAHEDRON(test)->f1)->e1)->v1)) ;
      if ( R < Rmin ) {
	Rmin = R ;
	closest = test ;
      }
      n ++ ;
    }
  }

  return closest ;
}

#endif /* VERSION > 2.4.0 */

/*end of Stephane Popinet's code*/

static GtvFacet *random_facet(GtvTetrahedron *t,
			      GtsPoint **v1,
			      GtsPoint **v2,
			      GtsPoint **v3,
			      GtsPoint **v4)

{
  gint i ;
  GtsVertex *w1, *w2, *w3, *w4 ;

  g_assert(t != NULL) ;
  gtv_tetrahedron_vertices(t, &w1, &w2, &w3, &w4) ;
  i = random() ;
  if ( i < RAND_MAX/4 ) {
    *v1 = GTS_POINT(w1) ; *v2 = GTS_POINT(w2) ; 
    *v3 = GTS_POINT(w3) ; *v4 = GTS_POINT(w4) ; 
    return t->f1 ; 
  }
  if ( i < RAND_MAX/2 ) {
    *v1 = GTS_POINT(w2) ; *v2 = GTS_POINT(w3) ; 
    *v3 = GTS_POINT(w4) ; *v4 = GTS_POINT(w1) ; 
    return t->f2 ; 
  }
  if ( i < RAND_MAX/4*3 ) {
    *v1 = GTS_POINT(w3) ; *v2 = GTS_POINT(w4) ; 
    *v3 = GTS_POINT(w1) ; *v4 = GTS_POINT(w2) ; 
    return t->f3 ; 
  }

  *v1 = GTS_POINT(w4) ; *v2 = GTS_POINT(w1) ; 
  *v3 = GTS_POINT(w2) ; *v4 = GTS_POINT(w3) ; 
  return t->f4 ; 
}

#if GLIB_CHECK_VERSION(2,4,0)

static gboolean hash_cell_locate(gpointer key, gpointer value,
				 gpointer data[])

{
  GtsPoint *p = (GtsPoint *)data[0] ;
  GtvCell **t = (GtvCell **)(data[1]) ;
  GtvIntersect inter ;

  if ( (inter = gtv_point_in_tetrahedron(p, 
					 GTV_TETRAHEDRON(key),
					 NULL)) 
       != GTV_OUT ) {
    g_debug("%s: point %p (%lg,%lg,%lg) located %s %p",
	    __FUNCTION__, p, 
	    GTS_POINT(p)->x, GTS_POINT(p)->y, GTS_POINT(p)->z,
	    intersection_status(inter), key) ;
    *t = key ;
    return TRUE ;
  }
  
  return FALSE ;
}

#else

static gint foreach_cell_locate(GtvCell *t, gpointer data[])

{
  GtsPoint *p = GTS_POINT(data[0]) ;
  GtvCell **c = (GtvCell **)data[1] ;
  GtvIntersect inter ;

  if ( (inter = gtv_point_in_tetrahedron(p, 
					 GTV_TETRAHEDRON(t),
					 NULL)) 
       != GTV_OUT ) {
    g_debug("%s: point %p (%lg,%lg,%lg) located %s %p",
	    __FUNCTION__, p, 
	    GTS_POINT(p)->x, GTS_POINT(p)->y, GTS_POINT(p)->z,
	    intersection_status(inter), t) ;
    *c = t ;
  }

  return GTV_SUCCESS ;
}
#endif /* VERSION > 2.4.0 */

/** 
 * Find a ::GtvCell in a given volume which encloses a ::GtsPoint by
 * sequentially testing the cells. This is not a very good way of
 * locating a point but it does work and can be used in testing other
 * point location methods.
 * 
 * @param p a ::GtsPoint;
 * @param volume a ::GtvVolume to search for the location of \a p;
 * @param guess ignored, included for compatibility with other 
 * location functions. 
 * 
 * @return a ::GtvCell containing \a p or NULL if \a p is not in \a v.
 */

GtvCell *gtv_point_locate_slow(GtsPoint *p, GtvVolume *volume, 
			       GtvCell *guess)

{
  GtvCell *t = NULL ;
  gpointer data[2] ;

  g_return_val_if_fail(p != NULL, NULL) ;
  g_return_val_if_fail(GTS_IS_POINT(p), NULL) ;
  g_return_val_if_fail(volume != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_VOLUME(volume) , NULL) ;

  g_debug("%s: ", __FUNCTION__) ;

  data[0] = p ; data[1] = &t ;

#if GLIB_CHECK_VERSION(2,4,0)
  g_hash_table_find(volume->cells, (GHRFunc)hash_cell_locate, data) ;
#else
  gtv_volume_foreach_cell(volume, (GtsFunc)foreach_cell_locate, data) ;
#endif /* VERSION > 2.4.0 */

  if ( t != NULL ) 
    g_assert( gtv_point_in_tetrahedron(GTS_POINT(p), 
				       GTV_TETRAHEDRON(t), NULL) != GTV_OUT ) ;

  return t ;
}

static inline GtvFacet *next_facet(GtvTetrahedron *t, GtvFacet *f)

{
  if ( f == t->f1 ) return t->f2 ;
  if ( f == t->f2 ) return t->f3 ;
  if ( f == t->f3 ) return t->f4 ;
  if ( f == t->f4 ) return t->f1 ;

  g_assert_not_reached() ;
  return NULL ;
}

/** 
 * Locate a point in a tetrahedralization using a remembering
 * stochastic walk (Olivier Devillers, Sylvain Pion and Monique
 * Teillaud, 2002, <a
 * href="http://hal.inria.fr/docs/00/10/21/94/PDF/hal.pdf">`Walking in
 * a triangulation'</a>, International Journal of Foundations in
 * Computer Science, 13:181--199). Note that at present this will not
 * work for volumes with holes in their interior.
 * 
 * @param p a GtsPoint to locate;
 * @param volume a ::GtvVolume;
 * @param guess a ::GtvCell which is an initial guess for a cell 
 * containing \a p or NULL.
 * 
 * @return a ::GtvCell containing \a p or NULL if \a p is not in \a
 * volume.
 */

GtvCell *gtv_point_locate(GtsPoint *p, GtvVolume *volume, GtvCell *guess)

{
  GtvCell *t = NULL, *previous ;
  GtvFacet *f ;
  GtsPoint *v1, *v2, *v3, *v4 ;
  gdouble D ;
  gboolean stop ;

  g_return_val_if_fail(p != NULL, NULL) ;
  g_return_val_if_fail(GTS_IS_POINT(p), NULL) ;
  g_return_val_if_fail(volume != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_VOLUME(volume) , NULL) ;

  g_debug("%s: p=%p; volume=%p; guess=%p", 
	  __FUNCTION__, p, volume, guess) ;

  if ( guess != NULL ) 
    t = guess ;
  else
    t = random_closest_cell(volume->cells, p) ;

  stop = FALSE ; previous = t ;

  while ( !stop ) {
    f = random_facet(GTV_TETRAHEDRON(t), &v1, &v2, &v3, &v4) ;
    D = gts_point_orientation_3d(GTS_POINT(v1), GTS_POINT(v2),
				 GTS_POINT(v3), GTS_POINT(v4)) ;
    if ( !NEIGHBOURS_THROUGH_FACET(t, previous, f) &&
	 OUTSIDE_FACET(p, v2, v3, v4, D) ) {
      previous = t ;
      t = GTV_CELL(gtv_tetrahedron_opposite(GTV_TETRAHEDRON(t), f)) ;
      if ( t == NULL ) stop = TRUE ;
    } else {
      f = next_facet(GTV_TETRAHEDRON(t), f) ;
      if ( !NEIGHBOURS_THROUGH_FACET(t, previous, f) &&
	   OUTSIDE_FACET(v1, p, v3, v4, D) ) {
	previous = t ;
	t = GTV_CELL(gtv_tetrahedron_opposite(GTV_TETRAHEDRON(t), f)) ;
	if ( t == NULL ) stop = TRUE ;
      } else {
	f = next_facet(GTV_TETRAHEDRON(t), f) ;
	if ( !NEIGHBOURS_THROUGH_FACET(t, previous, f) &&
	     OUTSIDE_FACET(v1, v2, p, v4, D) ) {
	  previous = t ;
	  t = GTV_CELL(gtv_tetrahedron_opposite(GTV_TETRAHEDRON(t), f)) ;
	  if ( t == NULL ) stop = TRUE ;
	} else {
	  f = next_facet(GTV_TETRAHEDRON(t), f) ;
	  if ( !NEIGHBOURS_THROUGH_FACET(t, previous, f) &&
	       OUTSIDE_FACET(v1, v2, v3, p, D) ) {
	    previous = t ;
	    t = GTV_CELL(gtv_tetrahedron_opposite(GTV_TETRAHEDRON(t), f)) ;
	    if ( t == NULL ) stop = TRUE ;
	  } else 
	    stop = TRUE ;
	}
      }
    }
  }

  if ( t != NULL ) 
    g_assert( gtv_point_in_tetrahedron(GTS_POINT(p), 
				       GTV_TETRAHEDRON(t), NULL) != GTV_OUT ) ;

  return t ;
}

/**
 * @}
 * 
 */
