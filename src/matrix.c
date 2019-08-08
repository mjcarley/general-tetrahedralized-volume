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

#include <glib.h>

#include "gtv-private.h"

#define M411  0
#define M412  1
#define M413  2
#define M414  3
#define M421  4
#define M422  5
#define M423  6
#define M424  7
#define M431  8
#define M432  9
#define M433 10
#define M434 11
#define M441 12
#define M442 13
#define M443 14
#define M444 15


inline void invert3x3(gdouble *Ai, gdouble *A)

{
  gdouble det ;

  det = 
    A[0]*(A[8]*A[4]-A[7]*A[5]) - 
    A[3]*(A[8]*A[1]-A[7]*A[2]) +
    A[6]*(A[5]*A[1]-A[4]*A[2]) ;
  det = 1.0/det ;

  Ai[0] =  det*(A[8]*A[4] - A[7]*A[5]) ;
  Ai[1] = -det*(A[8]*A[1] - A[7]*A[2]) ;
  Ai[2] =  det*(A[5]*A[1] - A[4]*A[2]) ;

  Ai[3] = -det*(A[8]*A[3] - A[6]*A[5]) ;
  Ai[4] =  det*(A[8]*A[0] - A[6]*A[2]) ;
  Ai[5] = -det*(A[5]*A[0] - A[3]*A[2]) ;

  Ai[6] =  det*(A[7]*A[3] - A[6]*A[4]) ;
  Ai[7] = -det*(A[7]*A[0] - A[6]*A[1]) ;
  Ai[8] =  det*(A[4]*A[0] - A[3]*A[1]) ;

  return ;
}

void invert4x4(gdouble *Ai, gdouble *A)

