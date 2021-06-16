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

#ifndef GTV_H_INCLUDED
#define GTV_H_INCLUDED

#include <gts.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  /*
    Taken from gts.h which has the following note
   */

  /* Added based on glib.h by M J Loehr 01/01/01 */
  /* GTS version.
   * we prefix variable declarations so they can
   * properly get exported in windows dlls.
   */
#ifdef NATIVE_WIN32
#  ifdef GTV_COMPILATION
#    define GTV_C_VAR __declspec(dllexport)
#  else /* not GTV_COMPILATION */
#    define GTV_C_VAR extern __declspec(dllimport)
#  endif /* not GTV_COMPILATION */
#else /* not NATIVE_WIN32 */
#  define GTV_C_VAR extern
#endif /* not NATIVE_WIN32 */

/*basic definitions*/

  /**
   * @defgroup status Status codes
   * @{
   * 
   */

  /**
   * Status codes returned by GTV functions.
   */
  typedef enum {
    GTV_FAILURE =           -1,	/**< unspecified failure */
    GTV_SUCCESS =            0,	/**< function succeeded */
    GTV_NULL_ARGUMENT =      1,	/**< an argument was NULL */
    GTV_WRONG_TYPE =         2,	/**< an argument was of the wrong type */
    GTV_VERTEX_PRESENT =     3,	/**< vertex already present in tetrahedralization */
    GTV_COINCIDENT_VERTEX =  4,	/**< vertex in tetrahedralization has the same coordinates */
    GTV_VERTEX_NOT_IN_CELL = 5,	/**< the vertex to be added is not inside the cell */
    GTV_REPEATED_ARGUMENT = 6, /**< two or more input arguments are identical */
    GTV_VERTEX_ON_HULL =     7,	/**< vertex lies on the convex hull of a tetrahedralization */
    GTV_VERTEX_NOT_IN_VOLUME = 8, /**< vertex is not contained in volume */
    GTV_UNKNOWN_FORMAT = 9, /**< unrecognized file format */
    GTV_ARGUMENT_OUT_OF_RANGE = 10 /**< argument outside valid range*/
  } GtvStatus ;

  /**
   * Status codes returned by GTV functions which check if a vertex
   * inside, outside or on the surface of a simplex, e.g. a tetrahedron.
   */

typedef enum {
  GTV_OUT =      -1, /// vertex lies outside tetrahedron
  GTV_ON =        0, /// vertex lies on tetrahedron surface
  GTV_IN =        1, /// vertex lies strictly inside tetrahedron
  GTV_ON_FACET =  2, /// vertex lies on a facet of tetrahedron
  GTV_ON_EDGE =   3, /// vertex lies on an edge of tetrahedron
  GTV_ON_VERTEX = 4  /// vertex lies on a vertex of tetrahedron
} GtvIntersect ;

/**
 * @}
 * 
 */


  /**
   * @struct GtvFacet
   * @ingroup facet
   * Triangular facet used to form ::GtvTetrahedron's.
   */

typedef struct _GtvFacet            GtvFacet;

  /**
   * @struct GtvFacetClass
   * @ingroup facet
   * The basic facet class, derived from the ::GtsTriangleClass.
   */
typedef struct _GtvFacetClass       GtvFacetClass;

  /**
   * @struct GtvTetrahedron
   * @ingroup tetrahedron
   * Basic tetrahedral cell made up of four ::GtvFacet's. 
   */

typedef struct _GtvTetrahedron      GtvTetrahedron ;

  /**
   * @struct GtvTetrahedronClass
   * @ingroup tetrahedron
   * The basic tetrahedron class.
   */

typedef struct _GtvTetrahedronClass GtvTetrahedronClass ;

  /**
   * @struct GtvCell
   * @ingroup cell
   * Basic tetrahedral cell derived from ::GtvTetrahedron, used to 
   * build up ::GtvVolume's.
   * 
   */

typedef struct _GtvCell             GtvCell;

  /**
   * @struct GtvCellClass
   * @ingroup cell
   * The basic cell class, derived from the ::GtvTetrahedronClass.
   */

typedef struct _GtvCellClass        GtvCellClass;

  /**
   * @struct GtvVolume
   * @ingroup volume
   * Opaque data structure for the GTV volume. 
   * 
   */

