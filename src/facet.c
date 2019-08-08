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

gboolean gtv_allow_floating_facets = FALSE ;

/**
 * @defgroup facet Facets
 * @{
 * 
 */

static void facet_destroy (GtsObject * object)
{
  GtvFacet *f = GTV_FACET(object) ;
  GSList *i ;
  /* do object-specific cleanup here */
  i = f->tetrahedra ;
  while (i) {
    GSList *next = i->next;
    gts_object_destroy(i->data) ;
    i = next ;
  }
  g_assert (f->tetrahedra == NULL) ;

  /* do not forget to call destroy method of the parent */
  (* GTS_OBJECT_CLASS (gtv_facet_class ())->parent_class->destroy) 
    (object);
}

static void gtv_facet_class_init (GtvFacetClass * klass)
{
  /* define new methods and overload inherited methods here */
  GTS_OBJECT_CLASS (klass)->destroy = facet_destroy;

}

static void gtv_facet_init (GtvFacet * object)
{
  GTV_FACET(object)->tetrahedra = NULL ;
}

GtvFacetClass * gtv_facet_class (void)
{
  static GtvFacetClass * klass = NULL;

  if (klass == NULL) {
    GtsObjectClassInfo gtv_facet_info = {
      "GtvFacet",
      sizeof (GtvFacet),
      sizeof (GtvFacetClass),
      (GtsObjectClassInitFunc) gtv_facet_class_init,
      (GtsObjectInitFunc) gtv_facet_init,
      (GtsArgSetFunc) NULL,
      (GtsArgGetFunc) NULL
    };
    klass = gts_object_class_new (GTS_OBJECT_CLASS (gts_triangle_class ()),
				  &gtv_facet_info);
  }

  return klass;
}

/** 
 * Make a new ::GtvFacet from three edges which must define a proper
 * triangle.
 * 
 * @param klass a ::GtvFacetClass;
 * @param e1 a ::GtsEdge; 
 * @param e2 a ::GtsEdge; 
 * @param e3 a ::GtsEdge.
 * 
 * @return a new ::GtvFacet or NULL if the edges do not define a valid
 * triangle.
 */

GtvFacet * gtv_facet_new (GtvFacetClass * klass,
			  GtsEdge *e1,
			  GtsEdge *e2,
			  GtsEdge *e3)

{
  GtvFacet * object;

  object = GTV_FACET (gts_object_new (GTS_OBJECT_CLASS (klass)));
  gts_triangle_set(GTS_TRIANGLE(object), e1, e2, e3) ;

  return object;
}

/** 
 * Check if a ::GtvFacet lies on the boundary of a ::GtvVolume.
 * 
 * @param f a ::GtvFacet;
 * @param v a ::GtvVolume or NULL.
 * 
 * @return a ::GtvCell which uses \a f and is on the boundary of \a v.
 */

GtvCell *gtv_facet_is_boundary(GtvFacet *f, GtvVolume *v)

{
  GSList *i ;
  GtvCell *c = NULL ;

  g_return_val_if_fail(f != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_FACET(f), NULL) ;

  i = f->tetrahedra ;
  while ( i ) {
    if ( GTV_IS_CELL(i->data) ) {
      if (!v || gtv_cell_has_parent_volume(i->data, v) ) {
	if ( c != NULL ) return NULL ;
	c = i->data ;
      }
    }
    i = i->next ;
  }

  return c ;
}

GtvFacet *gtv_facet_is_duplicate(GtvFacet *f)

{
  GSList *i ;
  GtsEdge *e2, *e3 ;

  g_return_val_if_fail(f != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_FACET(f), NULL) ;

  e2 = GTS_TRIANGLE(f)->e2 ;
  e3 = GTS_TRIANGLE(f)->e3 ;
  i =  GTS_TRIANGLE(f)->e1->triangles ;

  while ( i ) {
    GtvFacet *f1 = i->data ;
    if ( f1 != f && GTV_IS_FACET(f1) &&
	(GTS_TRIANGLE(f1)->e1 == e2 || 
	 GTS_TRIANGLE(f1)->e2 == e2 || 
	 GTS_TRIANGLE(f1)->e3 == e2 ) &&
	(GTS_TRIANGLE(f1)->e1 == e3 || 
	 GTS_TRIANGLE(f1)->e2 == e3 || 
	 GTS_TRIANGLE(f1)->e3 == e3 ) )
      return f1 ;
    i = i->next ;
  }

  return NULL ;
}

