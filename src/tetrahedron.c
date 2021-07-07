/* GTV - Library for the manipulation of tetrahedralized volumes
 *
 * Copyright (C) 2007, 2008, 2021 Michael Carley
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#include <math.h>
#include <stdlib.h>

#include <gts.h>

#include "gtv.h"
#include "gtv-private.h"

#include "predicates.h"

/* #ifdef HAVE_VERDICT_H */
/* #include <verdict.h> */
/* #endif /\*HAVE_VERDICT_H*\/ */

/**
 * @defgroup tetrahedron Tetrahedra
 * @{
 * 
 */

static void tetrahedron_destroy (GtsObject * object)
{
  GtvTetrahedron * tetrahedron = GTV_TETRAHEDRON (object);
  GtvFacet *f1 = tetrahedron->f1 ;
  GtvFacet *f2 = tetrahedron->f2 ;
  GtvFacet *f3 = tetrahedron->f3 ;
  GtvFacet *f4 = tetrahedron->f4 ;

  f1->tetrahedra = g_slist_remove (f1->tetrahedra, tetrahedron);
  if (!GTS_OBJECT_DESTROYED (f1) &&
      !gtv_allow_floating_facets && f1->tetrahedra == NULL)
    gts_object_destroy (GTS_OBJECT (f1));
  
  f2->tetrahedra = g_slist_remove (f2->tetrahedra, tetrahedron);
  if (!GTS_OBJECT_DESTROYED (f2) &&
      !gtv_allow_floating_facets && f2->tetrahedra == NULL)
    gts_object_destroy (GTS_OBJECT (f2));
  
  f3->tetrahedra = g_slist_remove (f3->tetrahedra, tetrahedron);
  if (!GTS_OBJECT_DESTROYED (f3) &&
      !gtv_allow_floating_facets && f3->tetrahedra == NULL)
    gts_object_destroy (GTS_OBJECT (f3));
  
  f4->tetrahedra = g_slist_remove (f4->tetrahedra, tetrahedron);
  if (!GTS_OBJECT_DESTROYED (f4) &&
      !gtv_allow_floating_facets && f4->tetrahedra == NULL)
    gts_object_destroy (GTS_OBJECT (f4));
  
  (* GTS_OBJECT_CLASS (gtv_tetrahedron_class ())->parent_class->destroy) 
    (object);
}

static void tetrahedron_class_init (GtsObjectClass * klass)
{
  GTS_OBJECT_CLASS(klass)->destroy = tetrahedron_destroy;
}

static void tetrahedron_init (GtvTetrahedron *tetrahedron)
{
  tetrahedron->f1 = tetrahedron->f2 = tetrahedron->f3 = 
    tetrahedron->f4 = NULL ;
}

/** 
 * The basic class for tetrahedra in GTV.
 * 
 * @return the ::GtvTetrahedronClass
 */

GtvTetrahedronClass * gtv_tetrahedron_class (void)

{
  static GtvTetrahedronClass * klass = NULL;

  if (klass == NULL) {
    GtsObjectClassInfo tetrahedron_info = {
      "GtvTetrahedron",
      sizeof (GtvTetrahedron),
      sizeof (GtvTetrahedronClass),
      (GtsObjectClassInitFunc) tetrahedron_class_init,
      (GtsObjectInitFunc) tetrahedron_init,
      (GtsArgSetFunc) NULL,
      (GtsArgGetFunc) NULL
    };
    klass = gts_object_class_new (gts_object_class (), 
				  &tetrahedron_info);
  }

  return klass;
}

/**
 * Set the facets of a ::GtvTetrahedron. A check is performed to
 * ensure that the facets define a valid tetrahedron.
 *
 * @param tetrahedron: a ::GtvTetrahedron;
 * @param f1: a ::GtvFacet;
 * @param f2: a ::GtvFacet;
 * @param f3: a ::GtvFacet;
 * @param f4: a ::GtvFacet.
 *
 * @return GTV_SUCCESS on success.
 */

gint gtv_tetrahedron_set (GtvTetrahedron *tetrahedron,
			  GtvFacet *f1,
			  GtvFacet *f2,
			  GtvFacet *f3,
			  GtvFacet *f4)
{
  GtsEdge *e1, *e2, *e3 ;

  g_return_val_if_fail(tetrahedron != NULL, GTV_NULL_ARGUMENT);
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(tetrahedron), 
		       GTV_WRONG_TYPE) ;
  g_return_val_if_fail(f1 != NULL, GTV_NULL_ARGUMENT);
  g_return_val_if_fail(GTV_IS_FACET(f1), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(f2 != NULL, GTV_NULL_ARGUMENT);
  g_return_val_if_fail(GTV_IS_FACET(f2), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(f3 != NULL, GTV_NULL_ARGUMENT);
  g_return_val_if_fail(GTV_IS_FACET(f3), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(f4 != NULL, GTV_NULL_ARGUMENT);
  g_return_val_if_fail(GTV_IS_FACET(f4), GTV_WRONG_TYPE) ;

  g_return_val_if_fail(f1 != f2 && f2 != f3 && f3 != f4 && 
			f4 != f1 && f4 != f2 && f1 != f3, 
			GTV_REPEATED_ARGUMENT);

  /*check that these faces define a valid tetrahedron*/
  e1 = gts_triangles_common_edge(GTS_TRIANGLE(f1),GTS_TRIANGLE(f2)) ;
  e2 = gts_triangles_common_edge(GTS_TRIANGLE(f1),GTS_TRIANGLE(f3)) ;
  e3 = gts_triangles_common_edge(GTS_TRIANGLE(f1),GTS_TRIANGLE(f4)) ;
  g_return_val_if_fail((e1 != NULL) && (e2 != NULL) && (e3 != NULL),
		       GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail((e1 != e2) && (e2 != e3) && (e3 != e1),
		       GTV_REPEATED_ARGUMENT) ;

  e1 = gts_triangles_common_edge(GTS_TRIANGLE(f2),GTS_TRIANGLE(f3)) ;
  e2 = gts_triangles_common_edge(GTS_TRIANGLE(f3),GTS_TRIANGLE(f4)) ;
  e3 = gts_triangles_common_edge(GTS_TRIANGLE(f4),GTS_TRIANGLE(f2)) ;
  g_return_val_if_fail((e1 != NULL) && (e2 != NULL) && (e3 != NULL),
		       GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail((e1 != e2) && (e2 != e3) && (e3 != e1),
		       GTV_NULL_ARGUMENT) ;  

  tetrahedron->f1 = f1 ; 
  tetrahedron->f2 = f2 ;
  tetrahedron->f3 = f3 ;
  tetrahedron->f4 = f4 ;

  f1->tetrahedra = g_slist_prepend(f1->tetrahedra, tetrahedron) ;
  f2->tetrahedra = g_slist_prepend(f2->tetrahedra, tetrahedron) ;
  f3->tetrahedra = g_slist_prepend(f3->tetrahedra, tetrahedron) ;
  f4->tetrahedra = g_slist_prepend(f4->tetrahedra, tetrahedron) ;

  return GTV_SUCCESS ;
}

/** 
 * Generate a new tetrahedron from four ::GtvFacet's
 * 
 * @param klass a ::GtvTetrahedronClass
 * @param f1 ::GtvFacet
 * @param f2 ::GtvFacet
 * @param f3 ::GtvFacet
 * @param f4 ::GtvFacet
 * 
 * @return the new ::GtvTetrahedron
 */

GtvTetrahedron *gtv_tetrahedron_new(GtvTetrahedronClass *klass,
				    GtvFacet *f1,
				    GtvFacet *f2,
				    GtvFacet *f3,
				    GtvFacet *f4)

{
  GtvTetrahedron *t ;

  g_return_val_if_fail(klass != NULL, NULL) ;
  g_return_val_if_fail(klass == 
		       gts_object_class_is_from_class(klass, 
						      gtv_tetrahedron_class()),
		       NULL) ;
  g_return_val_if_fail(f1 != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_FACET(f1), NULL) ;
  g_return_val_if_fail(f2 != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_FACET(f2), NULL) ;
  g_return_val_if_fail(f3 != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_FACET(f3), NULL) ;
  g_return_val_if_fail(f4 != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_FACET(f4), NULL) ;

  t = GTV_TETRAHEDRON (gts_object_new (GTS_OBJECT_CLASS (klass)));
  gtv_tetrahedron_set (t, f1, f2, f3, f4) ;
  
  return t;
}

static inline GtsVertex *triangle_opposite_vertex(GtsTriangle *t,
						  GtsVertex *v1,
						  GtsVertex *v2,
						  GtsVertex *v3)

{
  GtsVertex *w1, *w2, *w3 ;

  gts_triangle_vertices(t, &w1, &w2, &w3) ;
  if ( w1 != v1 && w1 != v2 && w1 != v3 ) return w1 ;
  if ( w2 != v1 && w2 != v2 && w2 != v3 ) return w2 ;
  if ( w3 != v1 && w3 != v2 && w3 != v3 ) return w3 ;

  return NULL ;
}

/** 
 * Extract the vertices of a tetrahedron. These are ordered so that \a
 * v1 is opposite the first face of \a t (t->f1) and so on.
 * 
 * @param t a ::GtvTetrahedron;
 * @param v1 a ::GtsVertex;
 * @param v2 a ::GtsVertex;
 * @param v3 a ::GtsVertex;
 * @param v4 a ::GtsVertex.
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_tetrahedron_vertices(GtvTetrahedron *t,
			      GtsVertex **v1,
			      GtsVertex **v2,
			      GtsVertex **v3,
			      GtsVertex **v4)

{
  GtsSegment *e12, *e13, *e23, *e24, *e34 ;

  g_return_val_if_fail(t != NULL, GTV_NULL_ARGUMENT);
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(v1 != NULL && v2 != NULL && v3 != NULL && v4 != NULL,
		       GTV_NULL_ARGUMENT);

  e12 = GTS_SEGMENT(gts_triangles_common_edge(GTS_TRIANGLE(t->f1),
					      GTS_TRIANGLE(t->f2))) ;
  e13 = GTS_SEGMENT(gts_triangles_common_edge(GTS_TRIANGLE(t->f1),
					      GTS_TRIANGLE(t->f3))) ;
  e23 = GTS_SEGMENT(gts_triangles_common_edge(GTS_TRIANGLE(t->f2),
					      GTS_TRIANGLE(t->f3))) ;
  e24 = GTS_SEGMENT(gts_triangles_common_edge(GTS_TRIANGLE(t->f2),
					      GTS_TRIANGLE(t->f4))) ;
  e34 = GTS_SEGMENT(gts_triangles_common_edge(GTS_TRIANGLE(t->f3),
					      GTS_TRIANGLE(t->f4))) ;

  g_assert(e12 != NULL && e13 != NULL && e23 != NULL && 
	   e24 != NULL && e34 != NULL) ;
  
  if ( e34->v1 == e24->v1 || e34->v1 == e24->v2 ) *v1 = e34->v1 ;
  else *v1 = e34->v2 ;

  if ( e34->v1 == e13->v1 || e34->v1 == e13->v2 ) *v2 = e34->v1 ;
  else *v2 = e34->v2 ;

  if ( e24->v1 == e12->v1 || e24->v1 == e12->v2 ) *v3 = e24->v1 ;
  else *v3 = e24->v2 ;

  if ( e23->v1 == e13->v1 || e23->v1 == e13->v2 ) *v4 = e23->v1 ;
  else *v4 = e23->v2 ;

  return GTV_SUCCESS ;
}

/** 
 * Find the signed volume of a tetrahedron.
 * 
 * @param t a ::GtvTetrahedron.
 * 
 * @return the signed volume of \a t; if this is negative, you might
 * want to use ::gtv_tetrahedron_invert or ::gtv_tetrahedron_orient to
 * reorient the vertices.
 */
			      
gdouble gtv_tetrahedron_volume(GtvTetrahedron *t)

{
  gdouble V ;
  GtsVertex *v1, *v2, *v3, *v4 ;

  g_return_val_if_fail(t != NULL, 0.0) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), 0.0);

  gtv_tetrahedron_vertices(t, &v1, &v2, &v3, &v4) ;

  V = gts_point_orientation_3d(GTS_POINT(v1),
			       GTS_POINT(v2),
			       GTS_POINT(v3),
			       GTS_POINT(v4))/6.0 ;

  return V ;
}

/** 
 * Check if a tetrahedron has a given facet.
 * 
 * @param t a ::GtvTetrahedron;
 * @param f a ::GtvFacet.
 * 
 * @return TRUE if \a t contains \a f, FALSE otherwise.
 */

gboolean gtv_tetrahedron_has_facet(GtvTetrahedron *t,
				   GtvFacet *f)

{
  g_return_val_if_fail(t != NULL, FALSE);
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), FALSE) ;
  g_return_val_if_fail(f != NULL, FALSE);
  g_return_val_if_fail(GTV_IS_FACET(f), FALSE);

  return ( (t->f1 == f) || (t->f2 == f) ||
	   (t->f3 == f) || (t->f4 == f) ) ;
}

