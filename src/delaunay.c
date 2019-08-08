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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#include "gtv.h"
#include "gtv-private.h"

/**
 * @defgroup delaunay Delaunay tetrahedralization
 * @{
 * 
 */

static inline gboolean flippable23(GtvCell *tau, GtvCell *tau1)
     /*Ledoux's method of checking convexity of intersection*/
{
  GtsSegment s ;
  GtvFacet *f ;
  GtsVertex *a, *b ;

  g_debug("%s:", __FUNCTION__) ;

  g_assert( (f = gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(tau),
					     GTV_TETRAHEDRON(tau1)))
	    != NULL ) ;
  a = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau), f) ;
  b = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau1), f) ;
  s.v1 = a ; s.v2 = b ;
  return (gts_segment_triangle_intersection(&s, GTS_TRIANGLE(f),
					    FALSE, gts_point_class())
	  != NULL) ;
}

static gint foreach_cell_check(GtvCell *t, GtvCell **c)

{
  g_debug("%s: cell %p volume %lg", __FUNCTION__, t,
	  gtv_tetrahedron_volume(GTV_TETRAHEDRON(t))) ;

  if ( !gtv_facet_is_regular(GTV_TETRAHEDRON(t)->f1) ) {
    *c = t ; return GTV_SUCCESS ;
  }

  if ( !gtv_facet_is_regular(GTV_TETRAHEDRON(t)->f2) ) {
    *c = t ; return GTV_SUCCESS ;
  }

  if ( !gtv_facet_is_regular(GTV_TETRAHEDRON(t)->f3) ) {
    *c = t ; return GTV_SUCCESS ;
  }

  if ( !gtv_facet_is_regular(GTV_TETRAHEDRON(t)->f4) ) {
    *c = t ; return GTV_SUCCESS ;
  }

  return GTV_SUCCESS ;
}

/** 
 * Check whether a ::GtvVolume satisfies the Delaunay property. 
 * 
 * @param v volume to check
 * 
 * @return a non-Delaunay ::GtvCell of \a v if \a v is non-Delaunay,
 * NULL otherwise.
 */

GtvCell *gtv_delaunay_check(GtvVolume *v)

{
  GtvCell *c = NULL ;

  gtv_volume_foreach_cell(v, (GtsFunc)foreach_cell_check, &c) ;

  return c ;
}

static inline void edge_split(GtvCell *tau, GtsVertex *p, 
			      GtsEdge *e, GSList **cells,
			      GtvCellClass *cell_class,
			      GtvFacetClass *facet_class,
			      GtsEdgeClass *edge_class)

{
  GtvCell *new ;
  GtsVertex *a, *b, *c, *d ;

  g_debug("%s:", __FUNCTION__) ;

#ifdef GTV_DEVELOPER_DEBUG
  g_debug("%s: splitting edge %p on cell %p", __FUNCTION__, e, tau) ;
#endif /*GTV_DEVELOPER_DEBUG*/

  a = GTS_SEGMENT(e)->v1 ; c = GTS_SEGMENT(e)->v2 ;
  gtv_tetrahedron_vertices_off_edge(GTV_TETRAHEDRON(tau), e, &b, &d) ;
  g_assert(d != NULL) ;

  g_assert(a != b && a != c && a != d &&  b != c && b != d && c != d) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   b, a, d, p) ;
  *cells = g_slist_prepend(*cells, new) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   c, b, d, p) ;
  *cells = g_slist_prepend(*cells, new) ;
  
  return ;
}

static inline void flipcell12(GtvCell *tau, GtsVertex *p, GtsEdge *e,
			      GtvVolume *v,
			      GtvCellClass *cell_class,
			      GtvFacetClass *facet_class,
			      GtsEdgeClass *edge_class,
			      GSList **remove, GSList **cells)

{
  GSList *edge_cells, *j ;

  g_debug("%s:", __FUNCTION__) ;
  
  g_debug("%s: (%lg,%lg,%lg) splitting edge (%lg,%lg,%lg) (%lg,%lg,%lg)", 
	  __FUNCTION__, 
	  GTS_POINT(p)->x, GTS_POINT(p)->y, GTS_POINT(p)->z,
	  GTS_POINT(GTS_SEGMENT(e)->v1)->x, 
	  GTS_POINT(GTS_SEGMENT(e)->v1)->y, 
	  GTS_POINT(GTS_SEGMENT(e)->v1)->z, 
	  GTS_POINT(GTS_SEGMENT(e)->v2)->x, 
	  GTS_POINT(GTS_SEGMENT(e)->v2)->y, 
	  GTS_POINT(GTS_SEGMENT(e)->v2)->z) ;

  edge_cells = gtv_edge_cells(e, v) ;

#ifdef GTV_DEVELOPER_DEBUG
  g_debug("%s: edge has %d cells", __FUNCTION__, g_slist_length(edge_cells)) ;
#endif /*GTV_DEVELOPER_DEBUG*/

  for ( j = edge_cells ; j != NULL ; j = j->next ) {
    edge_split(GTV_CELL(j->data), p, e, cells,
	       cell_class, facet_class, edge_class) ;
    *remove = g_slist_prepend(*remove, j->data) ;
  }

  return ;
}

static inline void flipcell13(GtvCell *tau, GtsVertex *p,
			      GtvFacet *f, GtvVolume *v,
			      GtvCellClass *cell_class,
			      GtvFacetClass *facet_class,
			      GtsEdgeClass *edge_class,
			      GSList **remove, GSList **cells)

