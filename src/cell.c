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
 * @defgroup cell Cells
 * @{
 * 
 */

static void cell_destroy (GtsObject * object)
{
  /* do object-specific cleanup here */
  GtvCell *c = GTV_CELL(object);
  GSList * i;

  i = c->volumes ;
  while ( i ) {
    GSList * next = i->next;
    gtv_volume_remove_cell(i->data, c);
    i = next ;
  }
  g_assert (c->volumes == NULL);

  /* do not forget to call destroy method of the parent */
  (* GTS_OBJECT_CLASS (gtv_cell_class ())->parent_class->destroy) 
    (object);
}

static void gtv_cell_class_init (GtvCellClass * klass)
{
  /* define new methods and overload inherited methods here */
  GTS_OBJECT_CLASS (klass)->destroy = cell_destroy;
}

static void gtv_cell_init (GtvCell * object)
{
  /* initialize object here */
}

/** 
 * The basic class for cells (volume elements) in GTV.
 * 
 * @return the ::GtvCellClass
 */


GtvCellClass * gtv_cell_class (void)

{
  static GtvCellClass * klass = NULL;

  if (klass == NULL) {
    GtsObjectClassInfo gtv_cell_info = {
      "GtvCell",
      sizeof (GtvCell),
      sizeof (GtvCellClass),
      (GtsObjectClassInitFunc) gtv_cell_class_init,
      (GtsObjectInitFunc) gtv_cell_init,
      (GtsArgSetFunc) NULL,
      (GtsArgGetFunc) NULL
    };
    klass = gts_object_class_new (GTS_OBJECT_CLASS (gtv_tetrahedron_class ()),
				  &gtv_cell_info);
  }

  return klass;
}

/** 
 * Make a new ::GtvCell from four facets which must define a proper
 * tetrahedron.
 * 
 * @param klass a ::GtvCellClass; 
 * @param f1 a ::GtvFacet;
 * @param f2 another ::GtvFacet;
 * @param f3 a further ::GtvFacet;
 * @param f4 one more ::GtvFacet;
 * 
 * @return the new ::GtvCell.
 */

GtvCell * gtv_cell_new (GtvCellClass * klass,
			GtvFacet *f1,
			GtvFacet *f2,
			GtvFacet *f3,
			GtvFacet *f4)

{
  GtvCell * object;

  object = GTV_CELL(gts_object_new (GTS_OBJECT_CLASS (klass)));
  gtv_tetrahedron_set(GTV_TETRAHEDRON(object), f1, f2, f3, f4) ;

  return object;
}

/** 
 * Find the neighbours of a ::GtvCell.
 * 
 * @param c a ::GtvCell;
 * @param v a ::GtvVolume, or NULL.
 * 
 * @return a ::GSList of the cells of \a v, or any cells if \a v is
 * NULL, which share a facet with \a c.
 */

GSList *gtv_cell_neighbours(GtvCell *c, GtvVolume *v)

{
  GSList *i, *n = NULL ;
  GtvFacet *f[5], **f1 = f ;

  g_return_val_if_fail(c != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_CELL(c), NULL) ;
  if ( v != NULL ) 
    g_return_val_if_fail(GTV_IS_VOLUME(v), NULL) ;

  f[0] = GTV_TETRAHEDRON(c)->f1 ;
  f[1] = GTV_TETRAHEDRON(c)->f2 ;
  f[2] = GTV_TETRAHEDRON(c)->f3 ;
  f[3] = GTV_TETRAHEDRON(c)->f4 ;
  f[4] = NULL ;

  while (*f1) {
    i = (*f1++)->tetrahedra ;
    while ( i ) {
      GtvTetrahedron *t = i->data ;
      if ( GTV_CELL(t) != c &&
	   GTV_IS_CELL(t) &&
	   (!v || gtv_cell_has_parent_volume(GTV_CELL(t), v)))
	n = g_slist_prepend(n, t) ;
      i = i->next ;
    }
  }

  return n ;
}

/** 
 * Generate a new ::GtvCell from four ::GtsVertex's, making use of any
 * existing ::GtvFacet's which connect them.
 * 
 * @param klass a ::GtvCellClass
 * @param facet_class a ::GtvFacetClass
 * @param edge_class a ::GtsEdgeClass
 * @param v1 a ::GtsVertex
 * @param v2 a ::GtsVertex
 * @param v3 a ::GtsVertex
 * @param v4 a ::GtsVertex
 * 
 * @return the new ::GtvCell
 */

GtvCell *gtv_cell_new_from_vertices(GtvCellClass *klass,
				    GtvFacetClass *facet_class,
				    GtsEdgeClass *edge_class,
				    GtsVertex *v1,
				    GtsVertex *v2,
				    GtsVertex *v3,
				    GtsVertex *v4)

{
  GtvCell *c ;
  GtvFacet *f1, *f2, *f3, *f4 ;

  g_return_val_if_fail(klass != NULL, NULL) ;
  g_return_val_if_fail(klass == 
		       gts_object_class_is_from_class(klass, 
						      gtv_cell_class()),
		       NULL) ;
  g_return_val_if_fail(facet_class != NULL, NULL) ;
  g_return_val_if_fail(facet_class == 
		       gts_object_class_is_from_class(facet_class, 
						      gtv_facet_class()),
		       NULL) ;
  g_return_val_if_fail(edge_class != NULL, NULL) ;
  g_return_val_if_fail(edge_class == 
		       gts_object_class_is_from_class(edge_class, 
						      gts_edge_class()),
		       NULL) ;
  g_return_val_if_fail(v1 != NULL, NULL) ;
  g_return_val_if_fail(GTS_IS_VERTEX(v1), NULL) ;
  g_return_val_if_fail(v2 != NULL, NULL) ;
  g_return_val_if_fail(GTS_IS_VERTEX(v2), NULL) ;
  g_return_val_if_fail(v3 != NULL, NULL) ;
  g_return_val_if_fail(GTS_IS_VERTEX(v3), NULL) ;
  g_return_val_if_fail(v4 != NULL, NULL) ;
  g_return_val_if_fail(GTS_IS_VERTEX(v4), NULL) ;
  
  g_return_val_if_fail(v1 != v2 && v1 != v3 && v1 != v4 && 
		       v2 != v3 && v2 != v4 && v3 != v4, NULL) ;  
  
  if ( (f1 = GTV_FACET(triangle_from_vertices(v2, v3, v4))) == NULL )
    f1 = facet_new_from_vertices(facet_class, edge_class, v2, v3, v4) ;

  if ( (f2 = GTV_FACET(triangle_from_vertices(v3, v4, v1))) == NULL )
    f2 = facet_new_from_vertices(facet_class, edge_class, v3, v4, v1) ;

  if ( (f3 = GTV_FACET(triangle_from_vertices(v4, v1, v2))) == NULL )
    f3 = facet_new_from_vertices(facet_class, edge_class, v4, v1, v2) ;

  if ( (f4 = GTV_FACET(triangle_from_vertices(v1, v2, v3))) == NULL )
    f4 = facet_new_from_vertices(facet_class, edge_class, v1, v2, v3) ;

  if ( (c = GTV_CELL(gtv_tetrahedron_from_facets(f1, f2, f3, f4))) == NULL )
    c = gtv_cell_new(klass, f1, f2, f3, f4) ;
  else 
    g_debug("%s: vertices %p, %p, %p, %p already form cell %p", 
	    __FUNCTION__, v1, v2, v3, v4, c) ;
  return c ;
}

/** 
 * Find the ::GtvCell's which use a given ::GtsEdge.
 * 
 * @param e a ::GtsEdge 
 * @param v a ::GtvVolume
 * 
 * @return a ::GSList of the ::GtvCell's of \a v which contain \a e,
 * NULL if \a e is not used by any cells.
 */

GSList *gtv_edge_cells(GtsEdge *e, GtvVolume *v)

{
  GSList *j, *i, *c ;
  
  g_return_val_if_fail(e != NULL, NULL) ;
  g_return_val_if_fail(GTS_IS_EDGE(e), NULL) ;
  if ( v != NULL ) 
    g_return_val_if_fail(GTV_IS_VOLUME(v), NULL) ;

  g_debug("%s:", __FUNCTION__) ;

  for ( (c = NULL), (i = e->triangles) ; i != NULL ; i = i->next ) {
    for ( j = GTV_FACET(i->data)->tetrahedra ; j != NULL ; j = j->next ) {
      if ( g_slist_find(c, j->data) == NULL &&
	   (!v || gtv_cell_has_parent_volume(GTV_CELL(j->data), v)) )
	c = g_slist_prepend(c, j->data) ;
    }
  }

  g_debug("%s: %d cells on edge %p",
	__FUNCTION__, g_slist_length(c), e) ;

  return c ;
}


/** 
 * Find the ::GtvCell's which use a given ::GtsVertex
 * 
 * @param p a ::GtsVertex;
 * @param v a ::GtvVolume or NULL.
 * 
 * @return a ::GSList of the ::GtvCell's of \a v which contain \a e,
 * NULL if \a e is not used by any cells.
 */

GSList *gtv_vertex_cells(GtsVertex *p, GtvVolume *v)

{
  GSList *j, *i, *c, *t ;
  
  g_return_val_if_fail(p != NULL, NULL) ;
  g_return_val_if_fail(GTS_IS_VERTEX(p), NULL) ;
  if ( v != NULL )
    g_return_val_if_fail(GTV_IS_VOLUME(v), NULL) ;
  g_debug("%s:", __FUNCTION__) ;

  t = gts_vertex_triangles(p, NULL) ;
  for ( (c = NULL), (i = t) ; i != NULL ; i = i->next ) {
    for ( j = GTV_FACET(i->data)->tetrahedra ; j != NULL ; j = j->next ) {
      if ( g_slist_find(c, j->data) == NULL &&
	   (!v || gtv_cell_has_parent_volume(GTV_CELL(j->data), v)) )
	c = g_slist_prepend(c, j->data) ;
    }
  }

  g_slist_free(t) ;

  g_debug("%s: %d cells on vertex %p", __FUNCTION__, g_slist_length(c), p) ;

  return c ;
}

/**
 * @}
 * 
 */
