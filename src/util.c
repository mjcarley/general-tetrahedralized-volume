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

GtsTriangle *triangle_from_vertices(GtsVertex *v1,
				    GtsVertex *v2,
				    GtsVertex *v3)

{
  GtsEdge *e1, *e2, *e3 ;

  g_return_val_if_fail(v1 != v2 && v2 != v3 && v3 != v1, NULL) ;

  if ( (e1 = GTS_EDGE(gts_vertices_are_connected(v1, v2)) ) 
       == NULL ) return NULL ;

  if ( (e2 = GTS_EDGE(gts_vertices_are_connected(v2, v3)) ) 
       == NULL ) return NULL ;

  if ( (e3 = GTS_EDGE(gts_vertices_are_connected(v3, v1)) ) 
       == NULL ) return NULL ;

  return gts_triangle_use_edges(e1, e2, e3) ;
}

GtvFacet *facet_new_from_vertices(GtvFacetClass *klass,
				  GtsEdgeClass *edge_class,
				  GtsVertex *v1,
				  GtsVertex *v2,
				  GtsVertex *v3)

{
  GtsEdge *e1, *e2, *e3 ;
  GtvFacet *f ;

  if ( (e1 = GTS_EDGE(gts_vertices_are_connected(v1, v2))) == NULL )
    e1 = gts_edge_new(edge_class, v1, v2) ;
  if ( (e2 = GTS_EDGE(gts_vertices_are_connected(v2, v3))) == NULL )
    e2 = gts_edge_new(edge_class, v2, v3) ;
  if ( (e3 = GTS_EDGE(gts_vertices_are_connected(v3, v1))) == NULL )
    e3 = gts_edge_new(edge_class, v3, v1) ;

  if ( (f = GTV_FACET(gts_triangle_use_edges(e1, e2, e3))) == NULL )
    f = gtv_facet_new(klass, e1, e2, e3) ;

  return f ;
}

gchar *intersection_status(GtvIntersect status)

{
  switch (status) {
  default: g_assert_not_reached() ; break ;
  case GTV_OUT: return "outside tetrahedron" ; break ;
  case GTV_ON: return "on tetrahedron" ; break ;
  case GTV_IN: return "inside tetrahedron" ; break ;
  case GTV_ON_FACET: return "on facet" ; break ;
  case GTV_ON_EDGE: return "on edge" ; break ;
  case GTV_ON_VERTEX: return "on vertex" ; break ;
  }
  
  return NULL ;
}