/** 
 * Find a tetrahedron which uses a set of facets.
 * 
 * @param f1 a ::GtvFacet;
 * @param f2 a ::GtvFacet;
 * @param f3 a ::GtvFacet;
 * @param f4 a ::GtvFacet.
 * 
 * @return a ::GtvTetrahedron which uses \a f1, \a f2, \a f3 and \a
 * f4, if one exists, NULL otherwise.
 */

GtvTetrahedron *gtv_tetrahedron_from_facets(GtvFacet *f1,
					    GtvFacet *f2,
					    GtvFacet *f3,
					    GtvFacet *f4)

{
  GSList *i ;

  g_return_val_if_fail(f1 != NULL, NULL);
  g_return_val_if_fail(GTV_IS_FACET(f1), NULL);
  g_return_val_if_fail(f2 != NULL, NULL);
  g_return_val_if_fail(GTV_IS_FACET(f2), NULL);
  g_return_val_if_fail(f3 != NULL, NULL);
  g_return_val_if_fail(GTV_IS_FACET(f3), NULL);
  g_return_val_if_fail(f4 != NULL, NULL);
  g_return_val_if_fail(GTV_IS_FACET(f4), NULL);

  i = f1->tetrahedra ;

  while ( i ) {
    GtvTetrahedron *t = i->data ;
    if ( gtv_tetrahedron_has_facet(t, f2) &&
	 gtv_tetrahedron_has_facet(t, f3) &&
	 gtv_tetrahedron_has_facet(t, f4) )
      return t ;
    i = i->next ;
  }

  return NULL ;
}

/** 
 * Check if a tetrahedron is duplicated.
 * 
 * @param t a ::GtvTetrahedron;
 * 
 * @return NULL if \a t is unique, otherwise the ::GtvTetrahedron
 * which duplicates \a t.
 */

GtvTetrahedron *gtv_tetrahedron_is_duplicate(GtvTetrahedron *t)

{
  GSList *i ;
  GtvFacet *f2, *f3, *f4 ;

  g_return_val_if_fail(t != NULL, NULL) ;

  f2 = t->f2 ;
  f3 = t->f3 ;
  f4 = t->f4 ;
  i = t->f1->tetrahedra ;
  while ( i ) {
    GtvTetrahedron *t1 = i->data ;
    if ( t1 != t &&
	 (t1->f1 == f2 || t1->f2 == f2 || t1->f3 == f2 || t1->f4 == f2) &&
	 (t1->f1 == f3 || t1->f2 == f3 || t1->f3 == f3 || t1->f4 == f3) &&
	 (t1->f1 == f4 || t1->f2 == f4 || t1->f3 == f4 || t1->f4 == f4) )
      return t1 ;
    i = i->next ;
  }

  return NULL ;
}

/** 
 * Check whether a point lies inside or outside the circumsphere of a
 * tetrahedron.
 * 
 * @param p a ::GtsPoint;
 * @param t a ::GtvTetrahedron.
 * 
 * @return positive value if \a p lies inside \a t, zero if lies on \a
 * t and a negative value if it lies outside \a t. 
 */

gdouble gtv_point_in_tetrahedron_sphere(GtsPoint *p, GtvTetrahedron *t)

{
  GtsVertex *p1, *p2, *p3, *p4 ;

  g_return_val_if_fail(p != NULL, 0.0) ;
  g_return_val_if_fail(GTS_IS_POINT(p), 0.0) ;
  g_return_val_if_fail(t != NULL, 0.0) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), 0.0) ;

  gtv_tetrahedron_vertices(t, &p1, &p2, &p3, &p4) ;

  return gtv_point_in_sphere(p, 
			     GTS_POINT(p1), 
			     GTS_POINT(p2), 
			     GTS_POINT(p3), 
			     GTS_POINT(p4)) ;
}

/** 
 * Check if a ::GtsPoint lies in a ::GtvTetrahedron, using the method
 * given by <a
 * href="http://steve.hollasch.net/cgindex/geometry/ptintet.html">Gary
 * Herron</a>.
 * 
 * @param p a ::GtsPoint;
 * @param t a ::GtvTetrahedron;
 * @param s if NULL, ignored; if not NULL, set to the vertex, edge or 
 * facet of \a t which \a p intersects, or NULL if \a p lies strictly 
 * inside or strictly outside \a t.
 * 
 * @return GTV_OUT if \a p is strictly outside \a t and GTV_IN if it
 * lies strictly inside \a t. If \a p lies on the surface of \a t,
 * returns GTV_IN if \a s is NULL, otherwise GTV_ON_FACET, GTV_ON_EDGE
 * or GTV_ON_VERTEX if it lies on a facet, edge or vertex of \a t
 * respectively.
 */

GtvIntersect gtv_point_in_tetrahedron(GtsPoint *p, 
				      GtvTetrahedron *t,
				      gpointer *s)

