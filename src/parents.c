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
 * @defgroup parent Parent entities
 * @{
 * 
 */

/** 
 * Check if a ::GtsVertex has a given parent volume.
 * 
 * @param p a ::GtsVertex;
 * @param v a ::GtvVolume.
 * 
 * @return a ::GtsEdge of \a v containing \a p, NULL otherwise.
 */

GtsEdge *gtv_vertex_has_parent_volume(GtsVertex *p, GtvVolume *v)

{
  GSList *i ;

  g_return_val_if_fail(p != NULL, NULL) ;
  i = p->segments ;
  while (i) {
    if (GTS_IS_EDGE (i->data) && 
	gtv_edge_has_parent_volume (i->data, v))
      return i->data;
    i = i->next;
  }

  return NULL ;
}

/** 
 * Check if a ::GtsEdge has a given parent volume.
 * 
 * @param e a ::GtsEdge;
 * @param v a ::GtvVolume.
 * 
 * @return a ::GtvFacet of \a v containing \a e, NULL otherwise.
 */

GtvFacet *gtv_edge_has_parent_volume(GtsEdge *e, GtvVolume *v)

{
  GSList *i ;

  g_return_val_if_fail(e != NULL, NULL) ;
  i = e->triangles ;
  while (i) {
    if (GTV_IS_FACET (i->data) && 
	gtv_facet_has_parent_volume (i->data, v))
      return i->data;
    i = i->next;
  }

  return NULL ;
}

/** 
 * Check if a ::GtvFacet has a given parent volume.
 * 
 * @param f a ::GtvFacet;
 * @param v a ::GtvVolume.
 * 
 * @return a ::GtvCell of \a v containing \a f, NULL otherwise.
 */

GtvCell *gtv_facet_has_parent_volume(GtvFacet *f, GtvVolume *v)

{
  GSList *i ;

  g_return_val_if_fail(f != NULL, NULL) ;
  i = f->tetrahedra ;
  while (i) {
    if (GTV_IS_CELL (i->data) && 
	gtv_cell_has_parent_volume (i->data, v))
      return i->data;
    i = i->next;
  }

  return NULL ;
}

/** 
 * Check if a ::GtvCell belongs to a given ::GtvVolume.
 * 
 * @param c a ::GtvCell;
 * @param v a ::GtvVolume.
 * 
 * @return TRUE if \a c belongs to \a v, FALSE otherwise.
 */

gboolean gtv_cell_has_parent_volume(GtvCell *c, GtvVolume *v)

{
  GSList *i ;

  g_return_val_if_fail(c != NULL, FALSE) ;

  i = c->volumes ;

  while (i) {
    if ( i->data == v ) return TRUE ;
    i = i->next ;
  }

  return FALSE ;
}

/** 
 * Count the number of facets using an edge.
 * 
 * @param e a ::GtsEdge;
 * @param v a ::GtvVolume.
 * 
 * @return the number of facets of \a v which contain \a e.
 */

guint gtv_edge_facet_number(GtsEdge *e, GtvVolume *v)

{
  GSList *i ;
  guint nt = 0 ;

  g_return_val_if_fail(e != NULL, 0) ;
  g_return_val_if_fail(v != NULL, 0) ;

  i = e->triangles ;

  while ( i ) {
    if ( GTV_IS_FACET(i->data) &&
	 gtv_facet_has_parent_volume(GTV_FACET(i->data), v) )
      nt ++ ;
    i = i->next ;
  }

  return nt ;
}

/** 
 * Count the number of cells using a facet.
 * 
 * @param f a ::GtvFacet;
 * @param v a ::GtvVolume.
 * 
 * @return the number of cells of \a v which contain \a f.
 */

guint gtv_facet_cell_number(GtvFacet *f, GtvVolume *v)

{
  GSList *i ;
  guint nc = 0 ;

  g_return_val_if_fail(f != NULL, 0) ;
  g_return_val_if_fail(v != NULL, 0) ;

  i = f->tetrahedra ;

  while ( i ) {
    if ( GTV_IS_CELL(i->data) &&
	 gtv_cell_has_parent_volume(GTV_CELL(i->data), v) )
      nc ++ ;
    i = i->next ;
  }

  return nc ;
}

/**
 * @}
 * 
 */