typedef struct _GtvVolume           GtvVolume;

/**
 * @struct GtvVolumeClass
 * The basic class for a GtvVolume
 * @ingroup volume
 * 
 */

typedef struct _GtvVolumeClass      GtvVolumeClass;

  /**
   * @struct GtvOctTreeBox
   * @ingroup octree
   * Opaque data structure for octtrees, used in fast multipole calculations
   * 
   */

typedef struct _GtvOctTreeBox         GtvOctTreeBox;

/**
 * @struct GtvOctTreeBoxClass
 * The basic class for a GtvOctTreeBox
 * @ingroup octtree
 * 
 */

typedef struct _GtvOctTreeBoxClass      GtvOctTreeBoxClass;

#define GTV_FACET(obj)            GTS_OBJECT_CAST(obj,\
					         GtvFacet,\
					         gtv_facet_class())
#define GTV_FACET_CLASS(klass)    GTS_OBJECT_CLASS_CAST(klass,\
						 GtvFacetClass,\
						 gtv_facet_class())
#define GTV_IS_FACET(obj)        (gts_object_is_from_class(obj,\
						 gtv_facet_class()))

#define GTV_IS_TETRAHEDRON(obj)(gts_object_is_from_class(obj,\
						gtv_tetrahedron_class()))
#define GTV_TETRAHEDRON(obj)     GTS_OBJECT_CAST(obj,\
						  GtvTetrahedron,\
						  gtv_tetrahedron_class())
#define GTV_TETRAHEDRON_CLASS(klass) GTS_OBJECT_CLASS_CAST(klass,\
						 GtvTetrahedronClass,\
						 gtv_tetrahedron_class())
#define GTV_CELL(obj)            GTS_OBJECT_CAST(obj,\
					         GtvCell,\
					         gtv_cell_class())
#define GTV_CELL_CLASS(klass)    GTS_OBJECT_CLASS_CAST(klass,\
						 GtvCellClass,\
						 gtv_cell_class())
#define GTV_IS_CELL(obj)        (gts_object_is_from_class(obj,\
						 gtv_cell_class()))
#define GTV_VOLUME(obj)            GTS_OBJECT_CAST(obj,\
					         GtvVolume,\
					         gtv_volume_class())
#define GTV_VOLUME_CLASS(klass)    GTS_OBJECT_CLASS_CAST(klass,\
						 GtvVolumeClass,\
						 gtv_volume_class())
#define GTV_IS_VOLUME(obj)        (gts_object_is_from_class(obj,\
						 gtv_volume_class()))
#define GTV_OCT_TREE_BOX(obj)            GTS_OBJECT_CAST (obj,\
					         GtvOctTreeBox,\
					         gtv_oct_tree_box_class ())
#define GTV_OCT_TREE_BOX_CLASS(klass)    GTS_OBJECT_CLASS_CAST (klass,\
						 GtvOctTreeBoxClass,\
						 gtv_oct_tree_box_class())
#define GTV_IS_OCT_TREE_BOX(obj)         (gts_object_is_from_class (obj,\
						 gtv_oct_tree_box_class ()))
#define GTV_IS_OCT_TREE_BOX_CLASS(class) \
  (gts_object_class_is_from_class (class,gtv_oct_tree_box_class()) != NULL)

struct _GtvFacet {
  /*< private >*/
  GtsTriangle triangle ;

  /*< public >*/
  GSList *tetrahedra ;
};

struct _GtvFacetClass {
  /*< private >*/
  GtsTriangleClass parent_class;

  /*< public >*/
  /* add extra methods here */
};

struct _GtvTetrahedron {
  GtsObject object;

  GtvFacet *f1 ;
  GtvFacet *f2 ;
  GtvFacet *f3 ;
  GtvFacet *f4 ;
};

struct _GtvTetrahedronClass {
  GtsObjectClass parent_class;
};

struct _GtvCell {
  /*< private >*/
  GtvTetrahedron tetrahedron ;

  /*< public >*/
  GSList *volumes ;
};

struct _GtvCellClass {
  /*< private >*/
  GtvTetrahedronClass parent_class;

  /*< public >*/
  /* add extra methods here */
};