{
  gdouble det ;

  det = 
    A[M411]*A[M422]*A[M433]*A[M444] + A[M411]*A[M423]*A[M434]*A[M442] + 
    A[M411]*A[M424]*A[M432]*A[M443] +
    A[M412]*A[M421]*A[M434]*A[M443] + A[M412]*A[M423]*A[M431]*A[M444] + 
    A[M412]*A[M424]*A[M433]*A[M441] +
    A[M413]*A[M421]*A[M432]*A[M444] + A[M413]*A[M422]*A[M434]*A[M441] + 
    A[M413]*A[M424]*A[M431]*A[M442] +
    A[M414]*A[M421]*A[M433]*A[M442] + A[M414]*A[M422]*A[M431]*A[M443] + 
    A[M414]*A[M423]*A[M432]*A[M441] -
    A[M411]*A[M422]*A[M434]*A[M443] - A[M411]*A[M423]*A[M432]*A[M444] -
    A[M411]*A[M424]*A[M433]*A[M442] -
    A[M412]*A[M421]*A[M433]*A[M444] - A[M412]*A[M423]*A[M434]*A[M441] -
    A[M412]*A[M424]*A[M431]*A[M443] -
    A[M413]*A[M421]*A[M434]*A[M442] - A[M413]*A[M422]*A[M431]*A[M444] -
    A[M413]*A[M424]*A[M432]*A[M441] -
    A[M414]*A[M421]*A[M432]*A[M443] - A[M414]*A[M422]*A[M433]*A[M441] -
    A[M414]*A[M423]*A[M431]*A[M442] ;

  det = 1.0/det ;

  Ai[M411] = 
    A[M422]*A[M433]*A[M444] + A[M423]*A[M434]*A[M442] + 
    A[M424]*A[M432]*A[M443] -
    A[M422]*A[M434]*A[M443] - A[M423]*A[M432]*A[M444] - 
    A[M424]*A[M433]*A[M442] ;
  Ai[M412] = 
    A[M412]*A[M434]*A[M443] + A[M413]*A[M432]*A[M444] + 
    A[M414]*A[M433]*A[M442] -
    A[M412]*A[M433]*A[M444] - A[M413]*A[M434]*A[M442] - 
    A[M414]*A[M432]*A[M443] ;
  Ai[M413] = 
    A[M412]*A[M423]*A[M444] + A[M413]*A[M424]*A[M442] + 
    A[M414]*A[M422]*A[M443] -
    A[M412]*A[M424]*A[M443] - A[M413]*A[M422]*A[M444] - 
    A[M414]*A[M423]*A[M442] ;
  Ai[M414] = 
    A[M412]*A[M424]*A[M433] + A[M413]*A[M422]*A[M434] + 
    A[M414]*A[M423]*A[M432] -
    A[M412]*A[M423]*A[M434] - A[M413]*A[M424]*A[M432] - 
    A[M414]*A[M422]*A[M433] ;
  Ai[M421] = 
    A[M421]*A[M434]*A[M443] + A[M423]*A[M431]*A[M444] + 
    A[M424]*A[M433]*A[M441] -
    A[M421]*A[M433]*A[M444] - A[M423]*A[M434]*A[M441] - 
    A[M424]*A[M431]*A[M443] ;
  Ai[M422] = 
    A[M411]*A[M433]*A[M444] + A[M413]*A[M434]*A[M441] + 
    A[M414]*A[M431]*A[M443] -
    A[M411]*A[M434]*A[M443] - A[M413]*A[M431]*A[M444] - 
    A[M414]*A[M433]*A[M441] ;
  Ai[M423] = 
    A[M411]*A[M424]*A[M443] + A[M413]*A[M421]*A[M444] +
    A[M414]*A[M423]*A[M441] -
    A[M411]*A[M423]*A[M444] - A[M413]*A[M424]*A[M441] - 
    A[M414]*A[M421]*A[M443] ;
  Ai[M424] = 
    A[M411]*A[M423]*A[M434] + A[M413]*A[M424]*A[M431] + 
    A[M414]*A[M421]*A[M433] -
    A[M411]*A[M424]*A[M433] - A[M413]*A[M421]*A[M434] - 
    A[M414]*A[M423]*A[M431] ;
  Ai[M431] = 
    A[M421]*A[M432]*A[M444] + A[M422]*A[M434]*A[M441] + 
    A[M424]*A[M431]*A[M442] -
    A[M421]*A[M434]*A[M442] - A[M422]*A[M431]*A[M444] - 
    A[M424]*A[M432]*A[M441] ;
  Ai[M432] = 
    A[M411]*A[M434]*A[M442] + A[M412]*A[M431]*A[M444] +
    A[M414]*A[M432]*A[M441] -
    A[M411]*A[M432]*A[M444] - A[M412]*A[M434]*A[M441] - 
    A[M414]*A[M431]*A[M442] ;
  Ai[M433] = 
    A[M411]*A[M422]*A[M444] + A[M412]*A[M424]*A[M441] +
    A[M414]*A[M421]*A[M442] -
    A[M411]*A[M424]*A[M442] - A[M412]*A[M421]*A[M444] - 
    A[M414]*A[M422]*A[M441] ;
  Ai[M434] = 
    A[M411]*A[M424]*A[M432] + A[M412]*A[M421]*A[M434] +
    A[M414]*A[M422]*A[M431] -
    A[M411]*A[M422]*A[M434] - A[M412]*A[M424]*A[M431] - 
    A[M414]*A[M421]*A[M432] ;
  Ai[M441] = 
    A[M421]*A[M433]*A[M442] + A[M422]*A[M431]*A[M443] + 
    A[M423]*A[M432]*A[M441] -
    A[M421]*A[M432]*A[M443] - A[M422]*A[M433]*A[M441] - 
    A[M423]*A[M431]*A[M442] ;
  Ai[M442] = 
    A[M411]*A[M432]*A[M443] + A[M412]*A[M433]*A[M441] + 
    A[M413]*A[M431]*A[M442] -
    A[M411]*A[M433]*A[M442] - A[M412]*A[M431]*A[M443] - 
    A[M413]*A[M432]*A[M441] ;
  Ai[M443] = 
    A[M411]*A[M423]*A[M442] + A[M412]*A[M421]*A[M443] + 
    A[M413]*A[M422]*A[M441] -
    A[M411]*A[M422]*A[M443] - A[M412]*A[M423]*A[M441] - 
    A[M413]*A[M421]*A[M442] ;
  Ai[M444] = 
    A[M411]*A[M422]*A[M433] + A[M412]*A[M423]*A[M431] + 
    A[M413]*A[M421]*A[M432] -
    A[M411]*A[M423]*A[M432] - A[M412]*A[M421]*A[M433] - 
    A[M413]*A[M422]*A[M431] ;

  Ai[M411] *= det ; Ai[M412] *= det ; Ai[M413] *= det ; Ai[M414] *= det ;
  Ai[M421] *= det ; Ai[M422] *= det ; Ai[M423] *= det ; Ai[M424] *= det ;
  Ai[M431] *= det ; Ai[M432] *= det ; Ai[M433] *= det ; Ai[M434] *= det ;
  Ai[M441] *= det ; Ai[M442] *= det ; Ai[M443] *= det ; Ai[M444] *= det ;

  return ;
}

