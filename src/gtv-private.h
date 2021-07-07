/* GTV - Library for the manipulation of tetrahedralized volumes
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

#ifndef GTV_PRIVATE_INCLUDED
#define GTV_PRIVATE_INCLUDED

#include <gts.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#if DEVELOPER_DEBUG
#define GTV_DEVELOPER_DEBUG
#endif /*DEVELOPER_DEBUG*/

#define GTV_GMSH_ELEMENT_LINE          1
#define GTV_GMSH_ELEMENT_TRIANGLE      2
#define GTV_GMSH_ELEMENT_QUADRANGLE    3
#define GTV_GMSH_ELEMENT_TETRAHEDRON   4
#define GTV_GMSH_ELEMENT_HEXAHEDRON    5
#define GTV_GMSH_ELEMENT_PRISM         6
#define GTV_GMSH_ELEMENT_PYRAMID       7
#define GTV_GMSH_ELEMENT_LINE_2        8
#define GTV_GMSH_ELEMENT_TRIANGLE_2    9
#define GTV_GMSH_ELEMENT_QUADRANGLE_2  10
#define GTV_GMSH_ELEMENT_TETRAHEDRON_2 11
#define GTV_GMSH_ELEMENT_HEXAHEDRON_2  12
#define GTV_GMSH_ELEMENT_PRISM_2       13
#define GTV_GMSH_ELEMENT_PYRAMID_2     14
#define GTV_GMSH_ELEMENT_POINT         15

#ifndef g_debug
#define g_debug(format...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, format)
#endif
#ifndef g_message
#define g_message(format...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, format)
#endif
#ifndef g_warning
#define g_warning(format...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, format)
#endif
#ifndef g_error
#define g_error(format...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, format)
#endif

#include "gtv.h"

#define GTV_LOGGING_DATA_WIDTH     4
#define GTV_LOGGING_DATA_FID       0
#define GTV_LOGGING_DATA_PREFIX    1
#define GTV_LOGGING_DATA_LEVEL     2
#define GTV_LOGGING_DATA_EXIT_FUNC 3

#define GTV_STARTUP_MESSAGE \
"This is free software; see the source code for copying conditions.\n\
There is ABSOLUTELY NO WARRANTY, not even for MERCHANTABILITY or\n\
FITNESS FOR A PARTICULAR PURPOSE.\n"

GtsTriangle *triangle_from_vertices(GtsVertex *v1,
				    GtsVertex *v2,
				    GtsVertex *v3) ;
GtvFacet *facet_new_from_vertices(GtvFacetClass *klass,
				  GtsEdgeClass *edge_class,
				  GtsVertex *v1,
				  GtsVertex *v2,
				  GtsVertex *v3) ;
/* GtsVertex *triangle_opposite_vertex(GtsTriangle *t, */
/* 				    GtsVertex *v1, */
/* 				    GtsVertex *v2, */
/* 				    GtsVertex *v3) ; */
gchar *intersection_status(GtvIntersect status) ;

/* inline void invert3x3(gdouble *Ai, gdouble *A) ; */
/* inline void multiply3x1(gdouble y[], gdouble *A, gdouble x[]) ; */
void invert4x4(gdouble *Ai, gdouble *A) ;
void invert3x3(gdouble *Ai, gdouble *A) ;

#define multiply3x1(_y,_A,_x)						\
  ((_y)[0] = (_A)[0]*(_x)[0] + (_A)[1]*(_x)[1] + (_A)[2]*(_x)[2],	\
   (_y)[1] = (_A)[3]*(_x)[0] + (_A)[4]*(_x)[1] + (_A)[5]*(_x)[2],	\
   (_y)[2] = (_A)[6]*(_x)[0] + (_A)[7]*(_x)[1] + (_A)[8]*(_x)[2])	

#define box_diagonal(box) (sqrt((box->x1-box->x2)*(box->x1-box->x2) + \
				(box->y1-box->y2)*(box->y1-box->y2) + \
				(box->z1-box->z2)*(box->z1-box->z2)))

#endif /* GTV_PRIVATE_INCLUDED*/