struct _GtvVolume {
  GtsObject object;

  GHashTable *cells ;
  GtvCellClass *cell_class ;
  GtvFacetClass *facet_class ;
  GtsEdgeClass *edge_class ;
  GtsVertexClass *vertex_class ;
  gboolean keep_cells ;
};

struct _GtvVolumeClass {
  GtsObjectClass parent_class ;
};

struct _GtvOctTreeBox {
  /*< private >*/
  GtsBBox parent;
  GtvOctTreeBoxClass *klass ;

  /*< public >*/
  /* add extra data here (if public) */
  GSList *elements ;
  GHashTable *cfft ;
  GArray *Imnk, 
    *dImnk, *B ;
  gint stride, order ;
  GtsPoint *xc ;
};

struct _GtvOctTreeBoxClass {
  /*< private >*/
  GtsBBoxClass parent_class;

  /*< public >*/
  /* add extra methods here */
};

#ifdef DOXYGEN_BLOCK
  /**
   * @struct GtvVolumeStats
   * @ingroup volume
   * Data structure containing statistics for a ::GtvVolume
   *
   */

typedef struct {
  guint n_cells;		/**< number of cells in volume */
  guint n_incompatible_cells;	/**< number of incompatible cells */
  guint n_duplicate_cells;	/**< number of duplicate cells */
  guint n_facets;		/**< number of facets */
  guint n_incompatible_facets;	/**< number of incompatible facets */
  guint n_duplicate_facets;	/**< number of duplicate facets */
  guint n_boundary_facets;	/**< number of facets on boundary of volume */
  guint n_duplicate_edges;	/**< number of duplicate edges */
  GtsRange edges_per_vertex,	/**< statistics for edges per vertex */
    facets_per_edge,		/**< statistics for facets per edge */
    cells_per_facet ; 	        /**< statistics for cells per facet */
  GtvVolume *parent;		/**< ::GtvVolume from which these statistics have been derived */
} GtvVolumeStats ;

#else
typedef struct _GtvVolumeStats      GtvVolumeStats ;

struct _GtvVolumeStats {
  guint n_cells;
  guint n_incompatible_cells;
  guint n_duplicate_cells;
  guint n_facets;
  guint n_incompatible_facets;
  guint n_duplicate_facets;
  guint n_boundary_facets;
  guint n_duplicate_edges;
  GtsRange edges_per_vertex, facets_per_edge, cells_per_facet ;
  GtvVolume *parent;  
} ;
#endif /*DOXYGEN_BLOCK*/

#ifdef DOXYGEN_BLOCK
  /**
   * @struct GtvVolumeQualityStats
   * @ingroup volume
   * Data structure containing quality statistics for a ::GtvVolume
   *
   */

  typedef struct {
    GtsRange cell_quality ; /**< statistics for cell quality (see ::gtv_tetrahedron_quality)*/
    GtsRange cell_volume ;  /**< statistics for cell volume (see ::gtv_tetrahedron_volume)*/
    GtsRange facet_quality; /**< statistics for facet quality (see ::gts_triangle_quality)*/
    GtsRange facet_area;    /**< statistics for facet area (see ::gts_triangle_area)*/
    GtsRange edge_length;   /**< statistics for edge length*/
    GtsRange edge_angle;    /**< statistics for edge angles*/
    GtvVolume *parent;      /**< ::GtvVolume for which these statistics have been obtained*/
  } GtvVolumeQualityStats ;
#else
typedef struct _GtvVolumeQualityStats      GtvVolumeQualityStats ;
  struct _GtvVolumeQualityStats {
    GtsRange cell_quality ;
    GtsRange cell_volume ;
    GtsRange facet_quality;
    GtsRange facet_area;
    GtsRange edge_length;
    GtsRange edge_angle;
    GtvVolume *parent;  
  } ;