{
  GtvCell *tau1, *new ;
  GtsVertex *a, *b, *c, *d ;

  g_debug("%s:", __FUNCTION__) ;
  
  tau1 = GTV_CELL(gtv_tetrahedron_opposite(GTV_TETRAHEDRON(tau), f)) ;
  if ( tau1 != NULL ) 
    if ( v != NULL && !gtv_cell_has_parent_volume(tau1, v) ) tau1 = NULL ;
  gts_triangle_vertices(GTS_TRIANGLE(f), &b, &a, &c) ;

  g_debug("%s: (%lg,%lg,%lg) splitting facet (%lg,%lg,%lg) "
	  "(%lg,%lg,%lg) (%lg,%lg,%lg)", 
	  __FUNCTION__, 
	  GTS_POINT(p)->x, GTS_POINT(p)->y, GTS_POINT(p)->z,
	  GTS_POINT(a)->x, GTS_POINT(a)->y, GTS_POINT(a)->z,
	  GTS_POINT(b)->x, GTS_POINT(b)->y, GTS_POINT(b)->z,
	  GTS_POINT(c)->x, GTS_POINT(c)->y, GTS_POINT(c)->z) ;

  d = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau), f) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   a, b, d, p) ;
  *cells = g_slist_prepend(*cells, new) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   b, c, d, p) ;
  *cells = g_slist_prepend(*cells, new) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   c, d, a, p) ;
  *cells = g_slist_prepend(*cells, new) ;

  *remove = g_slist_prepend(*remove, tau) ;

  if ( tau1 == NULL ) return ;

  d = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau1), f) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   a, b, d, p) ;
  *cells = g_slist_prepend(*cells, new) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   b, c, d, p) ;
  *cells = g_slist_prepend(*cells, new) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   c, d, a, p) ;
  *cells = g_slist_prepend(*cells, new) ;
    
  *remove = g_slist_prepend(*remove, tau1) ;

  return ;
}

static inline void flipcell14(GtvCell *c, GtsVertex *v, 
			      GtvCellClass *cell_class,
			      GtvFacetClass *facet_class,
			      GtsEdgeClass *edge_class,
			      GSList **remove, GSList **cells)

{
  GtvCell *new ;
  GtsVertex *v1, *v2, *v3, *v4 ;

  g_debug("%s:", __FUNCTION__) ;
  gtv_tetrahedron_vertices(GTV_TETRAHEDRON(c), &v1, &v2, &v3, &v4) ;

  g_assert(g_slist_length(gts_vertex_triangles(v, NULL))==0) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				  v, v1, v3, v2) ;
  *cells = g_slist_prepend(*cells, new) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				  v, v2, v3, v4) ;
  *cells = g_slist_prepend(*cells, new) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				  v, v3, v1, v4) ;
  *cells = g_slist_prepend(*cells, new) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				  v, v4, v1, v2) ;
  *cells = g_slist_prepend(*cells, new) ;

  *remove = g_slist_prepend(*remove, c) ;

  return ;
}

static inline void flipcell23(GtvCell *tau, GtvCell *tau1,
			      GtvFacet *f,
			      GtvCellClass *cell_class,
			      GtvFacetClass *facet_class,
			      GtsEdgeClass *edge_class,
			      GSList **cells, GSList **remove)

{
  GtvCell *new ;
  GtsVertex *a, *b, *c, *d, *p ;

  g_debug("%s:", __FUNCTION__) ;
  g_assert(tau != NULL) ; g_assert(tau1 != NULL) ; g_assert(f != NULL) ;

  gts_triangle_vertices(GTS_TRIANGLE(f), &a, &b, &c) ;
  p = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau), f) ;
  d = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau1), f) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   d, a, p, b) ;
  *cells = g_slist_prepend(*cells, new) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   d, c, b, p) ;
  *cells = g_slist_prepend(*cells, new) ;			   

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   d, a, c, p) ;
  *cells = g_slist_prepend(*cells, new) ;
  
  *remove = g_slist_prepend(*remove, tau) ;
  *remove = g_slist_prepend(*remove, tau1) ;

  return ;
}

static inline void flipcell32(GtvCell *tau, GtvCell *tau1, GtvCell *tau2,
			      GtvFacet *f,
			      GtvCellClass *cell_class,
			      GtvFacetClass *facet_class,
			      GtsEdgeClass *edge_class,
			      GSList **cells, GSList **remove)

{
  GtvCell *new ;
  GtsVertex *a, *b, *c, *d, *p ;
  GtvFacet *g, *h ;
  gpointer swap ;
  gint i ;

  g_debug("%s:", __FUNCTION__) ;
  g_assert(tau != NULL) ; g_assert(tau1 != NULL) ; g_assert(f != NULL) ;

  g_assert(f == gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(tau),
					    GTV_TETRAHEDRON(tau1))) ;
  g_assert((g = gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(tau),
					    GTV_TETRAHEDRON(tau2))) != NULL) ;
  g_assert(gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(tau1),
				       GTV_TETRAHEDRON(tau2)) != NULL) ;

  gts_triangle_vertices(GTS_TRIANGLE(f), &a, &b, &c) ;

  p = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau), f) ;
  d = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau1), f) ;
  for ( i = 0 ; i < 3 ; i ++ ) {
    h = gtv_facet_from_vertices(a, b, p) ;
    if (g == h) break ;
    swap = a ; a = b ; b = c ; c = swap ;
  }
  g_assert(g == h) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   c, p, d, b) ;
  *cells = g_slist_prepend(*cells, new) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   p, c, d, a) ;
  *cells = g_slist_prepend(*cells, new) ;			   
  
  *remove = g_slist_prepend(*remove, tau) ;
  *remove = g_slist_prepend(*remove, tau1) ;
  *remove = g_slist_prepend(*remove, tau2) ;

  return ;
}