{
  gdouble D, D1, D2, D3, D4, sgn ;
  gdouble xmin, xmax, ymin, ymax, zmin, zmax ;
  GtsVertex *v1, *v2, *v3, *v4 ;

  g_return_val_if_fail(p != NULL, GTV_OUT) ;
  g_return_val_if_fail(GTS_IS_POINT(p), GTV_OUT) ;
  g_return_val_if_fail(t != NULL, GTV_OUT) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), GTV_OUT) ;

  if ( s != NULL ) *s = NULL ;

  gtv_tetrahedron_vertices(t, &v1, &v2, &v3, &v4) ;
  xmin = MIN(GTS_POINT(v1)->x,
	     MIN(GTS_POINT(v2)->x,
		 MIN(GTS_POINT(v3)->x,GTS_POINT(v4)->x))) ;
  xmax = MAX(GTS_POINT(v1)->x,
	     MAX(GTS_POINT(v2)->x,
		 MAX(GTS_POINT(v3)->x,GTS_POINT(v4)->x))) ;
  ymin = MIN(GTS_POINT(v1)->y,
	     MIN(GTS_POINT(v2)->y,
		 MIN(GTS_POINT(v3)->y,GTS_POINT(v4)->y))) ;
  ymax = MAX(GTS_POINT(v1)->y,
	     MAX(GTS_POINT(v2)->y,
		 MAX(GTS_POINT(v3)->y,GTS_POINT(v4)->y))) ;
  zmin = MIN(GTS_POINT(v1)->z,
	     MIN(GTS_POINT(v2)->z,
		 MIN(GTS_POINT(v3)->z,GTS_POINT(v4)->z))) ;
  zmax = MAX(GTS_POINT(v1)->z,
	     MAX(GTS_POINT(v2)->z,
		 MAX(GTS_POINT(v3)->z,GTS_POINT(v4)->z))) ;
  if ( (p->x < xmin) || (p->x > xmax) ||
       (p->y < ymin) || (p->y > ymax) ||
       (p->z < zmin) || (p->z > zmax) ) return GTV_OUT ;

  D = gts_point_orientation_3d(GTS_POINT(v1),
			       GTS_POINT(v2),
			       GTS_POINT(v3),
			       GTS_POINT(v4)) ;
  /*degenerate tetrahedron*/
  if ( D == 0.0 ) {
    g_debug("%s: degenerate tetrahedron", __FUNCTION__) ;
    return GTV_OUT ;
  } 

  sgn = 1.0 ;
  if ( D < 0.0 ) { D = -D ; sgn = -1.0 ; }

  D1 = sgn*gts_point_orientation_3d(GTS_POINT(p), GTS_POINT(v2),
				    GTS_POINT(v3), GTS_POINT(v4))  ;
  if ( D1 < 0.0 ) return GTV_OUT ;

  D2 = sgn*gts_point_orientation_3d(GTS_POINT(v1), GTS_POINT(p),
				    GTS_POINT(v3), GTS_POINT(v4)) ;
  if ( D2 < 0.0 ) return GTV_OUT ;

  D3 = sgn*gts_point_orientation_3d(GTS_POINT(v1), GTS_POINT(v2),
				    GTS_POINT(p), GTS_POINT(v4)) ;
  if ( D3 < 0.0 ) return GTV_OUT ;

  D4 = sgn*gts_point_orientation_3d(GTS_POINT(v1), GTS_POINT(v2),
				    GTS_POINT(v3), GTS_POINT(p)) ;
  if ( D4 < 0.0 ) return GTV_OUT ;

#ifdef GTV_DEVELOPER_DEBUG
  g_debug("%s: (%lg,%lg,%lg) in (%lg,%lg,%lg) (%lg,%lg,%lg) "
	  "(%lg,%lg,%lg) (%lg,%lg,%lg)", 
	  __FUNCTION__,
	  GTS_POINT(p)->x, GTS_POINT(p)->y, GTS_POINT(p)->z,	  
	  GTS_POINT(v1)->x, GTS_POINT(v1)->y, GTS_POINT(v1)->z,
	  GTS_POINT(v2)->x, GTS_POINT(v2)->y, GTS_POINT(v2)->z,
	  GTS_POINT(v3)->x, GTS_POINT(v3)->y, GTS_POINT(v3)->z,
	  GTS_POINT(v4)->x, GTS_POINT(v4)->y, GTS_POINT(v4)->z) ;
  g_debug("%s: D: %lg %lg %lg %lg %lg %1.16e", 
	  __FUNCTION__, D, D1, D2, D3, D4, D-D1-D2-D3-D4) ;
#endif /*GTV_DEVELOPER_DEBUG*/

  if ( D1 > 0.0 && D2 > 0.0 && D3 > 0.0 && D4 > 0.0 ) return GTV_IN ;

  if ( s == NULL ) return GTV_ON ;

  /*facet intersections*/
  if ( D1 == 0.0 && D2 != 0.0 && D3 != 0.0 && D4 != 0.0 ) {
    *s = t->f1 ; return GTV_ON_FACET ;
  }
  if ( D1 != 0.0 && D2 == 0.0 && D3 != 0.0 && D4 != 0.0 ) {
    *s = t->f2 ; return GTV_ON_FACET ;
  }
  if ( D1 != 0.0 && D2 != 0.0 && D3 == 0.0 && D4 != 0.0 ) {
    *s = t->f3 ; return GTV_ON_FACET ;
  }
  if ( D1 != 0.0 && D2 != 0.0 && D3 != 0.0 && D4 == 0.0 ) {
    *s = t->f4 ; return GTV_ON_FACET ;
  }

  /*edge intersections*/
  if ( D1 != 0.0 && D2 != 0.0 && D3 == 0.0 && D4 == 0.0 ) {
    g_assert((*s = gts_vertices_are_connected(v1, v2)) != NULL) ; 
    return GTV_ON_EDGE ;
  }
  if ( D1 != 0.0 && D2 == 0.0 && D3 != 0.0 && D4 == 0.0 ) {
    g_assert((*s = gts_vertices_are_connected(v1, v3)) != NULL) ; 
    return GTV_ON_EDGE ;
  }
  if ( D1 != 0.0 && D2 == 0.0 && D3 == 0.0 && D4 != 0.0 ) {
    g_assert((*s = gts_vertices_are_connected(v1, v4)) != NULL) ; 
    return GTV_ON_EDGE ;
  }
  if ( D1 == 0.0 && D2 != 0.0 && D3 != 0.0 && D4 == 0.0 ) {
    g_assert((*s = gts_vertices_are_connected(v2, v3)) != NULL) ; 
    return GTV_ON_EDGE ;
  }
  if ( D1 == 0.0 && D2 != 0.0 && D3 == 0.0 && D4 != 0.0 ) {
    g_assert((*s = gts_vertices_are_connected(v2, v4)) != NULL) ; 
    return GTV_ON_EDGE ;
  }
  if ( D1 == 0.0 && D2 == 0.0 && D3 != 0.0 && D4 != 0.0 ) {
    g_assert((*s = gts_vertices_are_connected(v3, v4)) != NULL) ; 
    return GTV_ON_EDGE ;
  }

  /*on a vertex*/
  if ( D1 != 0.0 ) { *s = v1 ; return GTV_ON_VERTEX ; }
  if ( D2 != 0.0 ) { *s = v2 ; return GTV_ON_VERTEX ; }
  if ( D3 != 0.0 ) { *s = v3 ; return GTV_ON_VERTEX ; }
  if ( D4 != 0.0 ) { *s = v4 ; return GTV_ON_VERTEX ; }

  g_error("%s: geometrically impossible situation "
	  "tetrahedron: (%lg,%lg,%lg), (%lg,%lg,%lg), "
	  "(%lg,%lg,%lg), (%lg,%lg,%lg);\n"
	  "point: (%lg,%lg,%lg)\n"
	  "D: %lg, %lg, %lg, %lg, %lg; sgn: %lg", 
	  __FUNCTION__,
	  GTS_POINT(v1)->x, GTS_POINT(v1)->y, GTS_POINT(v1)->z,
	  GTS_POINT(v2)->x, GTS_POINT(v2)->y, GTS_POINT(v2)->z,
	  GTS_POINT(v3)->x, GTS_POINT(v3)->y, GTS_POINT(v3)->z,
	  GTS_POINT(v4)->x, GTS_POINT(v4)->y, GTS_POINT(v4)->z,
	  GTS_POINT(p)->x, GTS_POINT(p)->y, GTS_POINT(p)->z,
	  D, D1, D2, D3, D4, sgn) ;

  return GTV_OUT ; 
}

/** 
 * Generate a `large' tetrahedron which can be used to enclose a
 * Delaunay tetrahedralization under construction. 
 * 
 * @param klass a ::GtvTetrahedronClass;
 * @param facet_class a ::GtvFacetClass;
 * @param edge_class a ::GtsEdgeClass;
 * @param vertex_class a ::GtsVertexClass;
 * @param len a length.
 * 
 * @return a ::GtvTetrahedron with vertices at (0,0,\a len), (0,\a
 * len, -\a len), (\a len, -\a len, -\a len) and (-\a len, -\a len,
 * -\a len).
 */