#endif /*DOXYGEN_BLOCK*/

  GTV_C_VAR gboolean gtv_allow_floating_facets ;
  GTV_C_VAR gboolean gtv_allow_floating_cells ;

  /*Quadrature struct and macros*/

  /**
   * @struct GtvQuadratureRule
   * Quadrature rules for integration on tetrahedra.
   * @ingroup quadrature
   * 
   */

  typedef struct _GtvQuadratureRule GtvQuadratureRule ;

  struct _GtvQuadratureRule {
    GtvTetrahedron *t ;
    GtsVertex *v1, *v2, *v3, *v4 ;
    GArray *rule ;
  } ;

  /**
   * @typedef GtvLookupFunc
   * @ingroup multipole
   *
   * Lookup function for calculating multipole moments. If \a func is
   * a ::GtvLookupFunc then \a func(\a v, \a data) returns a gpointer
   * to data associated with \a v.
   *
   * @param v a ::GtsVertex
   * @param data user data passed by the calling GTV function
   *
   * @return a pointer of type ::gpointer to data which will be used
   * in calculating a multipole expansion.
   * 
   */

  typedef gpointer (*GtvLookupFunc)(GtsVertex *v, gpointer data) ;

/**
 * @addtogroup quadrature
 * @{
 * 
 */

/**
 * Quadrature types built in to GTV.
 */

  typedef enum {
    GTV_QUADRATURE_KEAST_1, /**< Keast four point rule*/
    GTV_QUADRATURE_ZIENKIEWICZ_4, /**< Zienkiewicz four point rule*/
    GTV_QUADRATURE_ZIENKIEWICZ_5 /**< Zienkiewicz five point rule*/
  } GtvQuadrature ;

  /**
   * The number of points in a ::GtvQuadratureRule q.
   * @hideinitializer
   */

#define gtv_quadrature_rule_length(q)		\
  ((q->rule->len)/4)

  /**
   * The first normalized coordinate of the \a i th point
   * of a ::GtvQuadratureRule \a q.
   * @hideinitializer
   */

#define gtv_quadrature_rule_xi1(q,i)		\
  (g_array_index(q->rule,gdouble,4*(i)+0))

  /**
   * The second normalized coordinate of the \a i th point
   * of a ::GtvQuadratureRule \a q.
   * @hideinitializer
   */

#define gtv_quadrature_rule_xi2(q,i)		\
  (g_array_index(q->rule,gdouble,4*(i)+1))

  /**
   * The third normalized coordinate of the \a i th point
   * of a ::GtvQuadratureRule \a q.
   * @hideinitializer
   */

#define gtv_quadrature_rule_xi3(q,i)		\
  (g_array_index(q->rule,gdouble,4*(i)+2))

  /**
   * The fourth normalized coordinate of the \a i th point
   * of a ::GtvQuadratureRule \a q.
   * @hideinitializer
   */

#define gtv_quadrature_rule_xi4(q,i)			\
  (1.0-g_array_index(q->rule,gdouble,4*(i)+0)-	\
   g_array_index(q->rule,gdouble,4*(i)+1)-		\
   g_array_index(q->rule,gdouble,4*(i)+2))

  /**
   * The weight of the \a i th point of a ::GtvQuadratureRule \a q.
   *
   * @hideinitializer
   */

#define gtv_quadrature_rule_weight(q,i)		\
  (g_array_index((q)->rule,gdouble,(4*(i)+3)))

/**
 * @}
 * 
 */

  typedef struct _GtvEvaluationStats GtvEvaluationStats ;

  struct _GtvEvaluationStats {
    gint depth, ndirect, ncalc[32] ;
  } ;

#define gtv_evaluation_stats_depth(s) ((s)->depth)
#define gtv_evaluation_stats_box_number(s,depth) ((s)->ncalc[(depth)])
#define gtv_evaluation_stats_direct_number(s) ((s)->ndirect)
#define gtv_evaluation_stats_clear(s) (memset((s)->ncalc,0,(32*sizeof(gint))))

/* Facets: facet.c */

GtvFacetClass *gtv_facet_class(void) ;
GtvFacet *gtv_facet_new(GtvFacetClass *klass, 
			GtsEdge *e1, 
			GtsEdge *e2,
			GtsEdge *e3) ;
GtvCell *gtv_facet_is_boundary(GtvFacet *f, GtvVolume *v) ;
  GtvCell *gtv_edge_is_boundary(GtsEdge *e, GtvVolume *v) ;
  GtvCell *gtv_vertex_is_boundary(GtsVertex *p, GtvVolume *v) ;
