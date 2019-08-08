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
#include <stdlib.h>

#include <gts.h>

#include "predicates.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#include "gtv.h"
#include "gtv-private.h"

/**
 * @defgroup geometry Geometric tests
 * @{
 * 
 */

/** 
 * Check whether a point lies inside a sphere defined by four
 * GtsPoint's.
 * 
 * @param p a ::GtsPoint;
 * @param p1 a ::GtsPoint;
 * @param p2 a ::GtsPoint;
 * @param p3 a ::GtsPoint;
 * @param p4 a ::GtsPoint;
 * 
 * @return positive value if \a p lies inside the sphere, zero if it
 * lies on the sphere and a negative value if it lies outside.
 */

gdouble gtv_point_in_sphere(GtsPoint *p, 
			    GtsPoint *p1,
			    GtsPoint *p2,
			    GtsPoint *p3,
			    GtsPoint *p4)

{
  gdouble isp ;

  g_return_val_if_fail(p != NULL, 0.0) ;
  g_return_val_if_fail(GTS_IS_POINT(p), 0.0) ;
  g_return_val_if_fail(p1 != NULL, 0.0) ;
  g_return_val_if_fail(GTS_IS_POINT(p1), 0.0) ;
  g_return_val_if_fail(p2 != NULL, 0.0) ;
  g_return_val_if_fail(GTS_IS_POINT(p2), 0.0) ;
  g_return_val_if_fail(p3 != NULL, 0.0) ;
  g_return_val_if_fail(GTS_IS_POINT(p3), 0.0) ;
  g_return_val_if_fail(p4 != NULL, 0.0) ;
  g_return_val_if_fail(GTS_IS_POINT(p4), 0.0) ;

  isp = insphere((gdouble *)(&(GTS_POINT(p1)->x)),
		 (gdouble *)(&(GTS_POINT(p2)->x)),
		 (gdouble *)(&(GTS_POINT(p3)->x)),
		 (gdouble *)(&(GTS_POINT(p4)->x)),
		 (gdouble *)(&p->x)) ;

  if ( gts_point_orientation_3d(GTS_POINT(p1),
				GTS_POINT(p2),
				GTS_POINT(p3),
				GTS_POINT(p4)) < 0.0 ) 
    return (-isp) ;

  return isp ;
}


/** 
 * Check if three points are collinear by checking their orientation
 * in three dimensions.
 * 
 * @param p1 a ::GtsPoint;
 * @param p2 a ::GtsPoint;
 * @param p3 a ::GtsPoint.
 * 
 * @return TRUE if \a p1, \a p2 and \a p3 are collinear, FALSE otherswise. 
 */

gboolean gtv_points_are_collinear(GtsPoint *p1,
				  GtsPoint *p2,
				  GtsPoint *p3)

{
/*   gdouble x1[4], x2[4], x3[4] ; */

  g_return_val_if_fail(p1 != NULL, FALSE) ;
  g_return_val_if_fail(GTS_IS_POINT(p1), FALSE) ;
  g_return_val_if_fail(p2 != NULL, FALSE) ;
  g_return_val_if_fail(GTS_IS_POINT(p2), FALSE) ;
  g_return_val_if_fail(p3 != NULL, FALSE) ;
  g_return_val_if_fail(GTS_IS_POINT(p3), FALSE) ;

/*   x1[0] = x1[3] = p1->x ; x1[1] = p1->y ; x1[2] = p1->z ; */
/*   x2[0] = x2[3] = p2->x ; x2[1] = p2->y ; x2[2] = p2->z ; */
/*   x3[0] = x3[3] = p3->x ; x3[1] = p3->y ; x3[2] = p3->z ; */
/*   if ( orient2d(&x1[0], &x2[0], &x3[0]) != 0.0 ) return FALSE ; */
/*   if ( orient2d(&x1[1], &x2[1], &x3[1]) != 0.0 ) return FALSE ; */
/*   if ( orient2d(&x1[2], &x2[2], &x3[2]) != 0.0 ) return FALSE ; */
  if ( orient2d(&(p1->x), &(p2->x), &(p3->x)) != 0.0 ) return FALSE ;
  if ( orient2d(&(p1->y), &(p2->y), &(p3->y)) != 0.0 ) return FALSE ;

  return TRUE ;
}

/**
 * @}
 * 
 */