GtvTetrahedron *gtv_tetrahedron_large(GtvTetrahedronClass *klass,
				      GtvFacetClass *facet_class,
				      GtsEdgeClass *edge_class,
				      GtsVertexClass *vertex_class,
				      gdouble len)

{
  GtsVertex *v1, *v2, *v3, *v4 ;
  GtsEdge *e1, *e2, *e3, *e4, *e5, *e6 ;
  GtvFacet *f1, *f2, *f3, *f4 ;
  GtvTetrahedron *t ;

  g_debug("%s: len=%lg", __FUNCTION__, len) ;

  g_return_val_if_fail(klass != NULL, NULL) ;
  g_return_val_if_fail(klass == 
		       gts_object_class_is_from_class(klass, 
						      gtv_tetrahedron_class()),
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
  g_return_val_if_fail(vertex_class != NULL, NULL) ;
  g_return_val_if_fail(vertex_class == 
		       gts_object_class_is_from_class(vertex_class, 
						      gts_vertex_class()),
		       NULL) ;
  g_return_val_if_fail(len != 0.0, NULL) ;

  v1 = gts_vertex_new(vertex_class, 0, 0, len) ;
  v2 = gts_vertex_new(vertex_class, 0, len, -len) ;
  v3 = gts_vertex_new(vertex_class, len, -len, -len) ;
  v4 = gts_vertex_new(vertex_class, -len, -len, -len) ;

  e1 = gts_edge_new(edge_class, v1, v2) ;
  e2 = gts_edge_new(edge_class, v2, v3) ;
  e3 = gts_edge_new(edge_class, v3, v1) ;
  e4 = gts_edge_new(edge_class, v1, v4) ;
  e5 = gts_edge_new(edge_class, v2, v4) ;
  e6 = gts_edge_new(edge_class, v3, v4) ;

  f1 = gtv_facet_new(facet_class, e1, e2, e3) ;
  f2 = gtv_facet_new(facet_class, e1, e4, e5) ;
  f3 = gtv_facet_new(facet_class, e2, e5, e6) ;
  f4 = gtv_facet_new(facet_class, e3, e6, e4) ;

  t = gtv_tetrahedron_new(klass, f1, f2, f3, f4) ;

  return t ;
}

/** 
 * Generate an equilateral tetrahedron.
 * 
 * @param klass a ::GtvTetrahedronClass;
 * @param facet_class a ::GtvFacetClass;
 * @param edge_class a ::GtsEdgeClass;
 * @param vertex_class a ::GtsVertexClass;
 * @param len the length of a side of the tetrahedron.
 * 
 * @return a ::GtvTetrahedron with vertices at (\a
 * \f$\sqrt{3}/3\f$\a len,0,0), (-\f$\sqrt{3}/6\f$\a len,\f$\pm\f$\a
 * len/2,0) and (0,0,\f$\sqrt{6}/3\f$ \a len).
 */

GtvTetrahedron *gtv_tetrahedron_equilateral(GtvTetrahedronClass *klass,
					    GtvFacetClass *facet_class,
					    GtsEdgeClass *edge_class,
					    GtsVertexClass *vertex_class,
					    gdouble len)
{

  GtsVertex *v1, *v2, *v3, *v4 ;
  GtsEdge *e1, *e2, *e3, *e4, *e5, *e6 ;
  GtvFacet *f1, *f2, *f3, *f4 ;
  GtvTetrahedron *t ;
  gdouble root3 = sqrt(3.0) ;

  g_return_val_if_fail(klass != NULL, NULL) ;
  g_return_val_if_fail(klass == 
		       gts_object_class_is_from_class(klass, 
						      gtv_tetrahedron_class()),
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
  g_return_val_if_fail(vertex_class != NULL, NULL) ;
  g_return_val_if_fail(vertex_class == 
		       gts_object_class_is_from_class(vertex_class, 
						      gts_vertex_class()),
		       NULL) ;
  g_return_val_if_fail(len != 0.0, NULL) ;

  g_debug("%s: len=%lg", __FUNCTION__, len) ;

  v1 = gts_vertex_new(vertex_class, -0.5*len/root3, -0.5*len, 0) ;
  v2 = gts_vertex_new(vertex_class, len/root3, 0, 0) ;
  v3 = gts_vertex_new(vertex_class, -0.5*len/root3,  0.5*len, 0) ;
  v4 = gts_vertex_new(vertex_class, 0, 0, len*M_SQRT2/root3) ;

  e1 = gts_edge_new(edge_class, v1, v2) ;
  e2 = gts_edge_new(edge_class, v2, v3) ;
  e3 = gts_edge_new(edge_class, v3, v1) ;
  e4 = gts_edge_new(edge_class, v1, v4) ;
  e5 = gts_edge_new(edge_class, v2, v4) ;
  e6 = gts_edge_new(edge_class, v3, v4) ;

  f1 = gtv_facet_new(facet_class, e1, e2, e3) ;
  f2 = gtv_facet_new(facet_class, e1, e4, e5) ;
  f3 = gtv_facet_new(facet_class, e2, e5, e6) ;
  f4 = gtv_facet_new(facet_class, e3, e6, e4) ;

  t = gtv_tetrahedron_new(klass, f1, f2, f3, f4) ;

  return t ;
}

/** 
 * Find the facet shared by two tetrahedra.
 * 
 * @param t1 a ::GtvTetrahedron;
 * @param t2 another ::GtvTetrahedron.
 * 
 * @return the ::GtvFacet between \a t1 and \a t2, if they neighbour
 * each other, NULL otherwise.
 */

GtvFacet *gtv_tetrahedra_common_facet(GtvTetrahedron *t1,
				      GtvTetrahedron *t2)

{
  g_return_val_if_fail(t1 != NULL, NULL) ;
  g_return_val_if_fail(t2 != NULL, NULL) ;

  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t1), NULL) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t2), NULL) ;

  if ( (t1->f1 == t2->f1) ||
       (t1->f1 == t2->f2) ||
       (t1->f1 == t2->f3) ||
       (t1->f1 == t2->f4) ) return t1->f1 ;

  if ( (t1->f2 == t2->f1) ||
       (t1->f2 == t2->f2) ||
       (t1->f2 == t2->f3) ||
       (t1->f2 == t2->f4) ) return t1->f2 ;

  if ( (t1->f3 == t2->f1) ||
       (t1->f3 == t2->f2) ||
       (t1->f3 == t2->f3) ||
       (t1->f3 == t2->f4) ) return t1->f3 ;

  if ( (t1->f4 == t2->f1) ||
       (t1->f4 == t2->f2) ||
       (t1->f4 == t2->f3) ||
       (t1->f4 == t2->f4) ) return t1->f4 ;

  return NULL ;
}

/** 
 * Find the tetrahedron neighbouring a specified tetrahedron on a
 * given side.
 * 
 * @param t a ::GtvTetrahedron
 * @param f a ::GtvFacet of \a t
 * 
 * @return the ::GtvTetrahedron which neighbours \a t on the side \a
 * f, if there is one, otherwise NULL.
 */

GtvTetrahedron *gtv_tetrahedron_opposite(GtvTetrahedron *t,
					 GtvFacet *f)

{
  GSList *i ;

  g_return_val_if_fail(t != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), NULL) ;
  g_return_val_if_fail(f != NULL, NULL) ;
  g_return_val_if_fail(t->f1 == f || t->f2 == f || t->f3 == f ||
		       t->f4 == f, NULL) ;

  for ( i = f->tetrahedra ; i != NULL ; i = i->next ) 
    if ( GTV_TETRAHEDRON(i->data) != t ) return GTV_TETRAHEDRON(i->data) ;

  g_debug("%s: no opposite tetrahedron found for facet %p on tetrahedron %p",
	  __FUNCTION__, f, t) ;

  return NULL ;
}

/** 
 * Find the orientation of the vertices of a tetrahedron. This is an
 * approximation of six times the signed volume of the tetrahedron. 
 * 
 * @param t a ::GtvTetrahedron.
 * 
 * @return a positive value if the tetrahedron apex lies above the
 * plane of the other three points, taken in cyclic order; a negative
 * value if it lies below that plane and zero if it lies in the plane. 
 */

gdouble gtv_tetrahedron_orientation(GtvTetrahedron *t)

{
  GtsVertex *v1, *v2, *v3, *v4 ;

  g_return_val_if_fail(t != NULL, 0.0) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), 0.0) ;

  gtv_tetrahedron_vertices(t, &v1, &v2, &v3, &v4) ;

  return gts_point_orientation_3d(GTS_POINT(v1),
				  GTS_POINT(v2),
				  GTS_POINT(v3),
				  GTS_POINT(v4)) ;    
}