GtvFacet *gtv_facet_is_duplicate(GtvFacet *f) ;
  gint gtv_facet_replace(GtvFacet *f, GtvFacet *with) ;
gint gtv_facet_tetrahedra_vertices(GtvFacet *f, GtsVertex **v1,
				   GtsVertex **v2, GtsVertex **v3,
				   GtsVertex **v4, GtsVertex **v5) ;
gint gtv_facet_tetrahedra(GtvFacet *f, GtvTetrahedron **t1,
			  GtvTetrahedron **t2) ;
GtvFacet *gtv_facet_from_vertices(GtsVertex *v1,
				  GtsVertex *v2,
				  GtsVertex *v3) ;
  gboolean gtv_facet_has_vertex(GtvFacet *f, GtsVertex *v) ;
  gboolean gtv_facet_has_edge(GtvFacet *f, GtsEdge *e) ;
/* Tetrahedra: tetrahedron.c */

GtvTetrahedronClass *gtv_tetrahedron_class(void) ;
gdouble gtv_point_in_tetrahedron_sphere(GtsPoint *p, GtvTetrahedron *t) ;
gint gtv_tetrahedron_revert(GtvTetrahedron *t) ;
  gint gtv_tetrahedron_orient(GtvTetrahedron *t) ;
gint gtv_tetrahedron_set(GtvTetrahedron *tetrahedron,
			 GtvFacet *f1,
			 GtvFacet *f2,
			 GtvFacet *f3,
			 GtvFacet *f4) ;
GtvTetrahedron *gtv_tetrahedron_new(GtvTetrahedronClass *klass,
				    GtvFacet *f1,
				    GtvFacet *f2,
				    GtvFacet *f3,
				    GtvFacet *f4) ;
GtsVertex *gtv_tetrahedron_apex(GtvTetrahedron *t) ;
gint gtv_tetrahedron_vertices(GtvTetrahedron *t,
			      GtsVertex **v1,
			      GtsVertex **v2,
			      GtsVertex **v3,
			      GtsVertex **v4) ;
gdouble gtv_tetrahedron_volume(GtvTetrahedron *t) ;
GtvTetrahedron *gtv_tetrahedron_from_facets(GtvFacet *f1,
					    GtvFacet *f2,
					    GtvFacet *f3,
					    GtvFacet *f4) ;
gboolean gtv_tetrahedron_has_facet(GtvTetrahedron *t,
				   GtvFacet *f) ;
GtvTetrahedron *gtv_tetrahedron_is_duplicate(GtvTetrahedron *t) ;
GtvIntersect gtv_point_in_tetrahedron(GtsPoint *p, 
				      GtvTetrahedron *t,
				      gpointer *s) ;
GtvTetrahedron *gtv_tetrahedron_large(GtvTetrahedronClass *klass,
				      GtvFacetClass *facet_class,
				      GtsEdgeClass *edge_class,
				      GtsVertexClass *vertex_class,
				      gdouble len) ;
GtvTetrahedron *gtv_tetrahedron_equilateral(GtvTetrahedronClass *klass,
					    GtvFacetClass *facet_class,
					    GtsEdgeClass *edge_class,
					    GtsVertexClass *vertex_class,
					    gdouble len) ;
GtvFacet *gtv_tetrahedra_common_facet(GtvTetrahedron *t1,
				      GtvTetrahedron *t2) ;
GtvTetrahedron *gtv_tetrahedron_opposite(GtvTetrahedron *t,
					 GtvFacet *f) ;
gdouble gtv_tetrahedron_orientation(GtvTetrahedron *t) ;
gint gtv_tetrahedron_facets(GtvTetrahedron *t,
			    GtvFacet **f1, GtvFacet **f2,
			    GtvFacet **f3, GtvFacet **f4) ;
  gint gtv_tetrahedron_invert(GtvTetrahedron *t) ;
  GtsVertex *gtv_tetrahedron_vertex_opposite(GtvTetrahedron *t,
					     GtvFacet *f) ;