static inline void flipcell41(GtvCell *tau, GtvCell *tau1, 
			      GtvCell *tau2, GtvCell *tau3, 
			      GtsVertex *p,
			      GtvCellClass *cell_class,
			      GtvFacetClass *facet_class,
			      GtsEdgeClass *edge_class,
			      GSList **cells, GSList **remove) 
{
  GtsVertex *a, *b, *c, *d ;
  GtvFacet *f ;
  GtvCell *new ;

  g_debug("%s:", __FUNCTION__) ;

  g_assert( (f = gtv_tetrahedron_facet_opposite(GTV_TETRAHEDRON(tau),
						p)) != NULL) ;
  gts_triangle_vertices(GTS_TRIANGLE(f), &a, &b, &c) ;
  g_assert( (f = gtv_facet_from_vertices(a, b, p)) != NULL ) ;

  d = NULL ;
  if ( gtv_tetrahedron_has_facet(GTV_TETRAHEDRON(tau1), f) )
    d = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau1), f) ;
  if ( gtv_tetrahedron_has_facet(GTV_TETRAHEDRON(tau2), f) )
    d = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau2), f) ;
  if ( gtv_tetrahedron_has_facet(GTV_TETRAHEDRON(tau3), f) )
    d = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau3), f) ;

  g_assert(a != b && a != c && a != d && b != c && b != d && c != d) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   a, b, c, d) ;
  *cells = g_slist_prepend(*cells, new) ;

  *remove = g_slist_prepend(*remove, tau) ;
  *remove = g_slist_prepend(*remove, tau1) ;
  *remove = g_slist_prepend(*remove, tau2) ;
  *remove = g_slist_prepend(*remove, tau3) ;

  return ;
}

static inline void flipcell44(GtvCell *tau, GtvCell *tau1, 
			      GtvCell *tau2, GtvCell *tau3,
			      GtvCellClass *cell_class,
			      GtvFacetClass *facet_class,
			      GtsEdgeClass *edge_class,
			      GSList **cells, GSList **remove) 

{
  GtsVertex *a, *b, *c, *d, *e, *f ;
  GtvFacet *s, *t ;
  GtsEdge *ee ;
  GtvCell *new ;
  
  g_debug("%s:", __FUNCTION__) ;

  g_assert( (t = gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(tau),
					     GTV_TETRAHEDRON(tau1))) != NULL) ;
  b = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau), t) ;
  e = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau1), t) ;
  g_assert( (s = gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(tau),
					     GTV_TETRAHEDRON(tau3))) != NULL) ;
  g_assert( (ee = gts_triangles_common_edge(GTS_TRIANGLE(s),
					    GTS_TRIANGLE(t))) != NULL) ;
  c = GTS_SEGMENT(ee)->v1 ; d = GTS_SEGMENT(ee)->v2 ;
  a = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau),s) ;

  g_assert( (t = gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(tau1),
					     GTV_TETRAHEDRON(tau2))) != NULL) ;
  f = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau2), t) ;

#ifdef GTV_DEVELOPER_DEBUG
  t = gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(tau2), 
				  GTV_TETRAHEDRON(tau3)) ;
  g_assert(t != NULL) ;
  g_assert(b == gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau3), t)) ;
  g_debug("%s: orientation of %p %p %p %p is %lg", 
	  __FUNCTION__, b, c, d, e,
	  gts_point_orientation_3d(GTS_POINT(b), 
				   GTS_POINT(c), 
				   GTS_POINT(d), 
				   GTS_POINT(e))) ;
	  
  g_assert(gts_point_orientation_3d(GTS_POINT(b), 
				    GTS_POINT(c), 
				    GTS_POINT(d), 
				    GTS_POINT(e)) == 0.0) ;
#endif /*GTV_DEVELOPER_DEBUG*/

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   a, b, e, d) ;
  *cells = g_slist_prepend(*cells, new) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   a, b, e, c) ;
  *cells = g_slist_prepend(*cells, new) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   b, c, f, e) ;
  *cells = g_slist_prepend(*cells, new) ;

  new = gtv_cell_new_from_vertices(cell_class, facet_class, edge_class,
				   b, d, e, f) ;
  *cells = g_slist_prepend(*cells, new) ;

  *remove = g_slist_prepend(*remove, tau) ;
  *remove = g_slist_prepend(*remove, tau1) ;
  *remove = g_slist_prepend(*remove, tau2) ;
  *remove = g_slist_prepend(*remove, tau3) ;

  return ;
}

static inline void flipcells(GtvCell *tau, GtvCell *tau1,
			     GtvCellClass *cell_class,
			     GtvFacetClass *facet_class,
			     GtsEdgeClass *edge_class,
			     GSList **cells, GSList **remove)