/** 
 * Find the facets of a tetrahedron.
 * 
 * @param t ::GtvTetrahedron;
 * @param f1 ::GtvFacet of first face;
 * @param f2 ::GtvFacet of second face;
 * @param f3 ::GtvFacet of third face;
 * @param f4 ::GtvFacet of fourth face.
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_tetrahedron_facets(GtvTetrahedron *t,
			    GtvFacet **f1, GtvFacet **f2,
			    GtvFacet **f3, GtvFacet **f4)
{
  g_return_val_if_fail(t != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(f1 != NULL && f2 != NULL && f3 != NULL &&
		       f4 != NULL, GTV_NULL_ARGUMENT) ;

  *f1 = GTV_TETRAHEDRON(t)->f1 ;
  *f2 = GTV_TETRAHEDRON(t)->f2 ;
  *f3 = GTV_TETRAHEDRON(t)->f3 ;
  *f4 = GTV_TETRAHEDRON(t)->f4 ;

  return GTV_SUCCESS ;
}

/** 
 * Invert a tetrahedron by changing the order of two faces. This will
 * change the sign of the tetrahedron volume.
 * 
 * @param t ::GtvTetrahedron to invert.
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_tetrahedron_invert(GtvTetrahedron *t)

{
  GtvFacet *f ;

  g_return_val_if_fail(t != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), GTV_WRONG_TYPE) ;

  f = t->f3 ; t->f3 = t->f4 ; t->f4 = f ;

  return GTV_SUCCESS ;
}

/** 
 * Find the vertex of a tetrahedron which is opposite a specified
 * face.
 * 
 * @param t ::GtvTetrahedron;
 * @param f ::GtvFacet opposite to which a ::GtsVertex is to be found. 
 * 
 * @return ::GtsVertex of \a t which is opposite \a f, if \a f is on \a
 * t, NULL otherwise.
 */

GtsVertex *gtv_tetrahedron_vertex_opposite(GtvTetrahedron *t,
					   GtvFacet *f)

{
  GtsSegment *e1 = NULL, *e2 = NULL;

  g_return_val_if_fail(t != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), NULL) ;

  g_return_val_if_fail(f != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_FACET(f), NULL) ;
  g_return_val_if_fail(gtv_tetrahedron_has_facet(t, f), NULL) ;

  if ( f == t->f1 ) {
    e1 = GTS_SEGMENT(gts_triangles_common_edge(GTS_TRIANGLE(t->f2),
					       GTS_TRIANGLE(t->f4))) ;
    e2 = GTS_SEGMENT(gts_triangles_common_edge(GTS_TRIANGLE(t->f3),
					       GTS_TRIANGLE(t->f4))) ;
  }

  if ( f == t->f2 ) {
    e1 = GTS_SEGMENT(gts_triangles_common_edge(GTS_TRIANGLE(t->f3),
					       GTS_TRIANGLE(t->f4))) ;
    e2 = GTS_SEGMENT(gts_triangles_common_edge(GTS_TRIANGLE(t->f1),
					       GTS_TRIANGLE(t->f3))) ;
  }

  if ( f == t->f3 ) {
    e1 = GTS_SEGMENT(gts_triangles_common_edge(GTS_TRIANGLE(t->f2),
					       GTS_TRIANGLE(t->f4))) ;
    e2 = GTS_SEGMENT(gts_triangles_common_edge(GTS_TRIANGLE(t->f1),
					       GTS_TRIANGLE(t->f2))) ;
  }

  if ( f == t->f4 ) {
    e1 = GTS_SEGMENT(gts_triangles_common_edge(GTS_TRIANGLE(t->f2),
					       GTS_TRIANGLE(t->f3))) ;
    e2 = GTS_SEGMENT(gts_triangles_common_edge(GTS_TRIANGLE(t->f1),
					       GTS_TRIANGLE(t->f3))) ;
  }

  g_assert(e1 != NULL && e2 != NULL) ;

  if ( e1->v1 == e2->v1 || e1->v1 == e2->v2 ) return e1->v1 ;

  return e1->v2 ;
}

/** 
 * Find the face of a tetrahedron which is opposite a specified vertex. 
 * 
 * @param t ::GtvTetrahedron;
 * @param v ::GtsVertex opposite to which a ::GtvFacet is to be found. 
 * 
 * @return ::GtvFacet of \a t which is opposite \a v, if \a v is on \a
 * t, NULL otherwise.
 */

GtvFacet *gtv_tetrahedron_facet_opposite(GtvTetrahedron *t,
					 GtsVertex *v)

{
  GtsVertex *v1, *v2, *v3, *v4 ;

  g_return_val_if_fail(t != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), NULL) ;
  g_return_val_if_fail(v != NULL, NULL) ;
  g_return_val_if_fail(GTS_IS_VERTEX(v), NULL) ;

  gtv_tetrahedron_vertices(t, &v1, &v2, &v3, &v4) ;

  if ( v == v1 ) return ( t->f1 ) ;
  if ( v == v2 ) return ( t->f2 ) ;
  if ( v == v3 ) return ( t->f3 ) ;
  if ( v == v4 ) return ( t->f4 ) ;

  return NULL ;
}

/** 
 * Check that a ::GtvTetrahedron has positive volume and is not a
 * duplicate.
 * 
 * @param t ::GtvTetrahedron to check.
 * 
 * @return TRUE if \a t is okay, FALSE otherwise.
 */

gboolean gtv_tetrahedron_is_okay(GtvTetrahedron *t)

{
  g_return_val_if_fail(t != NULL, FALSE) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), FALSE) ;

  if ( gtv_tetrahedron_is_duplicate(t) ) return FALSE ;

  if ( gtv_tetrahedron_volume(t) <= 0.0 ) return FALSE ;

  return TRUE ;
}

/** 
 * Generate a new tetrahedron from four ::GtsVertex's, making use of
 * any existing ::GtvFacet's which connect them. 
 * 
 * @param klass a ::GtvTetrahedronClass
 * @param facet_class a ::GtvFacetClass
 * @param edge_class a ::GtsEdgeClass
 * @param v1 a ::GtsVertex
 * @param v2 a ::GtsVertex
 * @param v3 a ::GtsVertex
 * @param v4 a ::GtsVertex
 * 
 * @return the new ::GtvTetrahedron
 */

GtvTetrahedron *gtv_tetrahedron_new_from_vertices(GtvTetrahedronClass *klass,
						  GtvFacetClass *facet_class,
						  GtsEdgeClass *edge_class,
						  GtsVertex *v1,
						  GtsVertex *v2,
						  GtsVertex *v3,
						  GtsVertex *v4)

{
  GtvTetrahedron *t ;
  GtvFacet *f1, *f2, *f3, *f4 ;

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

  if ( (t = gtv_tetrahedron_from_facets(f1, f2, f3, f4)) == NULL )
    t = gtv_tetrahedron_new(klass, f1, f2, f3, f4) ;

  return t ;
}

/** 
 * Make a new ::GtvTetrahedron from existing connections between four
 * vertices.
 * 
 * @param v1 a GtsVertex; 
 * @param v2 a GtsVertex; 
 * @param v3 a GtsVertex; 
 * @param v4 a GtsVertex.
 * 
 * @return a ::GtvTetrahedron which uses facets connecting \a v1, \a
 * v2, \a v3 and \a v4, and NULL if any of these facets do not exist.
 */

GtvTetrahedron *gtv_tetrahedron_from_vertices(GtsVertex *v1,
					      GtsVertex *v2,
					      GtsVertex *v3,
					      GtsVertex *v4)

{
  GtvTetrahedron *t ;
  GtvFacet *f1, *f2, *f3, *f4 ;

  if ( (f1 = GTV_FACET(triangle_from_vertices(v2, v3, v4))) == NULL )
    return NULL ;
  if ( (f2 = GTV_FACET(triangle_from_vertices(v3, v4, v1))) == NULL )
    return NULL ;
  if ( (f3 = GTV_FACET(triangle_from_vertices(v4, v1, v2))) == NULL )
    return NULL ;
  if ( (f4 = GTV_FACET(triangle_from_vertices(v1, v2, v3))) == NULL )
    return NULL ;

  t = gtv_tetrahedron_from_facets(f1, f2, f3, f4) ;

  return t ;
}

/** 
 * Check if a ::GtvTetrahedron has a given GtsEdge.
 * 
 * @param t a ::GtvTetrahedron;
 * @param e a GtsEdge.
 * 
 * @return TRUE if a facet of \a t uses \a e, FALSE otherwise.
 */

gboolean gtv_tetrahedron_has_edge(GtvTetrahedron *t, GtsEdge *e)

{
  g_return_val_if_fail(t != NULL, FALSE) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), FALSE) ;
  g_return_val_if_fail(e != NULL, FALSE) ;
  g_return_val_if_fail(GTS_IS_EDGE(e), FALSE) ;

  if ( GTS_TRIANGLE(t->f1)->e1 == e ) return TRUE ;
  if ( GTS_TRIANGLE(t->f1)->e2 == e ) return TRUE ;
  if ( GTS_TRIANGLE(t->f1)->e3 == e ) return TRUE ;

  if ( GTS_TRIANGLE(t->f2)->e1 == e ) return TRUE ;
  if ( GTS_TRIANGLE(t->f2)->e2 == e ) return TRUE ;
  if ( GTS_TRIANGLE(t->f2)->e3 == e ) return TRUE ;

  if ( GTS_TRIANGLE(t->f3)->e1 == e ) return TRUE ;
  if ( GTS_TRIANGLE(t->f3)->e2 == e ) return TRUE ;
  if ( GTS_TRIANGLE(t->f3)->e3 == e ) return TRUE ;

  if ( GTS_TRIANGLE(t->f4)->e1 == e ) return TRUE ;
  if ( GTS_TRIANGLE(t->f4)->e2 == e ) return TRUE ;
  if ( GTS_TRIANGLE(t->f4)->e3 == e ) return TRUE ;

  return FALSE ;
}