GtvFacet *gtv_tetrahedron_facet_opposite(GtvTetrahedron *t,
					 GtsVertex *v) ;
  gboolean gtv_tetrahedron_is_okay(GtvTetrahedron *t) ;
  GtvTetrahedron *gtv_tetrahedron_new_from_vertices(GtvTetrahedronClass *klass,
						    GtvFacetClass *facet_class,
						    GtsEdgeClass *edge_class,
						    GtsVertex *v1,
						    GtsVertex *v2,
						    GtsVertex *v3,
						    GtsVertex *v4) ;
GtvTetrahedron *gtv_tetrahedron_from_vertices(GtsVertex *v1,
					      GtsVertex *v2,
					      GtsVertex *v3,
					      GtsVertex *v4) ;
  gboolean gtv_tetrahedron_has_edge(GtvTetrahedron *t, GtsEdge *e) ;
  gboolean gtv_tetrahedron_has_vertex(GtvTetrahedron *t, GtsVertex *v) ;
GtvTetrahedron *gtv_tetrahedra_wedge_neighbour(GtvTetrahedron *t1,
					       GtvTetrahedron *t2) ;
GtvFacet *gtv_point_in_tetrahedron_facet(GtvTetrahedron *t, 
					 GtsPoint *p) ;
gint gtv_tetrahedron_vertices_off_edge(GtvTetrahedron *t,
				       GtsEdge *e,
				       GtsVertex **a,
				       GtsVertex **b) ;
gint gtv_tetrahedron_opposite_vertices(GtvTetrahedron *t,
				       GtsVertex *v,
				       GtsVertex **v1,
				       GtsVertex **v2,
				       GtsVertex **v3) ;
  GtsVertex *gtv_tetrahedron_is_encroached(GtvTetrahedron *t) ;
  gdouble gtv_tetrahedron_circumradius(GtvTetrahedron *t) ;
  gdouble gtv_tetrahedron_inradius(GtvTetrahedron *t) ;
  gdouble gtv_tetrahedron_quality_radius_ratio(GtvTetrahedron *t) ;
  GtsBBox *gtv_bbox_tetrahedra(GtsBBoxClass *klass, GSList *tetrahedra) ;
  gint gtv_tetrahedron_centroid(GtvTetrahedron *t, GtsPoint *c) ;
  gdouble gtv_tetrahedron_radius_ratio(GtvTetrahedron *t) ;

  /**
   * A convenience macro for the quality measure of a
   * tetrahedron. This actually calls ::gtv_tetrahedron_radius_ratio,
   * although the behaviour may change in future.
   * @hideinitializer
   * @addtogroup tetrahedron
   */

#define gtv_tetrahedron_quality(t) gtv_tetrahedron_radius_ratio(t)

/* Cells: cell.c */

GtvCellClass *gtv_cell_class (void);
GtvCell *gtv_cell_new(GtvCellClass *klass,
		      GtvFacet *f1,
		      GtvFacet *f2,
		      GtvFacet *f3,
		      GtvFacet *f4) ;
GSList *gtv_cell_neighbours(GtvCell *c, GtvVolume *v) ;
GtvCell *gtv_cell_new_from_vertices(GtvCellClass *klass,
				    GtvFacetClass *facet_class,
				    GtsEdgeClass *edge_class,
				    GtsVertex *v1,
				    GtsVertex *v2,
				    GtsVertex *v3,
				    GtsVertex *v4) ;
  GSList *gtv_edge_cells(GtsEdge *e, GtvVolume *v) ;
  GSList *gtv_vertex_cells(GtsVertex *p, GtvVolume *v) ;

/* Volumes: volume.c */

GtvVolumeClass *gtv_volume_class (void);
gint gtv_volume_clean(GtvVolume *v) ;
GtvVolume *gtv_volume_new(GtvVolumeClass *klass,
			  GtvCellClass *cell_class,
			  GtvFacetClass *facet_class,
			  GtsEdgeClass *edge_class,
			  GtsVertexClass *vertex_class) ;
gint gtv_volume_add_cell(GtvVolume *v, GtvCell *c) ;
gint gtv_volume_remove_cell(GtvVolume *v, GtvCell *c) ;
gint gtv_volume_write(GtvVolume *v, FILE *f) ;
guint gtv_volume_read(GtvVolume *v, GtsFile *f) ;
  guint gtv_volume_read_gmsh(FILE *f, GtvVolume *v) ;
  gint gtv_volume_write_gmsh(GtvVolume *v, FILE *f) ;
  gint gtv_volume_write_gmsh1(GtvVolume *v, FILE *f) ;