{
  GtsVertex *a, *b, *c, *d, *p ;
  GtvFacet *f, *g, *h ;
  GtvCell *tau2, *tau3 ;
  gdouble orient ;
  gint i ;
  gpointer swap ;

  g_debug("%s:", __FUNCTION__) ;

  if ( (f = gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(tau),
					GTV_TETRAHEDRON(tau1))) == NULL )
    return ;

  tau2 = tau3 = NULL ; /*keeping the compiler happy*/
  p = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau), f) ;
  d = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau1), f) ;

  if ( flippable23(tau, tau1) ) {
    flipcell23(tau, tau1, f,
	       cell_class, facet_class, edge_class,
	       cells, remove) ;
    return ;
  }

  tau2 = GTV_CELL(gtv_tetrahedra_wedge_neighbour(GTV_TETRAHEDRON(tau),
						 GTV_TETRAHEDRON(tau1))) ;
  if ( tau2 != NULL ) {
    flipcell32(tau, tau1, tau2, f, cell_class, facet_class, edge_class,
	       cells, remove) ;
    return ;
  }

  gts_triangle_vertices(GTS_TRIANGLE(f), &a, &b, &c) ;
  for ( i = 0 ; i < 3 ; i ++ ) {
    if ( (orient = gts_point_orientation_3d(GTS_POINT(a), 
					    GTS_POINT(b), 
					    GTS_POINT(p), 
					    GTS_POINT(d))) == 0.0 ) {
      g_debug("%s: orientation of %p %p %p %p is %lg", 
	      __FUNCTION__, a, b, p, d, orient) ;
      if ( (g = gtv_facet_from_vertices(a, b, p)) != NULL) 
	tau3 = GTV_CELL(gtv_tetrahedron_opposite(GTV_TETRAHEDRON(tau), g)) ;
      if ( tau3 != NULL && (h = gtv_facet_from_vertices(a, b, d)) != NULL) 
	tau2 = GTV_CELL(gtv_tetrahedron_opposite(GTV_TETRAHEDRON(tau1), h)) ;
      
      if ( tau2 != NULL && tau3 != NULL &&
	   gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(tau2), 
				       GTV_TETRAHEDRON(tau3)) != NULL ) {
	flipcell44(tau, tau1, tau2, tau3, 
		   cell_class, facet_class, edge_class,
		   cells, remove) ;
	return ;
      }
    }
    swap = a ; a = b ; b = c ; c = swap ;
  }
    
  if ( gtv_tetrahedron_volume(GTV_TETRAHEDRON(tau)) == 0.0 ) {
    g_debug("%s: zero-volume tetrahedron", __FUNCTION__) ;
    flipcell23(tau, tau1, f,
	       cell_class, facet_class, edge_class,
	       cells, remove) ;
    return ;
  }

  g_debug("%s: no flip possible", __FUNCTION__) ;

  return ;
}

/** 
 * Add a ::GtsVertex to a ::GtvCell of a ::GtvVolume, restoring the
 * Delaunay property of the volume, using the method of Edelsbrunner,
 * H. and Shah, N. R., Algorithmica 15:223--241, 1996 and Lawson, C.,
 * Computer Aided Geometric Design, 3:231--246, 1986, as described in
 * Ledoux, H., <a
 * href="http://dx.doi.org/10.1109/ISVD.2007.10">`Computing the 3D
 * Voronoi diagram robustly: An easy explanation'</a>, 4th
 * International Symposium on Voronoi Diagrams in Science and
 * Engineering, 2007.
 * 
 * @param v a ::GtvVolume;
 * @param p a ::GtsVertex to add to \a p;
 * @param c a ::GtvCell to add \a p to.
 * 
 * @return ::GTV_SUCCESS on success, non-zero if \a p is already part
 * of \a c or coincides with a vertex of \a c.
 */

gint gtv_delaunay_add_vertex_to_cell(GtvVolume *v,
				     GtsVertex *p,
				     GtvCell *c)