/** 
 * Check if a ::GtvTetrahedron has a given GtsVertex.
 * 
 * @param t a ::GtvTetrahedron;
 * @param v a GtsVertex.
 * 
 * @return TRUE \a v is a vertex of \a t, FALSE otherwise.
 */

gboolean gtv_tetrahedron_has_vertex(GtvTetrahedron *t, GtsVertex *v)

{
  GtsVertex *v1, *v2, *v3, *v4 ;

  g_return_val_if_fail(t != NULL, GTV_NULL_ARGUMENT);
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT);
  g_return_val_if_fail(GTS_IS_VERTEX(v), GTV_WRONG_TYPE) ;

  gtv_tetrahedron_vertices(t, &v1, &v2, &v3, &v4) ;

  if ( ((v == v1) || (v == v2) || (v == v3) || (v == v4)) ) 
    g_debug("%s: tetrahedron %p has vertex %p", __FUNCTION__, t, v) ;
  else
    g_debug("%s: tetrahedron %p does not have vertex %p", __FUNCTION__, t, v) ;

  return ((v == v1) || (v == v2) || (v == v3) || (v == v4)) ;
}

/** 
 * Find a ::GtvTetrahedron which lies in the `wedge' between two other
 * tetrahedra.
 * 
 * @param t1 a ::GtvTetrahedron;
 * @param t2 another ::GtvTetrahedron;
 * 
 * @return a ::GtvTetrahedron which shares one facet with \a t1 and
 * another facet with \a t2, if it exists, NULL otherwise.
 */

GtvTetrahedron *gtv_tetrahedra_wedge_neighbour(GtvTetrahedron *t1,
					       GtvTetrahedron *t2)

{
  GtvTetrahedron *t ;

  g_return_val_if_fail(t1 != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t1), NULL) ;

  g_return_val_if_fail(t2 != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t2), NULL) ;

  g_return_val_if_fail(t1 != t2, NULL);

  if ( (t = gtv_tetrahedron_opposite(t1, t1->f1)) != NULL )
    if ( t == gtv_tetrahedron_opposite(t2, t2->f1) ||
	 t == gtv_tetrahedron_opposite(t2, t2->f2) ||
	 t == gtv_tetrahedron_opposite(t2, t2->f3) ||
	 t == gtv_tetrahedron_opposite(t2, t2->f4) ) return t ;

  if ( (t = gtv_tetrahedron_opposite(t1, t1->f2)) != NULL )
    if ( t == gtv_tetrahedron_opposite(t2, t2->f1) ||
	 t == gtv_tetrahedron_opposite(t2, t2->f2) ||
	 t == gtv_tetrahedron_opposite(t2, t2->f3) ||
	 t == gtv_tetrahedron_opposite(t2, t2->f4) ) return t ;

  if ( (t = gtv_tetrahedron_opposite(t1, t1->f3)) != NULL )
    if ( t == gtv_tetrahedron_opposite(t2, t2->f1) ||
	 t == gtv_tetrahedron_opposite(t2, t2->f2) ||
	 t == gtv_tetrahedron_opposite(t2, t2->f3) ||
	 t == gtv_tetrahedron_opposite(t2, t2->f4) ) return t ;

  if ( (t = gtv_tetrahedron_opposite(t1, t1->f4)) != NULL )
    if ( t == gtv_tetrahedron_opposite(t2, t2->f1) ||
	 t == gtv_tetrahedron_opposite(t2, t2->f2) ||
	 t == gtv_tetrahedron_opposite(t2, t2->f3) ||
	 t == gtv_tetrahedron_opposite(t2, t2->f4) ) return t ;

  return NULL ;
}

/** 
 * Find the facet of a ::GtvTetrahedron on which lies a ::GtsPoint. 
 * 
 * @param t a ::GtvTetrahedron;
 * @param p a ::GtsPoint.
 * 
 * @return the facet of \a t which contains \a p, i.e. \a p lies in
 * the plane of the facet and inside or on its boundary.
 */

GtvFacet *gtv_point_in_tetrahedron_facet(GtvTetrahedron *t, 
					 GtsPoint *p)

{
  GtsVertex *v1, *v2, *v3, *v4 ;
  gdouble xmin, xmax, ymin, ymax, zmin, zmax, Di ;

  g_return_val_if_fail(t != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), NULL) ;
  g_return_val_if_fail(p != NULL, NULL) ;
  g_return_val_if_fail(GTS_IS_POINT(p), NULL) ;

  gtv_tetrahedron_vertices(t, &v1, &v2, &v3, &v4) ;

  xmin = MIN(GTS_POINT(v1)->x,
	     MIN(GTS_POINT(v2)->x,
		 MIN(GTS_POINT(v3)->x,GTS_POINT(v4)->x))) ;
  xmax = MAX(GTS_POINT(v1)->x,
	     MAX(GTS_POINT(v2)->x,
		 MAX(GTS_POINT(v3)->x,GTS_POINT(v4)->x))) ;
  ymin = MIN(GTS_POINT(v1)->y,
	     MIN(GTS_POINT(v2)->y,
		 MIN(GTS_POINT(v3)->y,GTS_POINT(v4)->y))) ;
  ymax = MAX(GTS_POINT(v1)->y,
	     MAX(GTS_POINT(v2)->y,
		 MAX(GTS_POINT(v3)->y,GTS_POINT(v4)->y))) ;
  zmin = MIN(GTS_POINT(v1)->z,
	     MIN(GTS_POINT(v2)->z,
		 MIN(GTS_POINT(v3)->z,GTS_POINT(v4)->z))) ;
  zmax = MAX(GTS_POINT(v1)->z,
	     MAX(GTS_POINT(v2)->z,
		 MAX(GTS_POINT(v3)->z,GTS_POINT(v4)->z))) ;
  if ( (p->x < xmin) || (p->x > xmax) ||
       (p->y < ymin) || (p->y > ymax) ||
       (p->z < zmin) || (p->z > zmax) ) return NULL ;

  if ( (Di = gts_point_orientation_3d(GTS_POINT(p),
				      GTS_POINT(v2),
				      GTS_POINT(v3),
				      GTS_POINT(v4))) == 0.0 )
    return t->f1 ;

  if ( (Di = gts_point_orientation_3d(GTS_POINT(v1),
				      GTS_POINT(p),
				      GTS_POINT(v3),
				      GTS_POINT(v4))) == 0.0 )
    return t->f2 ;

  if ( (Di = gts_point_orientation_3d(GTS_POINT(v1),
				      GTS_POINT(v2),
				      GTS_POINT(p),
				      GTS_POINT(v4))) == 0.0 )
    return t->f3 ;

  if ( (Di = gts_point_orientation_3d(GTS_POINT(v1),
				      GTS_POINT(v2),
				      GTS_POINT(v3),
				      GTS_POINT(p))) == 0.0 )
    return t->f4 ;

  return NULL ;
}

/** 
 * Find the vertices of a ::GtvTetrahedron which are not used by a
 * GtsEdge. If \a e is not used by \a t, \a a and \a b will be NULL.
 * 
 * @param t a ::GtvTetrahedron;
 * @param e a GtsEdge;
 * @param a on exit, a GtsVertex of \a t which does not lie on \a e
 * @param b on exit, the other GtsVertex of \a t which does not lie on \a e.
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_tetrahedron_vertices_off_edge(GtvTetrahedron *t,
				       GtsEdge *e,
				       GtsVertex **a,
				       GtsVertex **b)

{
  GtsVertex *v1, *v2, *v3, *v4 ;

  g_return_val_if_fail(t != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(e != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTS_IS_EDGE(e), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(a != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(b != NULL, GTV_NULL_ARGUMENT) ;

  gtv_tetrahedron_vertices(t, &v1, &v2, &v3, &v4) ;

  if ( GTS_EDGE(gts_vertices_are_connected(v1, v2)) == e ) {
    *a = v3 ; *b = v4 ; return GTV_SUCCESS ;
  }
  if ( GTS_EDGE(gts_vertices_are_connected(v1, v3)) == e ) {
    *a = v2 ; *b = v4 ; return GTV_SUCCESS ;
  }
  if ( GTS_EDGE(gts_vertices_are_connected(v1, v4)) == e ) {
    *a = v2 ; *b = v3 ; return GTV_SUCCESS ;
  }

  if ( GTS_EDGE(gts_vertices_are_connected(v2, v3)) == e ) {
    *a = v1 ; *b = v4 ; return GTV_SUCCESS ;
  }
  if ( GTS_EDGE(gts_vertices_are_connected(v2, v4)) == e ) {
    *a = v1 ; *b = v3 ; return GTV_SUCCESS ;
  }

  if ( GTS_EDGE(gts_vertices_are_connected(v3, v4)) == e ) {
    *a = v1 ; *b = v2 ; return GTV_SUCCESS ;
  }

  *a = *b = NULL ;
    
  return GTV_FAILURE ;
}

/** 
 * Find the three vertices of a ::GtvTetrahedron opposite a given
 * ::GtsVertex of the tetrahedron, respecting the orientation of the
 * tetrahedron. On exit, \a v1, \a v2 and \a v3 will be the vertices
 * of the facet of \a t opposite \a v, such that the orientation of \a
 * v, \a v1, \a v2 and \a v3 will be the same as that of the
 * tetrahedron itself, including the sign.
 * 
 * @param t a ::GtvTetrahedron;
 * @param v a ::GtsVertex;
 * @param v1 a ::GtsVertex;
 * @param v2 a ::GtsVertex;
 * @param v3 a ::GtsVertex.
 * 
 * @return GTV_SUCCESS or GTV_FAILURE if \a v is not a vertex of \a t.
 */