gint gtv_volume_foreach_cell(GtvVolume *v, GtsFunc func, gpointer data) ;
gint gtv_volume_foreach_facet(GtvVolume *v, GtsFunc func, gpointer data) ;
gint gtv_volume_foreach_vertex(GtvVolume *v, GtsFunc func, gpointer data) ;
gint gtv_volume_foreach_edge(GtvVolume *v, GtsFunc func, gpointer data) ;
  gint gtv_volume_quality_stats(GtvVolume *v, GtvVolumeQualityStats *s) ;
gint gtv_volume_stats(GtvVolume *v, GtvVolumeStats *s) ;
gint gtv_volume_print_stats(GtvVolume *v, FILE *f) ;
gint gtv_volume_boundary(GtvVolume *v, GtsSurface *s) ;
gdouble gtv_volume_volume(GtvVolume *v) ;
guint gtv_volume_vertex_number(GtvVolume *v) ;
  guint gtv_volume_cell_number(GtvVolume *v) ;
  GtsVertex *gtv_volume_nearest_vertex(GtvVolume *v, GtsPoint *p) ;
  gint gtv_volume_write_tetgen(GtvVolume *v, gchar *stub) ;

  GtsEdge *gtv_vertex_has_parent_volume(GtsVertex *p, GtvVolume *v) ;
GtvFacet *gtv_edge_has_parent_volume(GtsEdge *e, GtvVolume *v) ;
GtvCell *gtv_facet_has_parent_volume(GtvFacet *f, GtvVolume *v) ;
gboolean gtv_cell_has_parent_volume(GtvCell *c, GtvVolume *v) ;
guint gtv_edge_facet_number(GtsEdge *e, GtvVolume *v) ;
guint gtv_facet_cell_number(GtvFacet *f, GtvVolume *v) ;
  GtsBBox *gtv_bbox_volume(GtsBBoxClass *klass, GtvVolume *v) ;

  /*point location*/
  GtvCell *gtv_point_locate(GtsPoint *p, GtvVolume *v, GtvCell *guess) ;
  GtvCell *gtv_point_locate_slow(GtsPoint *p, GtvVolume *volume, 
				 GtvCell *guess) ;
  /*Delaunay*/
  gboolean gtv_facet_is_regular(GtvFacet *f) ;
  GtvCell *gtv_delaunay_check(GtvVolume *v) ;
gint gtv_delaunay_add_vertex_to_cell(GtvVolume *v,
				     GtsVertex *p,
				     GtvCell *c) ;
  gint gtv_delaunay_add_vertex(GtvVolume *v, GtsVertex *p, GtvCell *c) ;
  gint gtv_delaunay_remove_vertex(GtvVolume *v, GtsVertex *p) ;

  /*geometric tests*/
gdouble gtv_point_in_sphere(GtsPoint *p, 
			    GtsPoint *p1,
			    GtsPoint *p2,
			    GtsPoint *p3,
			    GtsPoint *p4) ;
gboolean gtv_points_are_collinear(GtsPoint *p1,
				  GtsPoint *p2,
				  GtsPoint *p3) ;
  gint gtv_delaunay_remove_vertex(GtvVolume *v, GtsVertex *p) ;
  /*Logging*/