{
  GSList *new, *remove, *cells, *j ;
  GtvCell *tau, *tau1 ;
  GtvFacet *f ;
  GtsEdge *e ;
  GtsVertex *d, *v1, *v2, *v3, *v4 ;
  GtvIntersect inter ;
  gpointer s ;
  GtvCellClass *cell_class ;
  GtvFacetClass *facet_class ;
  GtsEdgeClass *edge_class ;
  
  g_debug("%s: vertex %p (%lg, %lg, %lg)", __FUNCTION__, p, 
	  GTS_POINT(p)->x, GTS_POINT(p)->y, GTS_POINT(p)->z) ;

  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(p != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTS_IS_VERTEX(p), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(c != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_CELL(c), GTV_WRONG_TYPE) ;

  gtv_tetrahedron_vertices(GTV_TETRAHEDRON(c), &v1, &v2, &v3, &v4) ;
  if ( p == v1 || p == v2 || p == v3 || p == v4 ) 
    return GTV_VERTEX_PRESENT ;

  inter = gtv_point_in_tetrahedron(GTS_POINT(p), 
				   GTV_TETRAHEDRON(c), &s) ;
  g_debug("%s: point %s", __FUNCTION__, intersection_status(inter)) ;
  if ( inter == GTV_OUT ) {
    g_debug("%s: vertex is not in cell", __FUNCTION__) ;
    return GTV_VERTEX_NOT_IN_CELL ;
  }

  if ( inter == GTV_ON_VERTEX ) return GTV_COINCIDENT_VERTEX ;

  e = NULL ; f = NULL ;
  if ( inter == GTV_ON_FACET ) {
    f = GTV_FACET(s) ;
    if ( gtv_facet_is_boundary(f, v) ) {
      g_debug("%s: vertex %p (%lg,%lg,%lg) is on convex hull "
	      "facet %p", __FUNCTION__, p,
	      GTS_POINT(p)->x, GTS_POINT(p)->y, GTS_POINT(p)->z, f) ;
      return GTV_VERTEX_ON_HULL ;
    }
  }
  if ( inter == GTV_ON_EDGE )  {
    e = GTS_EDGE(s) ;
    if ( gtv_edge_is_boundary(e, v) ) {
      g_debug("%s: vertex %p (%lg,%lg,%lg) is on convex hull "
	      "edge %p", __FUNCTION__, p,
	      GTS_POINT(p)->x, GTS_POINT(p)->y, GTS_POINT(p)->z, e) ;
      return GTV_VERTEX_ON_HULL ;
    }
  }
 
  new = remove = cells = NULL ;

  cell_class = v->cell_class ;
  facet_class = v->facet_class ;
  edge_class = v->edge_class ;

  if ( inter == GTV_IN ) flipcell14(c, p, 
				    cell_class, facet_class, edge_class,
				    &remove, &cells) ;
  if ( inter == GTV_ON_FACET ) flipcell13(c, p, f, v, 
					  cell_class, facet_class, edge_class,
					  &remove, &cells) ;
  if ( inter == GTV_ON_EDGE ) flipcell12(c, p, e, v, 
					 cell_class, facet_class, edge_class,
					 &remove, &cells) ;

  for ( j = remove ; j != NULL ; j = j->next )
    gtv_volume_remove_cell(v, GTV_CELL(j->data)) ;
  g_slist_free(remove) ;

  for ( j = cells ; j != NULL ; j = j->next )
    gtv_volume_add_cell(v, GTV_CELL(j->data)) ;

  new = remove = NULL ;
  g_debug("%s: %d cells to check", __FUNCTION__, g_slist_length(cells)) ;
  while ( cells != NULL ) {
    tau = GTV_CELL(cells->data) ;
    while ( GTS_OBJECT_DESTROYED(GTS_OBJECT(tau)) ) {
      cells = g_slist_remove(cells, tau) ;
      if ( cells == NULL ) {
	g_debug("%s: non-regular cells exhausted: breaking", __FUNCTION__) ;
	break ;
      }
      tau = GTV_CELL(cells->data) ;
    }
    if ( cells == NULL ) {
      g_debug("%s: non-regular cells exhausted: breaking", __FUNCTION__) ;
      break ;
    }
    
    cells = g_slist_remove(cells, tau) ;
    f = gtv_tetrahedron_facet_opposite(GTV_TETRAHEDRON(tau), p) ;
    tau1 = GTV_CELL(gtv_tetrahedron_opposite(GTV_TETRAHEDRON(tau), f)) ;
    if ( tau1 != NULL ) {
      d = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(tau1), f) ;
      if ( gtv_point_in_tetrahedron_sphere(GTS_POINT(d),
					   GTV_TETRAHEDRON(tau)) > 0.0 ) {
	remove = NULL ; new = NULL ;
	flipcells(tau, tau1, cell_class, facet_class, edge_class,
		  &new, &remove) ;
	for ( j = remove ; j != NULL ; j = j->next ) {
	  gtv_volume_remove_cell(v, GTV_CELL(j->data)) ;
	  cells = g_slist_remove(cells, j->data) ;
	}
	for ( j = new ; j != NULL ; j = j->next )
	  gtv_volume_add_cell(v, GTV_CELL(j->data)) ;
	cells = g_slist_concat(new, cells) ;

	g_slist_free(remove) ;
      }
    }
  }

  g_debug("%s: cell list empty: returning", __FUNCTION__) ;
  return GTV_SUCCESS ;
}

/** 
 * Add a ::GtsVertex to a ::GtvVolume, preserving the Delaunay
 * property. 
 * 
 * @param v a ::GtvVolume;
 * @param p a ::GtsVertex to be added to \a v;
 * @param c a ::GtvCell which is an initial guess for the location of 
 * \a p, or NULL.
 * 
 * @return GTV_SUCCESS on insertion of \a p in \a v, or an appropriate
 * error code.
 */

gint gtv_delaunay_add_vertex(GtvVolume *v, GtsVertex *p, GtvCell *c)

{
  GtvCell *add ;

  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(p != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTS_IS_VERTEX(p), GTV_WRONG_TYPE) ;
  if ( c != NULL ) 
    g_return_val_if_fail(GTV_IS_CELL(c), GTV_WRONG_TYPE) ;

  add = gtv_point_locate(GTS_POINT(p), v, c) ;

  if ( add == NULL ) return GTV_VERTEX_NOT_IN_VOLUME ;

  return gtv_delaunay_add_vertex_to_cell(v, p, add) ;
}

/** 
 * Check if a ::GtvFacet is regular. A facet is regular if neither of
 * the tetrahedra using it has the apex of the opposite tetrahedron
 * inside its circumsphere. Boundary facets and isolated facets are
 * also considered regular.
 * 
 * @param f a ::GtvFacet.
 * 
 * @return TRUE if \a f is regular, FALSE otherwise.
 */

gboolean gtv_facet_is_regular(GtvFacet *f)