gint gtv_tetrahedron_opposite_vertices(GtvTetrahedron *t,
				       GtsVertex *v,
				       GtsVertex **v1,
				       GtsVertex **v2,
				       GtsVertex **v3)

{
  GtsVertex *w1, *w2, *w3, *w4 ;

  g_return_val_if_fail(t != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTS_IS_VERTEX(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(v1 != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(v2 != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(v3 != NULL, GTV_NULL_ARGUMENT) ;

  *v1 = *v2 = *v3 = NULL ;

  gtv_tetrahedron_vertices(t, &w1, &w2, &w3, &w4) ;

  if ( v == w1 ) {
    *v1 = w2 ; *v2 = w3 ; *v3 = w4 ; return GTV_SUCCESS ;
  }

  if ( v == w2 ) {
    *v1 = w3 ; *v2 = w4 ; *v3 = w1 ; return GTV_SUCCESS ;
  }

  if ( v == w3 ) {
    *v1 = w4 ; *v2 = w1 ; *v3 = w2 ; return GTV_SUCCESS ;
  }

  if ( v == w4 ) {
    *v1 = w1 ; *v2 = w2 ; *v3 = w3 ; return GTV_SUCCESS ;
  }

  return GTV_FAILURE ;
}

/** 
 * Orient the facets of a tetrahedron so that it has non-negative volume. 
 * 
 * @param t a ::GtvTetrahedron;
 * 
 * @return GTV_SUCCESS on success, i.e. \a t has positive volume.
 */

gint gtv_tetrahedron_orient(GtvTetrahedron *t)

{
  gpointer swap ;

  g_return_val_if_fail(t != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), GTV_WRONG_TYPE) ;

  g_debug("%s:", __FUNCTION__) ;

  if ( gtv_tetrahedron_volume(t) >= 0.0 ) return GTV_SUCCESS ;

  g_debug("%s: swapping f3 and f4", __FUNCTION__) ;
  swap = t->f3 ; t->f3 = t->f4 ; t->f4 = swap ;

  if ( gtv_tetrahedron_volume(t) >= 0.0 ) return GTV_SUCCESS ;

  g_debug("%s: swapping f2 and f3", __FUNCTION__) ;
  swap = t->f2 ; t->f2 = t->f3 ; t->f3 = swap ;

  g_assert(gtv_tetrahedron_volume(t) >= 0.0 ) ;

  if ( gtv_tetrahedron_volume(t) >= 0.0 ) return GTV_SUCCESS ;

  return GTV_FAILURE ;
}				       

/** 
 * Check if any facet of a ::GtvTetrahedron is encroached.
 * 
 * @param t a ::GtvTetrahedron.
 * 
 * @return a GtsVertex encroaching a facet of \a t, NULL if all facets
 * of \a t are regular.
 */

GtsVertex *gtv_tetrahedron_is_encroached(GtvTetrahedron *t)

{
  g_return_val_if_fail(t != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), NULL) ;

  if ( !gtv_facet_is_regular(t->f1) )
    return gtv_tetrahedron_vertex_opposite(gtv_tetrahedron_opposite(t, t->f1),
					   t->f1) ;
  if ( !gtv_facet_is_regular(t->f2) )
    return gtv_tetrahedron_vertex_opposite(gtv_tetrahedron_opposite(t, t->f2),
					   t->f2) ;
  if ( !gtv_facet_is_regular(t->f3) )
    return gtv_tetrahedron_vertex_opposite(gtv_tetrahedron_opposite(t, t->f3),
					   t->f3) ;
  if ( !gtv_facet_is_regular(t->f4) )
    return gtv_tetrahedron_vertex_opposite(gtv_tetrahedron_opposite(t, t->f4),
					   t->f4) ;

  return NULL ;
}

/** 
 * Compute the radius of the insphere of a ::GtvTetrahedron, using the
 * formula given in <a
 * href="http://andrew.cmu.edu/user/sowen/abstracts/Do601.html">Proposal
 * of Benchmarks for 3D Unstructured Tetrahedral Mesh
 * Optimization</a>, Julien Dompierre, Paul Labbe, Francois Guibault,
 * Ricardo Camarero, 1998.
 * 
 * @param t a ::GtvTetrahedron.
 * 
 * @return the inradius of \a t. 
 */

gdouble gtv_tetrahedron_inradius(GtvTetrahedron *t)

{
  gdouble R, V, s ;

  g_return_val_if_fail(t != NULL, 0.0) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), 0.0) ;

  V = gtv_tetrahedron_volume(t) ;

  s = gts_triangle_area(GTS_TRIANGLE(t->f1)) +
    gts_triangle_area(GTS_TRIANGLE(t->f2)) +
    gts_triangle_area(GTS_TRIANGLE(t->f3)) +
    gts_triangle_area(GTS_TRIANGLE(t->f4)) ;
  
  R = 3.0*V/s ;

  return R ;
}

/** 
 * Calculate the radius of the circumsphere of a ::GtvTetrahedron
 * using the method given by <a
 * href="http://mathworld.wolfram.com/Tetrahedron.html">Eric
 * Weisstein's Mathworld</a>. 
 * 
 * @param t a ::GtvTetrahedron.
 * 
 * @return the circumradius of \a t.
 */

gdouble gtv_tetrahedron_circumradius(GtvTetrahedron *t)

{
  gdouble V, D, R, a, aa, b, bb, c, cc, s ;
  GtsVertex *v1, *v2, *v3, *v4 ;

  g_return_val_if_fail(t != NULL, 0.0) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), 0.0) ;

  V = gtv_tetrahedron_volume(t) ;

  gtv_tetrahedron_vertices(t, &v1, &v2, &v3, &v4) ;

  a = gts_point_distance(GTS_POINT(v2), GTS_POINT(v3)) ;
  b = gts_point_distance(GTS_POINT(v3), GTS_POINT(v1)) ;
  c = gts_point_distance(GTS_POINT(v1), GTS_POINT(v2)) ;
  aa = gts_point_distance(GTS_POINT(v1), GTS_POINT(v4)) ;
  bb = gts_point_distance(GTS_POINT(v2), GTS_POINT(v4)) ;
  cc = gts_point_distance(GTS_POINT(v3), GTS_POINT(v4)) ;

  s = 0.5*(a*aa + b*bb + c*cc) ;

  D = sqrt(s*(s-a*aa)*(s-b*bb)*(s-c*cc)) ;

  R = D/V/6.0 ;

  return R ;
}

/** 
 * Calculate the radius ratio quality measure for a ::GtvTetrahedron,
 * as in <a
 * href="http://andrew.cmu.edu/user/sowen/abstracts/Do601.html">Proposal
 * of Benchmarks for 3D Unstructured Tetrahedral Mesh
 * Optimization</a>, Julien Dompierre, Paul Labbe, Francois Guibault,
 * Ricardo Camarero, 1998. This is three times the ratio of the
 * inradius and the circumradius of \a t and will be one for an
 * equilateral tetrahedron.
 * 
 * @param t a ::GtvTetrahedron.
 * 
 * @return the radius ratio quality measure of \a t.
 */

gdouble gtv_tetrahedron_quality_radius_ratio(GtvTetrahedron *t)

{
  g_return_val_if_fail(t != NULL, 0.0) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), 0.0) ;

  return 
    (3.0*gtv_tetrahedron_inradius(t)/gtv_tetrahedron_circumradius(t)) ;  
}

/** 
 * Generate a bounding box for a list of tetrahedra
 * 
 * @param klass a ::GtsBBoxClass;
 * @param tetrahedra a ::GSlist of ::GtvTetrahedrons.
 * 
 * @return a ::GtsBBox which contains the tetrahedra in \a tetrahedra. 
 */


GtsBBox *gtv_bbox_tetrahedra(GtsBBoxClass *klass, GSList *tetrahedra)