gint gtv_facet_replace(GtvFacet *f, GtvFacet *with)

{
  GSList *i ;

  g_return_val_if_fail(f != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(with != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(f != with, GTV_REPEATED_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_FACET(f), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(GTV_IS_FACET(with), GTV_WRONG_TYPE) ;

  i = f->tetrahedra ;

  while (i) {
    GtvTetrahedron *t = GTV_TETRAHEDRON(i->data) ;
    if ( t->f1 == f ) t->f1 = with ;
    if ( t->f2 == f ) t->f2 = with ;
    if ( t->f3 == f ) t->f3 = with ;
    if ( t->f4 == f ) t->f4 = with ;
    if (!g_slist_find (with->tetrahedra, t))
      with->tetrahedra = g_slist_prepend (with->tetrahedra, t);  
    i = i->next ;
  }
  g_slist_free(f->tetrahedra) ;

  f->tetrahedra = NULL ;

  return GTV_SUCCESS ;
}

/** 
 * Find the vertices of the tetrahedra sharing a particular facet. If
 * \a f is used by only one tetrahedron, v5 will be NULL. If \a f is
 * an isolated facet, i.e. not used by any tetrahedra, all the
 * vertices will be NULL.
 * 
 * @param f a ::GtvFacet
 * @param v1 a vertex of a tetrahedron using \a f;
 * @param v2 a vertex of a tetrahedron using \a f;
 * @param v3 a vertex of a tetrahedron using \a f;
 * @param v4 a vertex of a tetrahedron using \a f;
 * @param v5 a vertex of a tetrahedron using \a f.
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_facet_tetrahedra_vertices(GtvFacet *f, GtsVertex **v1,
				   GtsVertex **v2, GtsVertex **v3,
				   GtsVertex **v4, GtsVertex **v5)
     
{
  GSList *t ;
  GtvTetrahedron *t1, *t2 ;

  g_return_val_if_fail(f != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_FACET(f), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(v1 != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(v2 != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(v3 != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(v4 != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(v5 != NULL, GTV_NULL_ARGUMENT) ;

  *v1 = *v2 = *v3 = *v4 = *v5 = NULL ;

  gts_triangle_vertices(GTS_TRIANGLE(f), v1, v2, v3) ;

  t = f->tetrahedra ;

  if ( t == NULL ) return GTV_SUCCESS ;

  t1 = GTV_TETRAHEDRON(t->data) ;
  *v4 = gtv_tetrahedron_vertex_opposite(t1, f) ;

  if ( t->next == NULL ) return GTV_SUCCESS ;
  t2 = GTV_TETRAHEDRON(t->next->data) ;

  *v5 = gtv_tetrahedron_vertex_opposite(t2, f) ;

  return GTV_SUCCESS ;
}

/** 
 * Find the tetrahedra which use a ::GtvFacet. 
 * 
 * @param f a ::GtvFacet
 * @param t1 a ::GtvTetrahedron which uses \a f or NULL if \a f is isolated;
 * @param t2 a ::GtvTetrahedron which uses \a f or NULL if \a f is a 
 * boundary facet. 
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_facet_tetrahedra(GtvFacet *f, GtvTetrahedron **t1,
			  GtvTetrahedron **t2) 

{
  g_return_val_if_fail(f != NULL, GTV_NULL_ARGUMENT) ;
  g_assert(!GTS_OBJECT_DESTROYED(f)) ;

  g_return_val_if_fail(GTV_IS_FACET(f), GTV_WRONG_TYPE) ;

  g_return_val_if_fail(t1 != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(t2 != NULL, GTV_NULL_ARGUMENT) ;

  *t1 = *t2 = NULL ;

  if ( f->tetrahedra == NULL ) return GTV_SUCCESS ;

  *t1 = GTV_TETRAHEDRON(f->tetrahedra->data) ;

  if ( f->tetrahedra->next == NULL ) return GTV_SUCCESS ;

  *t2 = GTV_TETRAHEDRON(f->tetrahedra->next->data) ;

  return GTV_SUCCESS ;
}

/** 
 * Find a ::GtvFacet which uses three given ::GtsVertex's. 
 * 
 * @param v1 a ::GtsVertex;
 * @param v2 a ::GtsVertex;
 * @param v3 a ::GtsVertex.
 * 
 * @return a ::GtvFacet which uses \a v1, \a v2 and \a v3, if it
 * exists, NULL otherwise.
 */

GtvFacet *gtv_facet_from_vertices(GtsVertex *v1,
				  GtsVertex *v2,
				  GtsVertex *v3)

{
  GtvFacet *f ;

  f = GTV_FACET(triangle_from_vertices(v1, v2, v3)) ;
  if ( f == NULL ) return NULL ;

  if ( GTV_IS_FACET(f) ) return f ;
  return NULL ;
}

/** 
 * Check if a ::GtvFacet uses a ::GtsVertex.
 * 
 * @param f a ::GtvFacet;
 * @param v a ::GtsVertex;
 * 
 * @return TRUE if \a f use \a v, FALSE otherwise.
 */

gboolean gtv_facet_has_vertex(GtvFacet *f, GtsVertex *v)

{
  GtsVertex *v1, *v2, *v3 ;

  g_return_val_if_fail(f != NULL, FALSE) ;
  g_return_val_if_fail(GTV_IS_FACET(f), FALSE) ;  
  g_return_val_if_fail(v != NULL, FALSE) ;
  g_return_val_if_fail(GTS_IS_VERTEX(v), FALSE) ;

  gts_triangle_vertices(GTS_TRIANGLE(f), &v1, &v2, &v3) ;

  return ((v == v1) || (v == v2) || (v == v3)) ;
}

/** 
 * Check if a ::GtvFacet uses a ::GtsEdge.
 * 
 * @param f a ::GtvFacet;
 * @param e a ::GtsEdge;
 * 
 * @return TRUE if \a f use \a e, FALSE otherwise.
 */

gboolean gtv_facet_has_edge(GtvFacet *f, GtsEdge *e)

{
  g_return_val_if_fail(f != NULL, FALSE) ;
  g_return_val_if_fail(GTV_IS_FACET(f), FALSE) ;  
  g_return_val_if_fail(e != NULL, FALSE) ;
  g_return_val_if_fail(GTS_IS_EDGE(e), FALSE) ;

  return (GTS_TRIANGLE(f)->e1 == e ||
	  GTS_TRIANGLE(f)->e2 == e ||
	  GTS_TRIANGLE(f)->e3 == e ) ;
}

/** 
 * Check if a GtsEdge lies on the boundary of a ::GtvVolume.
 * 
 * @param e a ::GtsEdge;
 * @param v a ::GtvVolume or NULL.
 * 
 * @return a ::GtvCell which uses \a e and is on the boundary of \a v.
 */

GtvCell *gtv_edge_is_boundary(GtsEdge *e, GtvVolume *v)

{
  GSList *i ;
  GtvCell *c = NULL ;

  g_return_val_if_fail(e != NULL, NULL) ;
  g_return_val_if_fail(GTS_IS_EDGE(e), NULL) ;

  i = e->triangles ;
  while ( i ) {
    if ( GTV_IS_FACET(i->data) ) {
      if ( (c = gtv_facet_is_boundary(GTV_FACET(i->data), v)) != NULL ) 
	return c ;
    }
    i = i->next ;
  }

  return NULL ;
}


/** 
 * Check if a GtsVertex lies on the boundary of a ::GtvVolume.
 * 
 * @param p a ::GtsVertex;
 * @param v a ::GtvVolume or NULL.
 * 
 * @return a ::GtvCell which uses \a p and is on the boundary of \a v.
 */

GtvCell *gtv_vertex_is_boundary(GtsVertex *p, GtvVolume *v)

{
  GSList *i ;
  GtvCell *c = NULL ;

  g_return_val_if_fail(p != NULL, NULL) ;
  g_return_val_if_fail(GTS_IS_VERTEX(p), NULL) ;

  i = p->segments ;
  while ( i ) {
    if ( GTS_IS_EDGE(i->data) ) {
      if ( (c = gtv_edge_is_boundary(GTS_EDGE(i->data), v)) != NULL ) 
	return c ;
    }
    i = i->next ;
  }

  return NULL ;
}

/**
 * @}
 * 
 */