{
  GtvTetrahedron *t1, *t2 ;
  GtsVertex *v1, *v2 ;
  gdouble isp ;
  gint nt ;

  g_return_val_if_fail(f != NULL, FALSE) ;
  g_return_val_if_fail(GTV_IS_FACET(f), FALSE) ;

  nt = g_slist_length(f->tetrahedra) ;
  
  if ( nt == 1 ) {
    g_debug("%s: %p: one tetrahedron: facet regular", __FUNCTION__, f) ;
    return TRUE ;
  }
  if ( nt == 0 ) {
    g_debug("%s: %p: isolated facet", __FUNCTION__, f) ;
    return TRUE ;
  }

  t1 = GTV_TETRAHEDRON(f->tetrahedra->data) ;
  t2 = GTV_TETRAHEDRON(f->tetrahedra->next->data) ;

  g_debug("%s: %p tets: %p %p", __FUNCTION__, f, t1, t2) ;

  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t1), TRUE) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t2), TRUE) ;

  v1 = gtv_tetrahedron_vertex_opposite(t1, f) ;
  if ( (isp = gtv_point_in_tetrahedron_sphere(GTS_POINT(v1), t2)) > 0.0 ) {
    g_debug("%s: %p: isp = %lg; v1 (%p) lies inside " 
	    "t2 (%p) sphere: facet irregular", 
	    __FUNCTION__, f, isp, v1, t2) ;
    return FALSE ;
  }

  v2 = gtv_tetrahedron_vertex_opposite(t2, f) ;
  if ( (isp = gtv_point_in_tetrahedron_sphere(GTS_POINT(v2), t1)) > 0.0 ) {
    g_debug("%s: %p: isp = %lg; v2 (%p) lies inside "
	    "t1 (%p) sphere: facet irregular", 
	    __FUNCTION__, f, isp, v2, t1) ;
    return FALSE ;
  }

  g_debug("%s: %p: facet regular", __FUNCTION__, f) ;

  return TRUE ;
}

#if 0

static void find_next_ear(GSList *star, GSList *cells,
			  GtvCell **c1,
			  GtvCell **c2, GtvCell **c3)

{
  GSList *i, *j ;

  *c1 = *c2 = *c3 = NULL ;
  for ( i = cells ; i != NULL ; i = i->next ) {
    *c1 = GTV_CELL(i->data) ;
    for ( j = star ; j != NULL ; j = j->next ) {
      if ( i->data != j->data && 
	   gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(*c1),
				       GTV_TETRAHEDRON(j->data)) != NULL )
	{
	  *c2 = j->data ;
	  *c3 = 
	    GTV_CELL(gtv_tetrahedra_wedge_neighbour(GTV_TETRAHEDRON(*c1),
						    GTV_TETRAHEDRON(*c2))) ;
	  if ( (*c3) == NULL || g_slist_find(star, (*c3)) != NULL ) return ;
	  *c3 = NULL ;
	  return ;
	}	
    }
  }

  return ;
}

static GSList *tetrahedra_vertices(GSList *t, GSList *v)

{
  GSList *w, *i ;
  GtsVertex *v1, *v2, *v3, *v4 ;

  w = v ;
  for ( i = t ; i != NULL ; i = i->next ) {
    gtv_tetrahedron_vertices(GTV_TETRAHEDRON(i->data), &v1, &v2, &v3, &v4) ;
    if ( g_slist_find(w, v1) == NULL ) w = g_slist_prepend(w, v1) ;
    if ( g_slist_find(w, v2) == NULL ) w = g_slist_prepend(w, v2) ;
    if ( g_slist_find(w, v3) == NULL ) w = g_slist_prepend(w, v3) ;
    if ( g_slist_find(w, v4) == NULL ) w = g_slist_prepend(w, v4) ;
  }

  return w ;
}

static gboolean valid_2ear(GSList *star, 
			   GtvCell *t1, GtvCell *t2, 
			   GtsVertex *v, GtvFacet **f)

{
  GtsVertex *a, *b, *c, *d ;
  GtsEdge *e ;
  gdouble o1, o2 ;
  GSList *i ;

  g_debug("%s: ", __FUNCTION__) ;
  /*basic quantities (see Ledoux et al, Figure 4a*/
  g_assert( (*f = gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(t1), 
					      GTV_TETRAHEDRON(t2))) != NULL) ;
  g_assert( (e = gts_triangle_edge_opposite(GTS_TRIANGLE(*f), v)) != NULL) ;
  a = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(t1), *f) ;
  b = GTS_SEGMENT(e)->v1 ; c = GTS_SEGMENT(e)->v2 ;
  d = gtv_tetrahedron_vertex_opposite(GTV_TETRAHEDRON(t2), *f) ;

  /*for ear to be convex outwards, a and v must lie on the same side
    of bcd*/
  g_assert(gts_point_orientation_3d(GTS_POINT(a), GTS_POINT(b),
				    GTS_POINT(d), GTS_POINT(v)) != 0.0) ;
  g_assert(gts_point_orientation_3d(GTS_POINT(a), GTS_POINT(c),
				    GTS_POINT(d), GTS_POINT(v)) != 0.0) ;
  o1 = gts_point_orientation_3d(GTS_POINT(b), GTS_POINT(c),
				GTS_POINT(d), GTS_POINT(a)) ;
  o2 = gts_point_orientation_3d(GTS_POINT(b), GTS_POINT(c),
				GTS_POINT(d), GTS_POINT(v)) ;
  g_assert(o1 != 0.0 && o2 != 0.0) ;
  if ( ((o1 < 0.0) && (o2 > 0.0)) || ((o1 < 0.0) && (o2 > 0.0)) ) {
    g_debug("%s: invalid ear (%p,%p), o1=%lg, o2=%lg",
	    __FUNCTION__, t1, t2, o1, o2) ;
    return FALSE ;
  }

  /*check for flippability*/
  if ( !flippable23(t1, t2) ) {
    g_debug("%s: unflippable ear (%p,%p)", __FUNCTION__, t1, t2) ;
    return FALSE ;
  }

  /*check ear is locally Delaunay*/
  for ( i = tetrahedra_vertices(star, NULL) ; i != NULL ; i = i->next ) {
    if ( i->data != v && i->data != a && i->data != b && i->data != c &&
	 i->data != d ) {
      if ( gtv_point_in_sphere(GTS_POINT(i->data),
			       GTS_POINT(a), GTS_POINT(b),
			       GTS_POINT(c), GTS_POINT(d)) > 0.0) {
	g_debug("%s: ear (%p,%p) not locally Delaunay", 
		__FUNCTION__, t1, t2) ;
	return FALSE ;
      }			
    }
  }

  g_debug("%s: ear (%p,%p) valid, flippable and locally Delaunay", 
	  __FUNCTION__, t1, t2) ;
  
  return TRUE ;
}