gint gtv_logging_init(FILE *f, gchar *p, 
		      GLogLevelFlags log_level,
		      gpointer exit_func) ;

  /*Quadrature*/
  GtvQuadratureRule *gtv_quadrature_rule_new(void) ;
  gint gtv_quadrature_rule_free(GtvQuadratureRule *q) ;
  gint gtv_quadrature_rule_select(GtvQuadratureRule *q, 
				  GtvQuadrature quadrature) ;
  gint gtv_quadrature_rule_compute(GtvQuadratureRule *q,
				   GtvTetrahedron *t, GtsPoint *p, 
				   gint nh, gint nr, gint nt) ;
  gint gtv_quadrature_rule_interp_point(GtvQuadratureRule *q,
					GtsPoint *p1, GtsPoint *p2,
					GtsPoint *p3, GtsPoint *p4,
					gint i, GtsPoint *p) ;
  gint gtv_quadrature_rule_interp_vector(GtvQuadratureRule *q,
					 GtsVector v1, GtsVector v2,
					 GtsVector v3, GtsVector v4,
					 gint i, GtsVector v) ;

  gint gtv_quadrature_rule_interp_scalar(GtvQuadratureRule *q,
					 gdouble v1, gdouble v2,
					 gdouble v3, gdouble v4,
					 gint i, gdouble *v) ;

  /*Octtrees*/
  GtvOctTreeBoxClass *gtv_oct_tree_box_class  (void);
  GtvOctTreeBox *gtv_oct_tree_box_new    (GtvOctTreeBoxClass * klass);
  GNode *gtv_oct_tree_new(GtvOctTreeBoxClass *klass, GtvVolume *v,
			  gint tetmax) ;

  /*Fast multipole*/
  /**
   * @addtogroup multipole
   * @{
   * 
   */

  /**
   * Field types for multipole integrations.
   * 
   */
  typedef enum {
    GTV_FIELD_SCALAR,		/**< scalar field, e.g. potential */
    GTV_FIELD_CURL,		/**< curl field, e.g. Biot-Savart law*/
  } GtvFieldType ;

#if 0
  /**
   * Start of packed block of multipole coefficients of order \a
   * h. This is the also the total space required for coefficients up
   * to order (\a h - 1).
   *
   * @hideinitializer
   */

#define gtv_multipole_block_start(h) ((h)*((h)+1)*((h)+2)/6)

  /**
   * Location of coefficient (\a m, \a n, \a k) in packed block of
   * multipole coefficients of order \a m + \a n + \a k. To locate a
   * coefficient use
   * ::gtv_multipole_block_start(m+n+k)+::gtv_multipole_index_mnk(m,n,k).
   *
   * @hideinitializer
   */

#define gtv_multipole_index_mnk(m,n,k) (((m)+(n))*((m)+(n)+1)/2 + (m))

  /**
   * Location of coefficient (\a m, \a n, \a k) in a packed array of
   * multipole coefficients of all orders up to \a m + \a n + \a
   * k. The returned index is equal to
   * ::gtv_multipole_block_start(m+n+k)+::gtv_multipole_index_mnk(m,n,k)
   *
   * @hideinitializer
   */

#define gtv_multipole_index(m,n,k) \
  (gtv_multipole_block_start((m)+(n)+(k))+gtv_multipole_index_mnk((m),(n),(k)))
#endif

  /**
   * @}
   */

  gint gtv_multipole_box_moment_coefficients(GtvOctTreeBox *box, 
					     gint order) ;
  gint gtv_multipole_leaf_moment_coefficients(GNode *tree, 
					      gint order) ;
  gint gtv_multipole_box_moments(GtvOctTreeBox *box,
				 gint stride,
				 GtvLookupFunc func,
				 gpointer data) ;
  gint gtv_multipole_leaf_moments(GNode *tree, 
				  gint stride,
				  GtvLookupFunc func,
				  gpointer data) ;
  gint gtv_multipole_box_shift_moments(GtvOctTreeBox *dest, 
				       GtvOctTreeBox *source) ;
  gint gtv_multipole_tree_moments(GNode *tree) ;
  gint gtv_multipole_moments(GNode *tree, 
			     gint order,
			     gint stride,
			     GtvLookupFunc func,
			     gpointer data) ;
  gint gtv_multipole_distances(GtsVector r, gint a, gint H, GArray *dR) ;

gint gtv_multipole_field_amm(GNode *tree, 
			     gint a, 
			     GtvFieldType field, 
			     GtvLookupFunc func,
			     gpointer data, 
			     gdouble tol,
			     GtsPoint *x, gdouble *I,
			     GtvEvaluationStats *s) ;

/* extern const gint GTV_BINOMIALS[] ; */
/* extern const gdouble GTV_FACTORIALS[] ; */

#define gtv_binomial(m,k) (GTV_BINOMIALS[(m)*((m)+1)/2+(k)])
#define gtv_factorial(i) GTV_FACTORIALS[(i)]

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*GTV_H_INCLUDED*/