{
  GtsBBox *bbox ;
  GSList *i ;
  GtvTetrahedron *t ;
  GtsVertex *v1, *v2, *v3, *v4 ;

  g_return_val_if_fail(klass != NULL, NULL);
  g_return_val_if_fail(tetrahedra != NULL, NULL);

  bbox = gts_bbox_new (GTS_BBOX_CLASS(klass), tetrahedra, 
		       0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  bbox->x1 = bbox->y1 = bbox->z1 = G_MAXDOUBLE;
  bbox->x2 = bbox->y2 = bbox->z2 = -G_MAXDOUBLE;

  for ( i = tetrahedra ; i != NULL ; i = i->next ) {
    t = GTV_TETRAHEDRON(i->data) ;
    gtv_tetrahedron_vertices(t, &v1, &v2, &v3, &v4) ;
    bbox->x1 = 
      MIN(bbox->x1,MIN(GTS_POINT(v1)->x,
		       MIN(GTS_POINT(v2)->x,
			   MIN(GTS_POINT(v3)->x,GTS_POINT(v4)->x)))) ;
    bbox->x2 = 
      MAX(bbox->x2,MAX(GTS_POINT(v1)->x,
		       MAX(GTS_POINT(v2)->x,
			   MAX(GTS_POINT(v3)->x,GTS_POINT(v4)->x)))) ;
    bbox->y1 = 
      MIN(bbox->y1,MIN(GTS_POINT(v1)->y,
		       MIN(GTS_POINT(v2)->y,
			   MIN(GTS_POINT(v3)->y,GTS_POINT(v4)->y)))) ;
    bbox->y2 = 
      MAX(bbox->y2,MAX(GTS_POINT(v1)->y,
		       MAX(GTS_POINT(v2)->y,
			   MAX(GTS_POINT(v3)->y,GTS_POINT(v4)->y)))) ;
    bbox->z1 = 
      MIN(bbox->z1,MIN(GTS_POINT(v1)->z,
		       MIN(GTS_POINT(v2)->z,
			   MIN(GTS_POINT(v3)->z,GTS_POINT(v4)->z)))) ;
    bbox->z2 = 
      MAX(bbox->z2,MAX(GTS_POINT(v1)->z,
		       MAX(GTS_POINT(v2)->z,
			   MAX(GTS_POINT(v3)->z,GTS_POINT(v4)->z)))) ;
  }

  return bbox ;
}

/** 
 * Compute the centroid of a ::GtvTetrahedron as the mean of the four
 * vertex positions.
 * 
 * @param t a ::GtvTetrahedron;
 * @param c a ::GtsPoint, whose components will be set to the centroid of 
 * \a t on successful exit.
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_tetrahedron_centroid(GtvTetrahedron *t, GtsPoint *c)

{
  GtsVertex *v1, *v2, *v3, *v4 ;

  g_return_val_if_fail(t != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), GTV_WRONG_TYPE) ;  
  g_return_val_if_fail(c != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTS_IS_POINT(c), GTV_WRONG_TYPE) ;

  gtv_tetrahedron_vertices(t, &v1, &v2, &v3, &v4) ;

  gts_point_set(c, 
		0.25*(GTS_POINT(v1)->x+GTS_POINT(v2)->x+
		      GTS_POINT(v3)->x+GTS_POINT(v4)->x),
		0.25*(GTS_POINT(v1)->y+GTS_POINT(v2)->y+
		      GTS_POINT(v3)->y+GTS_POINT(v4)->y),
		0.25*(GTS_POINT(v1)->z+GTS_POINT(v2)->z+
		      GTS_POINT(v3)->z+GTS_POINT(v4)->z)) ;

  return GTV_SUCCESS ;
}

static void tet_edge_lengths(GtvTetrahedron *t,
			     gdouble *l12, gdouble *l13, gdouble *l14,
			     gdouble *l23, gdouble *l24, 
			     gdouble *l34)

{
  GtsVertex *v1, *v2, *v3, *v4 ;

  gtv_tetrahedron_vertices(t, &v1, &v2, &v3, &v4) ;

  *l12 = gts_point_distance(GTS_POINT(v1), GTS_POINT(v2)) ;
  *l13 = gts_point_distance(GTS_POINT(v1), GTS_POINT(v3)) ;
  *l14 = gts_point_distance(GTS_POINT(v1), GTS_POINT(v4)) ;
  *l23 = gts_point_distance(GTS_POINT(v2), GTS_POINT(v3)) ;
  *l24 = gts_point_distance(GTS_POINT(v2), GTS_POINT(v4)) ;
  *l34 = gts_point_distance(GTS_POINT(v3), GTS_POINT(v4)) ;

  return ;
}

static void tet_facet_areas(GtvTetrahedron *t,
			    gdouble *s1, gdouble *s2,
			    gdouble *s3, gdouble *s4)

{
  GtvFacet *f1, *f2, *f3, *f4 ;

  gtv_tetrahedron_facets(t, &f1, &f2, &f3, &f4) ;

  *s1 = gts_triangle_area(GTS_TRIANGLE(f1)) ;
  *s2 = gts_triangle_area(GTS_TRIANGLE(f2)) ;
  *s3 = gts_triangle_area(GTS_TRIANGLE(f3)) ;
  *s4 = gts_triangle_area(GTS_TRIANGLE(f4)) ;

  return ;
}

/** 
 * Calculate the radius ratio quality measure for a ::GtvTetrahedron,
 * given in Dompierre, Julien, Labbe', Paul, Guibault, Francois and
 * Camarero, Ricardo, <a
 * href="http://www.scientificcommons.org/333339">`Proposal of
 * benchmarks for 3D unstructured tetrahedral mesh optimization'</a>,
 * 1998. The measure \f$\rho=3\rho_{in}/\rho_{out}\f$ is three times
 * the ratio of the inradius and circumradius of the tetrahedron
 * (although computed in a different way).
 * 
 * @param t a ::GtvTetrahedron.
 * 
 * @return the radius ratio quality measure or zero if \a t is
 * degenerate.
 */

gdouble gtv_tetrahedron_radius_ratio(GtvTetrahedron *t)

{
  gdouble q, l12, l13, l14, l23, l24, l34 ;
  gdouble s1, s2, s3, s4 ;
  gdouble v, a, b, c ;

  g_return_val_if_fail(t != NULL, 0.0) ;
  g_return_val_if_fail(GTV_IS_TETRAHEDRON(t), 0.0) ;

  v = gtv_tetrahedron_volume(t) ;
  g_return_val_if_fail(v != 0.0, 0.0) ;

  tet_edge_lengths(t, &l12, &l13, &l14, &l23, &l24, &l34) ;
  tet_facet_areas(t, &s1, &s2, &s3, &s4) ;

  a = l12*l34 ; b = l13*l24 ; c = l14*l23 ;
  
  /*radius ratio measure*/
  q = 216.0*v*v/(sqrt((a+b+c)*(a+b-c)*(a+c-b)*(b+c-a))*(s1+s2+s3+s4)) ;

  return q ;
}


gint gtv_tetrahedron_point_barycentric(GtvTetrahedron *t, GtsPoint *p,
				       gdouble *w)

{
  gdouble A[9], Ai[9], r[3] ;
  GtsVertex *v1, *v2, *v3, *v4 ;
  
  if ( gtv_point_in_tetrahedron(p, t, NULL) == GTV_OUT ) {
    w[0] = w[1] = w[2] = w[3] = 0.0 ;
    return -1 ;
  }

  gtv_tetrahedron_vertices(t, &v1, &v2, &v3, &v4) ;
  
  A[0] = GTS_POINT(v1)->x - GTS_POINT(v4)->x ; 
  A[1] = GTS_POINT(v2)->x - GTS_POINT(v4)->x ; 
  A[2] = GTS_POINT(v3)->x - GTS_POINT(v4)->x ; 
  A[3] = GTS_POINT(v1)->y - GTS_POINT(v4)->y ; 
  A[4] = GTS_POINT(v2)->y - GTS_POINT(v4)->y ; 
  A[5] = GTS_POINT(v3)->y - GTS_POINT(v4)->y ; 
  A[6] = GTS_POINT(v1)->z - GTS_POINT(v4)->z ; 
  A[7] = GTS_POINT(v2)->z - GTS_POINT(v4)->z ; 
  A[8] = GTS_POINT(v3)->z - GTS_POINT(v4)->z ; 

  invert3x3(Ai, A) ;

  r[0] = p->x - GTS_POINT(v4)->x ; 
  r[1] = p->y - GTS_POINT(v4)->y ; 
  r[2] = p->z - GTS_POINT(v4)->z ; 

  w[0] = Ai[0]*r[0] + Ai[1]*r[1] + Ai[2]*r[2] ;
  w[1] = Ai[3]*r[0] + Ai[4]*r[1] + Ai[5]*r[2] ;
  w[2] = Ai[6]*r[0] + Ai[7]*r[1] + Ai[8]*r[2] ;

  w[3] = 1.0 - w[0] - w[1] - w[2] ;
  
  return 0 ;
}

/**
 * @}
 * 
 */