static void ear_vertices(GtvFacet *f, GtvFacet *g, GtvFacet *h,
			 GtsVertex *v,
			 GtsVertex **a, GtsVertex **b, GtsVertex **c,
			 GtsVertex **d)

{
  GtsEdge *e ;

  e = gts_triangles_common_edge(GTS_TRIANGLE(f),GTS_TRIANGLE(g)) ;
  g_assert(gts_triangles_common_edge(GTS_TRIANGLE(f),GTS_TRIANGLE(h)) ==e);
  *a = *b = *c = *d = NULL ;

  if ( GTS_SEGMENT(e)->v1 == v ) *d = GTS_SEGMENT(e)->v2 ;
  if ( GTS_SEGMENT(e)->v2 == v ) *d = GTS_SEGMENT(e)->v1 ;

  *a = gts_triangle_vertex_opposite(GTS_TRIANGLE(f), e) ;
  *b = gts_triangle_vertex_opposite(GTS_TRIANGLE(g), e) ;
  *c = gts_triangle_vertex_opposite(GTS_TRIANGLE(h), e) ;

  g_assert(*a != NULL && *b != NULL && *c != NULL && *d != NULL) ;

  g_assert((*a != *b) && (*a != *c) && (*a != *d) &&
	   (*b != *c) && (*b != *d) && (*c != *d)) ;

  return ;
}

static gboolean valid_3ear(GSList *star, 
			   GtvCell *t1, GtvCell *t2, GtvCell *t3,
			   GtsVertex *v, GtvFacet **f)

{
  GtsVertex *a, *b, *c, *d ;
  GtvFacet *g, *h ;
  gdouble o1, o2 ;
  GSList *i ;

  g_debug("%s: ", __FUNCTION__) ;
  /*basic quantities (see Ledoux et al, Figure 4b*/
  g_assert( (*f = gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(t1), 
					      GTV_TETRAHEDRON(t2))) != NULL) ;
  g_assert( (g = gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(t2), 
					     GTV_TETRAHEDRON(t3))) != NULL) ;
  g_assert( (h = gtv_tetrahedra_common_facet(GTV_TETRAHEDRON(t3), 
					     GTV_TETRAHEDRON(t1))) != NULL) ;

  ear_vertices(*f, g, h, v, &a, &b, &c, &d) ;

  /*for ear to be convex outwards, d and v must lie on opposite sides
    of abc*/
  o1 = gts_point_orientation_3d(GTS_POINT(a), GTS_POINT(b),
				GTS_POINT(c), GTS_POINT(d)) ;
  o2 = gts_point_orientation_3d(GTS_POINT(a), GTS_POINT(b),
				GTS_POINT(c), GTS_POINT(v)) ;
  g_assert(o1 != 0.0 && o2 != 0.0) ;
  if ( ((o1 < 0.0) && (o2 < 0.0)) || ((o1 > 0.0) && (o2 > 0.0)) ) {
    g_debug("%s: invalid ear (%p,%p,%p), o1=%lg, o2=%lg",
	    __FUNCTION__, t1, t2, t3, o1, o2) ;
    return FALSE ;
  }

  /*check ear is locally Delaunay*/
  for ( i = tetrahedra_vertices(star, NULL) ; i != NULL ; i = i->next ) {
    if ( i->data != v && i->data != a && i->data != b && i->data != c &&
	 i->data != d ) {
      if ( gtv_point_in_sphere(GTS_POINT(i->data),
			       GTS_POINT(a), GTS_POINT(b),
			       GTS_POINT(c), GTS_POINT(d)) > 0.0) {
	g_debug("%s: ear (%p,%p,%p) not locally Delaunay", 
		__FUNCTION__, t1, t2, t3) ;
	return FALSE ;
      }			
    }
  }

  g_debug("%s: ear (%p,%p,%p) valid, flippable and locally Delaunay", 
	  __FUNCTION__, t1, t2, t3) ;
  
  return TRUE ;
}

static gboolean boundary_cell(GtvCell *c, GtvVolume *v)

{
  return (gtv_facet_is_boundary(GTV_TETRAHEDRON(c)->f1, v) ||
	  gtv_facet_is_boundary(GTV_TETRAHEDRON(c)->f2, v) ||
	  gtv_facet_is_boundary(GTV_TETRAHEDRON(c)->f3, v) ||
	  gtv_facet_is_boundary(GTV_TETRAHEDRON(c)->f4, v) ) ;
}

static void write_star(GSList *star)

{
  GtvVolume *v ;
  GSList *i ;

  v = gtv_volume_new(gtv_volume_class(),
		     gtv_cell_class(),
		     gtv_facet_class(),
		     gts_edge_class(),
		     gts_vertex_class()) ;

  for ( i = star ; i != NULL ; i = i->next ) 
    gtv_volume_add_cell(v, GTV_CELL(i->data)) ;

  gtv_volume_write(v, stdout) ;

  return ;
}

/** 
 * Remove a ::GtsVertex from a ::GtvVolume and restore the Delaunay
 * property, using the method of Ledoux, Hugo, Gold, Christopher and
 * Baciu, George, `Flipping to robustly delete a vertex in a Delaunay
 * tetrahedralization', LNCS 3480:737--747, 2005.
 * 
 * @param v a ::GtvVolume;
 * @param p a ::GtsVertex.
 * 
 * @return ::GTV_SUCCESS if \a p has been successfully removed.
 */

gint gtv_delaunay_remove_vertex(GtvVolume *v, GtsVertex *p)

{
  GtvCell *tau1, *tau2, *tau3 ;
  GSList *i, *star, *cells, *remove ;
  GtvFacet *f ;
  GtvCellClass *cell_class ;
  GtvFacetClass *facet_class ;
  GtsEdgeClass *edge_class ;
  gboolean valid ;
  gpointer s ;
  gint nc ;

  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;    
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;  
  g_return_val_if_fail(p != NULL, GTV_NULL_ARGUMENT) ;    
  g_return_val_if_fail(GTS_IS_VERTEX(p), GTV_WRONG_TYPE) ;  
  g_return_val_if_fail(gtv_vertex_has_parent_volume(p, v) != NULL, 
		       GTV_VERTEX_NOT_IN_VOLUME) ;

  g_debug("%s: ", __FUNCTION__) ;

  cell_class = v->cell_class ; facet_class = v->facet_class ;
  edge_class = v->edge_class ;

  star = gtv_vertex_cells(p, v) ; valid = TRUE ;
  while ( g_slist_length(star) > 4 && valid ) {
  for ( (nc = 0), (i = star) ; i != NULL ; i = i->next ) 
    if ( boundary_cell(GTV_CELL(i->data), v) ) nc ++ ;
  g_debug("%s: star(p) contains %d boundary cells", __FUNCTION__, nc) ;
      
    nc = g_slist_length(star) ;
    g_debug("%s: %d cells in star(p)", __FUNCTION__, nc) ;
    for ( (valid = FALSE), (i = star) ; (i != NULL) && !valid ; i = i->next ) {
      find_next_ear(star, i, &tau1, &tau2, &tau3) ;
      g_assert(tau2 != NULL) ;
      if ( tau3 == NULL ) valid = valid_2ear(star, tau1, tau2, p, &f) ;
      else valid = valid_3ear(star, tau1, tau2, tau3, p, &f) ;

/*       if ( tau2 != NULL ) { */
/* 	if ( tau3 == NULL ) valid = valid_2ear(star, tau1, tau2, p, &f) ; */
/* 	else { */
/* 	  if ( !(valid = valid_3ear(star, tau1, tau2, tau3, p, &f)) ) { */
/* 	    if ( (valid = valid_2ear(star, tau1, tau2, p, &f)) ) { */
/* 	      tau3 = NULL ; */
/* 	    } else { */
/* 	      if ( (valid = valid_2ear(star, tau2, tau3, p, &f)) ) { */
/* 		tau1 = tau3 ; tau3 = NULL ; */
/* 	      } else { */
/* 		if ( (valid = valid_2ear(star, tau3, tau1, p, &f)) ) { */
/* 		  tau2 = tau3 ; tau3 = NULL ; */
/* 		} */
/* 	      } */
/* 	    } */
/* 	  } */
/* 	} */
/*       } */

    }
    
    if ( valid ) {
      cells = remove = NULL ;
      if ( tau3 == NULL )
	flipcell23(tau1, tau2, f,
		   cell_class, facet_class, edge_class, &cells, &remove) ;
      else
	flipcell32(tau1, tau2, tau3, f,
		   cell_class, facet_class, edge_class, &cells, &remove) ;
      for ( i = remove ; i != NULL ; i = i->next  ) {
	gtv_volume_remove_cell(v, GTV_CELL(i->data)) ;
	star = g_slist_remove(star, i->data) ;
      }
      
      for ( i = cells ; i != NULL ; i = i->next ) {
	gtv_volume_add_cell(v, GTV_CELL(i->data)) ;
	if ( gtv_tetrahedron_has_vertex(GTV_TETRAHEDRON(i->data), p) )
	  star = g_slist_prepend(star, i->data) ;
      }
    }
/*     g_slist_free(star) ;  */
    g_slist_free(remove) ; g_slist_free(cells) ;
/*     star = gtv_vertex_cells(p, v) ; */
    star = g_slist_reverse(star) ;
  }

  g_debug("%s: %d cells in star(p)", __FUNCTION__, g_slist_length(star)) ;

  if ( g_slist_length(star) == 4 ) {
    cells = remove = NULL ;
    flipcell41(star->data, star->next->data, star->next->next->data, 
	       star->next->next->next->data, p,
	       cell_class, facet_class, edge_class, &cells, &remove) ;

    for ( i = remove ; i != NULL ; i = i->next  )
      gtv_volume_remove_cell(v, GTV_CELL(i->data)) ;
    
    for ( i = cells ; i != NULL ; i = i->next )
      gtv_volume_add_cell(v, GTV_CELL(i->data)) ;
    return GTV_SUCCESS ;
  }

  write_star(star) ;

  return GTV_SUCCESS ;
}

#endif

/**
 * @}
 * 
 */

