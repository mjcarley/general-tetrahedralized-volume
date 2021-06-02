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

/**
 * @defgroup volume Volume
 * @{
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#include <math.h>
#include <stdlib.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <glib.h>

#include <gts.h>

#include "gtv.h"
#include "gtv-private.h"

gboolean gtv_allow_floating_cells = FALSE ;

static void destroy_cell(GtvCell *c, GtvVolume *v)

{
  c->volumes = g_slist_remove(c->volumes, v) ;

  if ( !GTS_OBJECT_DESTROYED(c) &&
       !gtv_allow_floating_cells && c->volumes == NULL ) 
    gts_object_destroy(GTS_OBJECT(c)) ;

  return ;
}

static void volume_destroy (GtsObject *object)

{
  GtvVolume *v = GTV_VOLUME(object) ;

  gtv_volume_foreach_cell(v, (GtsFunc)destroy_cell, v) ;

  g_hash_table_destroy(v->cells) ;

  (*GTS_OBJECT_CLASS(gtv_volume_class ())->parent_class->destroy) (object);

  return ;
}

static void volume_write (GtsObject *object, FILE *f)
{
  fprintf (f, " %s %s %s %s %s", 
	   object->klass->info.name,
	   GTS_OBJECT_CLASS (GTV_VOLUME (object)->cell_class)->info.name,
	   GTS_OBJECT_CLASS (GTV_VOLUME (object)->facet_class)->info.name,
	   GTS_OBJECT_CLASS (GTV_VOLUME (object)->edge_class)->info.name,
	   GTS_POINT_CLASS (GTV_VOLUME (object)->vertex_class)->binary ?
	   "GtsVertexBinary" :
	   GTS_OBJECT_CLASS (GTV_VOLUME (object)->vertex_class)->info.name);
}


static void gtv_volume_class_init (GtvVolumeClass * klass)
{
  /* define new methods and overload inherited methods here */
  GTS_OBJECT_CLASS (klass)->write = volume_write;
  GTS_OBJECT_CLASS (klass)->destroy = volume_destroy;
}

static void gtv_volume_init (GtvVolume * volume)
{
  volume->cells = g_hash_table_new(NULL, NULL) ;
  volume->cell_class = gtv_cell_class() ;
  volume->facet_class = gtv_facet_class() ;
  volume->edge_class = gts_edge_class() ;
  volume->vertex_class = gts_vertex_class() ;
  volume->keep_cells = FALSE ;
}

/** 
 * Generate the class definition for the GtvVolume type.
 * 
 * @return GtvVolumeClass.
 */

GtvVolumeClass * gtv_volume_class (void)
{
  static GtvVolumeClass * klass = NULL;

  if (klass == NULL) {
    GtsObjectClassInfo gtv_volume_info = {
      "GtvVolume",
      sizeof (GtvVolume),
      sizeof (GtvVolumeClass),
      (GtsObjectClassInitFunc) gtv_volume_class_init,
      (GtsObjectInitFunc) gtv_volume_init,
      (GtsArgSetFunc) NULL,
      (GtsArgGetFunc) NULL
    };
    klass = gts_object_class_new (gts_object_class(),
				  &gtv_volume_info);
  }

  return klass;
}

/** 
 * Generate a new GtvVolume
 * 
 * @param klass GtvVolumeClass for new volume
 * @param cell_class GtvCellClass
 * @param facet_class GtvFacetClass
 * @param edge_class GtsEdgeClass
 * @param vertex_class GtsVertexClass
 * 
 * @return the new GtvVolume
 */

GtvVolume * gtv_volume_new(GtvVolumeClass * klass,
			   GtvCellClass *cell_class,
			   GtvFacetClass *facet_class,
			   GtsEdgeClass *edge_class,
			   GtsVertexClass *vertex_class)

{
  GtvVolume *v ;

  v = GTV_VOLUME(gts_object_new (GTS_OBJECT_CLASS (klass)));
  v->cell_class = cell_class ;
  v->facet_class = facet_class ;
  v->edge_class = edge_class ;
  v->vertex_class = vertex_class ;
  
  return v ;
}

/** 
 * Add a GtvCell to a GtvVolume. 
 * 
 * @param v GtvVolume
 * @param c GtvCell
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_volume_add_cell(GtvVolume *v, GtvCell *c)

{
  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;

  g_return_val_if_fail(c != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_CELL(c), GTV_WRONG_TYPE) ;

  if (!g_hash_table_lookup (v->cells, c)) {
    c->volumes = g_slist_prepend (c->volumes, v);
    g_hash_table_insert (v->cells, c, c);
  } else
    g_message("%s: cell %p already present", __FUNCTION__, c) ;

  return GTV_SUCCESS ;
}

/** 
 * Remove a GtvCell from a GtvVolume. If gtv_allow_floating_cells is
 * FALSE, the cell will be destroyed if it is not used by any other
 * GtvVolume.
 * 
 * @param v GtvVolume;
 * @param c GtvCell to remove.
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_volume_remove_cell(GtvVolume *v, GtvCell *c)

{
  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(c != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_CELL(c), GTV_WRONG_TYPE) ;

  if ( !g_hash_table_lookup(v->cells, c) ) return GTV_SUCCESS ;
  
  g_hash_table_remove(v->cells, c) ;

  c->volumes = g_slist_remove(c->volumes, v) ;

  if (!GTS_OBJECT_DESTROYED(c) &&
      !gtv_allow_floating_cells &&
      c->volumes == NULL)
    gts_object_destroy(GTS_OBJECT(c)) ;

  return GTV_SUCCESS ;
}

static void write_vertex (GtsPoint *p, gpointer *data)

{
  (*GTS_OBJECT (p)->klass->write) (GTS_OBJECT (p), (FILE *) data[0]);
  if (!GTS_POINT_CLASS (GTS_OBJECT (p)->klass)->binary)
    fputc ('\n', (FILE *) data[0]);
  g_hash_table_insert (data[2], p, 
		       GUINT_TO_POINTER (++(*((guint *) data[1]))));
}

static void write_edge (GtsSegment * s, gpointer * data) 
{
  fprintf ((FILE *) data[0], "%u %u",
	   GPOINTER_TO_UINT (g_hash_table_lookup (data[2], s->v1)),
	   GPOINTER_TO_UINT (g_hash_table_lookup (data[2], s->v2)));
  if (GTS_OBJECT (s)->klass->write)
    (*GTS_OBJECT (s)->klass->write) (GTS_OBJECT (s), (FILE *) data[0]);
  fputc ('\n', (FILE *) data[0]);
  g_hash_table_insert (data[3], s, 
		       GUINT_TO_POINTER (++(*((guint *) data[1]))));
}

static void write_facet (GtsTriangle * t, gpointer * data)
{
  fprintf (data[0], "%u %u %u",
	   GPOINTER_TO_UINT (g_hash_table_lookup (data[3], t->e1)),
	   GPOINTER_TO_UINT (g_hash_table_lookup (data[3], t->e2)),
	   GPOINTER_TO_UINT (g_hash_table_lookup (data[3], t->e3)));
  if (GTS_OBJECT (t)->klass->write)
    (*GTS_OBJECT (t)->klass->write) (GTS_OBJECT (t), data[0]);
  fputc ('\n', data[0]);
  g_hash_table_insert (data[4], t, 
		       GUINT_TO_POINTER (++(*((guint *) data[1]))));
}

static void write_cell (GtvTetrahedron * t, gpointer * data)
{
  fprintf (data[0], "%u %u %u %u",
	   GPOINTER_TO_UINT (g_hash_table_lookup (data[4], t->f1)),
	   GPOINTER_TO_UINT (g_hash_table_lookup (data[4], t->f2)),
	   GPOINTER_TO_UINT (g_hash_table_lookup (data[4], t->f3)),
	   GPOINTER_TO_UINT (g_hash_table_lookup (data[4], t->f4)));
  if (GTS_OBJECT (t)->klass->write)
    (*GTS_OBJECT (t)->klass->write) (GTS_OBJECT (t), data[0]);
  fputc ('\n', data[0]);
}

/** 
 * Write a GtvVolume to file. The file's first line is the number of
 * vertices, edges, facets and cells respectively, followed by the
 * class of each in the volume. There then follow the vertex
 * coordinates, one vertex per line, and then the edges, facets and
 * cells.
 * 
 * @param v GtvVolume to write;
 * @param f file pointer. 
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_volume_write(GtvVolume *v, FILE *f)

{
  guint n ;
  gpointer data[5] ;
  GHashTable *vi, *ei, *fi ;
  GtvVolumeStats s ;
  
  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(f != NULL, GTV_NULL_ARGUMENT) ;

  data[0] = f ;
  data[1] = &n ;
  data[2] = vi = g_hash_table_new(NULL, NULL) ;
  data[3] = ei = g_hash_table_new(NULL, NULL) ;
  data[4] = fi = g_hash_table_new(NULL, NULL) ;

  gtv_volume_stats(v, &s) ;

  fprintf(f, "%u %u %u %u", 
	  s.edges_per_vertex.n,
	  s.facets_per_edge.n,
	  s.cells_per_facet.n,
	  s.n_cells) ;
  if (GTS_OBJECT (v)->klass->write)
    (*GTS_OBJECT (v)->klass->write) (GTS_OBJECT (v), f);  
  fputc('\n', f) ;

  n = 0 ;
  gtv_volume_foreach_vertex(v, (GtsFunc)write_vertex, data) ;
  n = 0;
  if (GTS_POINT_CLASS (v->vertex_class)->binary)
    fputc ('\n', f);
  gtv_volume_foreach_edge(v, (GtsFunc)write_edge, data) ;
  n = 0;
  gtv_volume_foreach_facet(v, (GtsFunc)write_facet, data) ;
  n = 0;
  gtv_volume_foreach_cell(v, (GtsFunc)write_cell, data) ;

  g_hash_table_destroy(vi) ;
  g_hash_table_destroy(ei) ;
  g_hash_table_destroy(fi) ;

  return GTV_SUCCESS ;
}

/** 
 * Read a volume from file, adding its cells to the GtvVolume \a v.
 * 
 * @param v GtvVolume to add cells to;
 * @param f GtsFile to read from.
 * 
 * @return GTV_SUCCESS on success, otherwise the line number where the
 * error occurred.
 */

guint gtv_volume_read(GtvVolume *v, GtsFile *f)

{
  GtsVertex **vertices ;
  GtsEdge **edges ;
  GtvFacet **facets ;
  guint n, nv, ne, nf, nc ;

  g_return_val_if_fail (v != NULL, GTV_NULL_ARGUMENT);
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail (f != NULL, GTV_NULL_ARGUMENT);

  if (f->type != GTS_INT) {
    gts_file_error (f, "expecting an integer (number of vertices)");
    return f->line;
  }
  nv = atoi (f->token->str);

  gts_file_next_token (f);
  if (f->type != GTS_INT) {
    gts_file_error (f, "expecting an integer (number of edges)");
    return f->line;
  }
  ne = atoi (f->token->str);

  gts_file_next_token (f);
  if (f->type != GTS_INT) {
    gts_file_error (f, "expecting an integer (number of facets)");
    return f->line;
  }
  nf = atoi (f->token->str);

  gts_file_next_token (f);
  if (f->type != GTS_INT) {
    gts_file_error (f, "expecting an integer (number of cells)");
    return f->line;
  }
  nc = atoi (f->token->str);

  gts_file_next_token (f);
  if (f->type == GTS_STRING) {
    if (f->type != GTS_STRING) {
      gts_file_error (f, "expecting a string (GtvVolumeClass)");
      return f->line;
    }
    gts_file_next_token (f);
    if (f->type != GTS_STRING) {
      gts_file_error (f, "expecting a string (GtvCellClass)");
      return f->line;
    }
    gts_file_next_token (f);
    if (f->type != GTS_STRING) {
      gts_file_error (f, "expecting a string (GtvFacetClass)");
      return f->line;
    }
    gts_file_next_token (f);
    if (f->type != GTS_STRING) {
      gts_file_error (f, "expecting a string (GtsEdgeClass)");
      return f->line;
    }
    gts_file_next_token (f);
    if (f->type != GTS_STRING) {
      gts_file_error (f, "expecting a string (GtsVertexClass)");
      return f->line;
    }
    if (!strcmp (f->token->str, "GtsVertexBinary"))
      GTS_POINT_CLASS (v->vertex_class)->binary = TRUE;
    else {
      GTS_POINT_CLASS (v->vertex_class)->binary = FALSE;
      gts_file_first_token_after (f, '\n');
    }
  }
  else
    gts_file_first_token_after (f, '\n');

  if ( nf <= 0 ) return GTV_SUCCESS ;

  vertices = g_malloc((nv+1)*sizeof(GtsVertex *));
  edges = g_malloc((ne+1)*sizeof(GtsEdge *));
  facets = g_malloc((nf+1)*sizeof(GtvFacet *));
  
  n = 0 ;
  while (n < nv && f->type != GTS_ERROR) {
    GtsObject * new_vertex =
      gts_object_new (GTS_OBJECT_CLASS (v->vertex_class));

    (* GTS_OBJECT_CLASS (v->vertex_class)->read) (&new_vertex, f);
    if (f->type != GTS_ERROR) {
      if (!GTS_POINT_CLASS (v->vertex_class)->binary)
	gts_file_first_token_after (f, '\n');
      vertices[n++] = GTS_VERTEX (new_vertex);
    }
    else
      gts_object_destroy (new_vertex);
  }
  if (f->type == GTS_ERROR)
    nv = n;
  if (GTS_POINT_CLASS (v->vertex_class)->binary)
    gts_file_first_token_after (f, '\n');

  n = 0;
  while (n < ne && f->type != GTS_ERROR) {
    guint p1, p2;

    if (f->type != GTS_INT)
      gts_file_error (f, "expecting an integer (first vertex index)");
    else {
      p1 = atoi (f->token->str);
      if (p1 == 0 || p1 > nv)
	gts_file_error (f, "vertex index `%d' is out of range `[1,%d]'", 
			p1, nv);
      else {
	gts_file_next_token (f);
	if (f->type != GTS_INT)
	  gts_file_error (f, "expecting an integer (second vertex index)");
	else {
	  p2 = atoi (f->token->str);
	  if (p2 == 0 || p2 > nv)
	    gts_file_error (f, "vertex index `%d' is out of range `[1,%d]'", 
			    p2, nv);
	  else {
	    GtsEdge * new_edge =
	      gts_edge_new (v->edge_class,
			    vertices[p1 - 1], vertices[p2 - 1]);

	    gts_file_next_token (f);
	    if (f->type != '\n')
	      if (GTS_OBJECT_CLASS (v->edge_class)->read)
		(*GTS_OBJECT_CLASS (v->edge_class)->read)
		  ((GtsObject **) (&new_edge), f);
	    gts_file_first_token_after (f, '\n');
	    edges[n++] = new_edge;
	  }
	}
      }
    }
  }
  if (f->type == GTS_ERROR) ne = n;

  n = 0;
  while (n < nf && f->type != GTS_ERROR) {
    guint s1, s2, s3;

    if (f->type != GTS_INT)
      gts_file_error (f, "expecting an integer (first edge index)");
    else {
      s1 = atoi (f->token->str);
      if (s1 == 0 || s1 > ne)
	gts_file_error (f, "edge index `%d' is out of range `[1,%d]'", 
			s1, ne);
      else {
	gts_file_next_token (f);
	if (f->type != GTS_INT)
	  gts_file_error (f, "expecting an integer (second edge index)");
	else {
	  s2 = atoi (f->token->str);
	  if (s2 == 0 || s2 > ne)
	    gts_file_error (f, "edge index `%d' is out of range `[1,%d]'", 
			    s2, ne);
	  else {
	    gts_file_next_token (f);
	    if (f->type != GTS_INT)
	      gts_file_error (f, "expecting an integer (third edge index)");
	    else {
	      s3 = atoi (f->token->str);
	      if (s3 == 0 || s3 > ne)
		gts_file_error (f, "edge index `%d' is out of range `[1,%d]'", 
				s3, ne);
	      else {
		GtvFacet * new_facet = gtv_facet_new (v->facet_class,
						      edges[s1 - 1],
						      edges[s2 - 1],
						      edges[s3 - 1]);

		gts_file_next_token (f);
		if (f->type != '\n')
		  if (GTS_OBJECT_CLASS (v->facet_class)->read)
		    (*GTS_OBJECT_CLASS (v->facet_class)->read)
		      ((GtsObject **) &new_facet, f);
		gts_file_first_token_after (f, '\n');
		facets[n++] = new_facet ;
	      }
	    }
	  }
	}
      }
    }
  }

  n = 0;
  while (n < nc && f->type != GTS_ERROR) {
    guint f1, f2, f3, f4;

    if (f->type != GTS_INT)
      gts_file_error (f, "expecting an integer (first facet index)");
    else {
      f1 = atoi (f->token->str);
      if (f1 == 0 || f1 > nf)
	gts_file_error (f, "facet index `%d' is out of range `[1,%d]'", 
			f1, nf);
      else {
	gts_file_next_token (f);
	if (f->type != GTS_INT)
	  gts_file_error (f, "expecting an integer (second facet index)");
	else {
	  f2 = atoi (f->token->str);
	  if (f2 == 0 || f2 > nf)
	    gts_file_error (f, "facet index `%d' is out of range `[1,%d]'", 
			    f2, nf);
	  else {
	    gts_file_next_token (f);
	    if (f->type != GTS_INT)
	      gts_file_error (f, "expecting an integer (third facet index)");
	    else {
	      f3 = atoi (f->token->str);
	      if (f3 == 0 || f3 > nf)
		gts_file_error (f, 
				"edge index `%d' is out of range `[1,%d]'", 
				f3, nf);
	      else {
		gts_file_next_token (f);
		if (f->type != GTS_INT)
		  gts_file_error (f, 
				  "expecting an integer (third facet index)");
		else {
		  f4 = atoi (f->token->str);
		  if (f4 == 0 || f4 > nf)
		    gts_file_error (f, 
				    "edge index `%d' is out of range `[1,%d]'", 
				    f4, nf);
		
		  else {
		    GtvCell * new_cell = gtv_cell_new (v->cell_class,
						       facets[f1-1],
						       facets[f2-1],
						       facets[f3-1],
						       facets[f4-1]);

		    gts_file_next_token (f);
		    if (f->type != '\n')
		      if (GTS_OBJECT_CLASS (v->cell_class)->read)
			(*GTS_OBJECT_CLASS (v->cell_class)->read)
			  ((GtsObject **) &new_cell, f);
		    gts_file_first_token_after (f, '\n');
		    gtv_volume_add_cell(v, new_cell) ;
		    n++;
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }

  if (f->type == GTS_ERROR) {
    gts_allow_floating_vertices = TRUE;
    while (nv)
      gts_object_destroy (GTS_OBJECT (vertices[nv-- - 1]));
    gts_allow_floating_vertices = FALSE;
  }
  
  g_free(vertices) ; g_free(edges) ; g_free(facets) ;
  
  if (f->type == GTS_ERROR) return f->line;

  return GTV_SUCCESS ;
}

static void cell_foreach_cell(GtvCell *c, gpointer dummy, 
			      gpointer *foreach_data)

{
  (*((GtsFunc) foreach_data[0])) (c, foreach_data[1]) ;

  return ;
}

/** 
 * Execute a function for each cell of a GtvVolume. 
 * 
 * @param v GtvVolume;
 * @param func a GtsFunc to be evaluated for each cell;
 * @param data data to be passed to function.
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_volume_foreach_cell(GtvVolume *v, GtsFunc func, gpointer data)

{
  gpointer foreach_data[2] ;

  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(func != NULL, GTV_NULL_ARGUMENT) ;

  foreach_data[0] = func ; 
  foreach_data[1] = data ; 

  g_hash_table_foreach(v->cells, (GHFunc) cell_foreach_cell, foreach_data) ;

  return GTV_SUCCESS ;
}


static void facet_foreach_cell(GtvTetrahedron *t, gpointer dummy,
			       gpointer *foreach_data)

{
  GtsFunc func = (GtsFunc) foreach_data[0];
  gpointer data = foreach_data[1];
  GHashTable *h = foreach_data[2];

  if ( !g_hash_table_lookup (h, t->f1) ) {
    (*func) (t->f1, data) ;
    g_hash_table_insert(h, t->f1, GINT_TO_POINTER(-1)) ;
  }
  if ( !g_hash_table_lookup (h, t->f2) ) {
    (*func) (t->f2, data) ;
    g_hash_table_insert(h, t->f2, GINT_TO_POINTER(-1)) ;
  }
  if ( !g_hash_table_lookup (h, t->f3) ) {
    (*func) (t->f3, data) ;
    g_hash_table_insert(h, t->f3, GINT_TO_POINTER(-1)) ;
  }
  if ( !g_hash_table_lookup (h, t->f4) ) {
    (*func) (t->f4, data) ;
    g_hash_table_insert(h, t->f4, GINT_TO_POINTER(-1)) ;
  }

  return ;
}

/** 
 * Execute a function for each facet of a GtvVolume. 
 * 
 * @param v GtvVolume;
 * @param func a GtsFunc to be evaluated for each facet;
 * @param data data to be passed to function.
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_volume_foreach_facet(GtvVolume *v, GtsFunc func, gpointer data)

{
  gpointer foreach_data[3] ;

  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(func != NULL, GTV_NULL_ARGUMENT) ;

  foreach_data[0] = func ; 
  foreach_data[1] = data ; 
  foreach_data[2] = g_hash_table_new(NULL, NULL) ;

  g_hash_table_foreach(v->cells, (GHFunc) facet_foreach_cell, foreach_data) ;

  g_hash_table_destroy(foreach_data[2]) ;

  return GTV_SUCCESS ;
}

static void edge_foreach_cell(GtvTetrahedron *t, gpointer dummy,
			      gpointer *foreach_data)

{
  GtsFunc func = (GtsFunc) foreach_data[0];
  gpointer data = foreach_data[1];
  GHashTable *h = foreach_data[2];
  GtsEdge *e ;

  e = GTS_TRIANGLE(t->f1)->e1 ;
  if ( !g_hash_table_lookup (h, e) ) {
    (*func) (e, data) ;
    g_hash_table_insert(h, e, GINT_TO_POINTER(-1)) ;
  }
  e = GTS_TRIANGLE(t->f1)->e2 ;
  if ( !g_hash_table_lookup (h, e) ) {
    (*func) (e, data) ;
    g_hash_table_insert(h, e, GINT_TO_POINTER(-1)) ;
  }
  e = GTS_TRIANGLE(t->f1)->e3 ;
  if ( !g_hash_table_lookup (h, e) ) {
    (*func) (e, data) ;
    g_hash_table_insert(h, e, GINT_TO_POINTER(-1)) ;
  }
  e = gts_triangles_common_edge(GTS_TRIANGLE(t->f2),
				GTS_TRIANGLE(t->f3)) ;
  if ( !g_hash_table_lookup (h, e) ) {
    (*func) (e, data) ;
    g_hash_table_insert(h, e, GINT_TO_POINTER(-1)) ;
  }
  e = gts_triangles_common_edge(GTS_TRIANGLE(t->f3),
				GTS_TRIANGLE(t->f4)) ;
  if ( !g_hash_table_lookup (h, e) ) {
    (*func) (e, data) ;
    g_hash_table_insert(h, e, GINT_TO_POINTER(-1)) ;
  }
  e = gts_triangles_common_edge(GTS_TRIANGLE(t->f4),
				GTS_TRIANGLE(t->f2)) ;
  if ( !g_hash_table_lookup (h, e) ) {
    (*func) (e, data) ;
    g_hash_table_insert(h, e, GINT_TO_POINTER(-1)) ;
  }

  return ;
}

/** 
 * Execute a function for each edge of a GtvVolume. 
 * 
 * @param v GtvVolume;
 * @param func a GtsFunc to be evaluated for each edge;
 * @param data data to be passed to function.
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_volume_foreach_edge(GtvVolume *v, GtsFunc func, gpointer data)

{
  gpointer foreach_data[3] ;

  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(func != NULL, GTV_NULL_ARGUMENT) ;

  foreach_data[0] = func ; 
  foreach_data[1] = data ; 
  foreach_data[2] = g_hash_table_new(NULL, NULL) ;

  g_hash_table_foreach(v->cells, (GHFunc) edge_foreach_cell, foreach_data) ;

  g_hash_table_destroy(foreach_data[2]) ;

  return GTV_SUCCESS ;
}

static void vertex_foreach_cell(GtvTetrahedron *t, gpointer dummy,
				gpointer *foreach_data)

{
  GtsFunc func = (GtsFunc) foreach_data[0];
  gpointer data = foreach_data[1];
  GHashTable *h = foreach_data[2];
  GtsVertex *v1, *v2, *v3, *v4 ;

  gtv_tetrahedron_vertices(t, &v1, &v2, &v3, &v4) ;

  if ( !g_hash_table_lookup (h, v1)) {
    (*func) (v1, data) ;
    g_hash_table_insert(h, v1, GINT_TO_POINTER(-1)) ;
  }

  if ( !g_hash_table_lookup (h, v2)) {
    (*func) (v2, data) ;
    g_hash_table_insert(h, v2, GINT_TO_POINTER(-1)) ;
  }

  if ( !g_hash_table_lookup (h, v3)) {
    (*func) (v3, data) ;
    g_hash_table_insert(h, v3, GINT_TO_POINTER(-1)) ;
  }

  if ( !g_hash_table_lookup (h, v4)) {
    (*func) (v4, data) ;
    g_hash_table_insert(h, v4, GINT_TO_POINTER(-1)) ;
  }

  return ;
}

/** 
 * Execute a function for each vertex of a GtvVolume. 
 * 
 * @param v GtvVolume;
 * @param func a GtsFunc to be evaluated for each vertex;
 * @param data data to be passed to function.
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_volume_foreach_vertex(GtvVolume *v, GtsFunc func, gpointer data)

{
  gpointer foreach_data[3] ;

  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(func != NULL, GTV_NULL_ARGUMENT) ;

  foreach_data[0] = func ; 
  foreach_data[1] = data ; 
  foreach_data[2] = g_hash_table_new(NULL, NULL) ;

  g_hash_table_foreach(v->cells, (GHFunc) vertex_foreach_cell, foreach_data) ;

  g_hash_table_destroy(foreach_data[2]) ;

  return GTV_SUCCESS ;
}

static void stats_foreach_vertex(GtsVertex *v, GtvVolumeStats *s)

{
  GSList * i = v->segments;
  guint nedges = 0;

  while (i) {
    if (GTS_IS_EDGE (i->data) && 
	gtv_edge_has_parent_volume (i->data, s->parent))
      nedges++;
    i = i->next;
  }
  gts_range_add_value (&s->edges_per_vertex, nedges);

  return ;
}

static void stats_foreach_edge(GtsEdge *e, GtvVolumeStats *s)

{
  guint nf = gtv_edge_facet_number(e, s->parent) ;
  if (gts_edge_is_duplicate (e)) s->n_duplicate_edges++;

  gts_range_add_value(&s->facets_per_edge, nf) ;

  return ;
}

static void stats_foreach_facet(GtvFacet *f, GtvVolumeStats *s)

{
  guint nc = gtv_facet_cell_number(f, s->parent) ;

  if (gts_triangle_is_duplicate (GTS_TRIANGLE (f)))
    s->n_duplicate_facets++;
  if ( nc == 1 ) 
    s->n_boundary_facets ++ ;

  gts_range_add_value(&s->cells_per_facet, nc) ;  

  return ;
}

static void stats_foreach_cell(GtvCell *c, GtvVolumeStats *s)

{

  s->n_cells ++ ;

  return ;
}

/** 
 * Find basic statistics for a volume
 * 
 * @param v GtvVolume
 * @param s GtvVolumeStats to fill with data
 * 
 * @return GTV_SUCCESS on success
 */

gint gtv_volume_stats(GtvVolume *v, GtvVolumeStats *s)

{
  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(s != NULL, GTV_NULL_ARGUMENT) ;

  s->parent = v ;
  s->n_cells = 0 ;
  s->n_incompatible_cells = 0 ;
  s->n_duplicate_cells = 0 ;
  s->n_facets = 0 ;
  s->n_incompatible_facets = 0 ;
  s->n_duplicate_facets = 0 ;
  s->n_boundary_facets = 0 ;
  s->n_duplicate_edges = 0 ;
  gts_range_init(&s->edges_per_vertex) ;
  gts_range_init(&s->facets_per_edge) ;
  gts_range_init(&s->cells_per_facet) ;

  gtv_volume_foreach_vertex(v, (GtsFunc) stats_foreach_vertex, s);
  gtv_volume_foreach_edge(v, (GtsFunc) stats_foreach_edge, s);
  gtv_volume_foreach_facet(v, (GtsFunc) stats_foreach_facet, s);
  gtv_volume_foreach_cell(v, (GtsFunc) stats_foreach_cell, s);

  gts_range_update(&s->edges_per_vertex) ;
  gts_range_update(&s->facets_per_edge) ;
  gts_range_update(&s->cells_per_facet) ;

  return GTV_SUCCESS ;
}

static void quality_foreach_edge (GtsSegment * s,
				  GtvVolumeQualityStats * stats) 
{
  GSList * i = GTS_EDGE (s)->triangles;

  gts_range_add_value (&stats->edge_length, 
		   gts_point_distance (GTS_POINT (s->v1), 
				       GTS_POINT (s->v2)));
  while (i) {
    GSList * j = i->next;
    while (j) {
      gts_range_add_value (&stats->edge_angle,
			   fabs (gts_triangles_angle (i->data, j->data)));
      j = j->next;
    }
    i = i->next;
  }
}

static void quality_foreach_facet(GtsTriangle * t,
				  GtvVolumeQualityStats * stats) 
{
  gts_range_add_value (&stats->facet_quality, gts_triangle_quality (t));
  gts_range_add_value (&stats->facet_area, gts_triangle_area (t));
}

static void quality_foreach_cell(GtvTetrahedron *t,
				 GtvVolumeQualityStats * stats) 
{
  gts_range_add_value (&stats->cell_quality, gtv_tetrahedron_quality(t)) ;
  gts_range_add_value (&stats->cell_volume, gtv_tetrahedron_volume(t)) ;
}

/** 
 * Fill a ::GtvVolumeQualityStats with the geometrical quality
 * information for a ::GtvVolume.
 * 
 * @param v a ::GtvVolume;
 * @param s a ::GtvVolumeQualityStats to be filled with data.
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_volume_quality_stats(GtvVolume *v, GtvVolumeQualityStats *s)

{
  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(s != NULL, GTV_NULL_ARGUMENT) ;

  s->parent = v ;
  gts_range_init(&s->cell_quality) ;
  gts_range_init(&s->cell_volume) ;
  gts_range_init(&s->facet_quality) ;
  gts_range_init(&s->facet_area) ;
  gts_range_init(&s->edge_length) ;
  gts_range_init(&s->edge_angle) ;

  gtv_volume_foreach_edge(v, (GtsFunc)quality_foreach_edge, s) ;
  gtv_volume_foreach_facet(v, (GtsFunc)quality_foreach_facet, s) ;
  gtv_volume_foreach_cell(v, (GtsFunc)quality_foreach_cell, s) ;

  gts_range_update(&s->cell_quality) ;
  gts_range_update(&s->cell_volume) ;
  gts_range_update(&s->facet_quality) ;
  gts_range_update(&s->facet_area) ;
  gts_range_update(&s->edge_length) ;
  gts_range_update(&s->edge_angle) ;

  return GTV_SUCCESS ;
}

/** 
 * Print out basic statistics about a GtvVolume
 * 
 * @param v GtvVolume
 * @param f file pointer
 * 
 * @return GTV_SUCCESS on success
 */

gint gtv_volume_print_stats(GtvVolume *v, FILE *f)

{
  GtvVolumeStats s ;
  GtvVolumeQualityStats qs ;

  g_return_val_if_fail (v != NULL, GTV_NULL_ARGUMENT);
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail (f != NULL, GTV_NULL_ARGUMENT);

  gtv_volume_stats(v, &s) ;
  gtv_volume_quality_stats(v, &qs) ;

  fprintf(f, 
	  "# vertices: %u edges: %u facets: %u cells: %u\n"
	  "# Connectivity statistics\n"
	  "#   incompatible cells: %u\n"
	  "#   duplicate cells: %u\n"
	  "#   boundary facets: %u\n"
	  "#   duplicate facets: %u\n"
	  "#   duplicate edges: %u\n",
	  s.edges_per_vertex.n,
	  s.facets_per_edge.n,
	  s.cells_per_facet.n,
	  s.n_cells,
	  s.n_incompatible_cells,
	  s.n_duplicate_cells,
	  s.n_boundary_facets,
	  s.n_duplicate_facets,
	  s.n_duplicate_edges) ;

  fputs("#   edges per vertex: ", f) ;
  gts_range_print(&s.edges_per_vertex, f) ;
  fputs("\n#   facets per edge: ", f) ;
  gts_range_print(&s.facets_per_edge, f) ;
  fputs("\n#   cells per facet: ", f) ;
  gts_range_print(&s.cells_per_facet, f) ;
  fputs("\n# Geometric statistics\n#   cell quality: ", f);
  gts_range_print (&qs.cell_quality, f);
  fputs("\n#   cell volume: ", f) ;
  gts_range_print (&qs.cell_volume, f);
  fputs("\n#   facet quality: ", f) ;
  gts_range_print (&qs.facet_quality, f);
  fputs("\n#   facet_area: ", f) ;
  gts_range_print (&qs.facet_area, f);
  fputs("\n#   edge_length: ", f) ;
  gts_range_print (&qs.edge_length, f);
  fputc('\n', f) ;

  return GTV_SUCCESS ;
}

static void volume_boundary(GtvFacet *f, gpointer *data)

{
  if ( !gtv_facet_is_boundary(f, GTV_VOLUME(data[0]))) return ;

  gts_surface_add_face(GTS_SURFACE(data[1]),
		       gts_face_new(GTS_SURFACE(data[1])->face_class,
				    GTS_TRIANGLE(f)->e1,
				    GTS_TRIANGLE(f)->e2,
				    GTS_TRIANGLE(f)->e3)) ;
  return ;
}

/** 
 * Add the boundary facets of a GtvVolume to a GtsSurface. 
 * 
 * @param v GtvVolume
 * @param s GtsSurface to take boundary facets
 * 
 * @return GTV_SUCCESS on success
 */

gint gtv_volume_boundary(GtvVolume *v, GtsSurface *s)

{
  gpointer data[2] ;

  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(s != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTS_IS_SURFACE(s), GTV_WRONG_TYPE) ;

  data[0] = v ; data[1] = s ;
  gtv_volume_foreach_facet(v, (GtsFunc) volume_boundary, data) ;

  return GTV_SUCCESS ;
}

static void volume_volume(GtvCell *c, gdouble *V)

{
  *V += gtv_tetrahedron_volume(GTV_TETRAHEDRON(c)) ;

  return ;
}

/** 
 * Volume of a GtvVolume, found as the sum of the cell volumes. 
 * 
 * @param v GtvVolume
 * 
 * @return volume of \a v
 */

gdouble gtv_volume_volume(GtvVolume *v)

{
  gdouble V = 0.0 ;

  g_return_val_if_fail(v != NULL, 0.0) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;

  gtv_volume_foreach_cell(v, (GtsFunc) volume_volume, &V) ;

  return V ;
}

static void count_vertices(GtsVertex *v, guint *n) 

{
  (*n) ++ ;
}

guint gtv_volume_vertex_number(GtvVolume *v)

{
  guint n = 0 ;

  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;

  gtv_volume_foreach_vertex(v, (GtsFunc)count_vertices, &n) ;

  return n ;
}

static gint nearest_vertex(GtsVertex *v, gpointer data[])

{
  GtsVertex **n = (GtsVertex **)data[0] ;
  gdouble *R = (gdouble *)data[1] ;
  GtsPoint *p = (GtsPoint *)data[2] ;
  gdouble r ;

  if ( (r = gts_point_distance2(GTS_POINT(v), p)) < (*R) ) {
    *n = v ; *R = r ;
  }

  return GTV_SUCCESS ;
}

/** 
 * Find the vertex of a tetrahedralization which is nearest a ::GtsPoint. 
 * 
 * @param v a ::GtvVolume;
 * @param p a ::GtsPoint.
 * 
 * @return the vertex of \a v which is closest to \a p, NULL in case
 * of an error.
 */

GtsVertex *gtv_volume_nearest_vertex(GtvVolume *v, GtsPoint *p)

{
  GtsVertex *n ;
  gpointer data[3] ;
  gdouble R ;

  g_return_val_if_fail(v != NULL, NULL) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), NULL) ;
  g_return_val_if_fail(p != NULL, NULL) ;
  g_return_val_if_fail(GTS_IS_POINT(p), NULL) ;

  n = NULL ; R = G_MAXDOUBLE ;
  data[0] = &n ; data[1] = &R ; data[2] = p ;

  gtv_volume_foreach_vertex(v, (GtsFunc)nearest_vertex, data) ;

  return n ;
}

static FILE *file_open(gchar *file)

{
  FILE *output ;

  output = fopen(file, "w") ;
  if ( output == NULL ) {
    g_message("%s: cannot open file%s", __FUNCTION__, file) ;
  }
  
  return output ;
}

static gint index_and_write(GtsVertex *v, gpointer data[])

{
  GHashTable *h = (GHashTable *)data[0] ;
  FILE *f = (FILE *)data[1] ;
  gint *n = (gint *)data[2] ;

  g_hash_table_insert(h, v, GINT_TO_POINTER((*n))) ;

  fprintf(f, "%d %lg %lg %lg\n", (*n), 
	  GTS_POINT(v)->x, GTS_POINT(v)->y, GTS_POINT(v)->z) ;

  (*n) ++ ;

  return GTV_SUCCESS ;
}

static gint tetgen_write_cell(GtvCell *t, gpointer data[])

{
  GHashTable *h = (GHashTable *)data[0] ;
  FILE *f = (FILE *)data[1] ;
  gint *n = (gint *)data[2] ;
  GtsVertex *v1, *v2, *v3, *v4 ;

  gtv_tetrahedron_vertices(GTV_TETRAHEDRON(t), &v1, &v2, &v3, &v4) ;

  fprintf(f, "%5d   %5d %5d %5d %5d\n", (*n),
	  GPOINTER_TO_INT(g_hash_table_lookup(h, v1)),
	  GPOINTER_TO_INT(g_hash_table_lookup(h, v2)),
	  GPOINTER_TO_INT(g_hash_table_lookup(h, v3)),
	  GPOINTER_TO_INT(g_hash_table_lookup(h, v4))) ;

  (*n) ++ ;

  return GTV_SUCCESS ;
}

/** 
 * Write a ::GtvVolume to .node and .ele files which can be read by
 * tetgen, to allow checking and comparisons.
 * 
 * @param v a ::GtvVolume; @param stub the stub file name. Files will
 * be written to stub.node and stub.ele.
 * 
 * @return GTV_SUCCESS on success.
 */

gint gtv_volume_write_tetgen(GtvVolume *v, gchar *stub)

{
  FILE *output ;
  GHashTable *h ;
  gpointer foreach_data[4] ;
  GString *filename ;
  GtvVolumeStats s ;
  gint n ;

  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(stub != NULL, GTV_NULL_ARGUMENT) ;

  g_debug("%s:", __FUNCTION__) ;

  h = g_hash_table_new(NULL, NULL) ;

  gtv_volume_stats(v, &s) ;

  filename = g_string_new(stub) ;

#if GLIB_CHECK_VERSION(2,0,0)
  g_string_printf(filename, "%s.node", stub) ;
#else
  g_string_sprintf(filename, "%s.node", stub) ;
#endif /*GLIB_CHECK_VERSION(2,0,0)*/

  if ( (output = file_open(filename->str)) == NULL ) return GTV_FAILURE ;

  foreach_data[0] = h ; foreach_data[1] = output ; foreach_data[2] = &n ; 
  n = 1 ;
  fprintf(output, "%u 3 0 0\n", gtv_volume_vertex_number(v)) ;
  gtv_volume_foreach_vertex(v, (GtsFunc)index_and_write, foreach_data) ;
  fprintf(output, "# generated by %s()\n", __FUNCTION__) ;

  fclose(output) ;

#if GLIB_CHECK_VERSION(2,0,0)
  g_string_printf(filename, "%s.ele", stub) ;
#else
  g_string_sprintf(filename, "%s.ele", stub) ;
#endif /*GLIB_CHECK_VERSION(2,0,0)*/

  if ( (output = file_open(filename->str)) == NULL ) return GTV_FAILURE ;

  foreach_data[1] = output ; n = 1 ;
  fprintf(output, "%u  4  0\n", s.n_cells) ;
  gtv_volume_foreach_cell(v, (GtsFunc)tetgen_write_cell, foreach_data) ;
  fprintf(output, "# generated by %s()\n", __FUNCTION__) ;
  fclose(output) ;

  return GTV_SUCCESS ;
}


static gint msh_element_n_nodes(gint type)

{
  switch (type) {
  default: g_assert_not_reached() ; break ;
  case GTV_GMSH_ELEMENT_LINE: return 2 ; break ;
  case GTV_GMSH_ELEMENT_TRIANGLE: return 3 ; break ;
  case GTV_GMSH_ELEMENT_QUADRANGLE: return 4 ; break ;
  case GTV_GMSH_ELEMENT_TETRAHEDRON: return 4 ; break ;
  case GTV_GMSH_ELEMENT_HEXAHEDRON: return 8 ; break ;
  case GTV_GMSH_ELEMENT_PRISM: return 6 ; break ;
  case GTV_GMSH_ELEMENT_PYRAMID: return 5 ; break ;
  case GTV_GMSH_ELEMENT_LINE_2: return 2+1 ; break ;
  case GTV_GMSH_ELEMENT_TRIANGLE_2: return 3+3 ; break ;
  case GTV_GMSH_ELEMENT_QUADRANGLE_2: return 4+4+1 ; break ;
  case GTV_GMSH_ELEMENT_TETRAHEDRON_2: return 4+6 ; break ;
  case GTV_GMSH_ELEMENT_HEXAHEDRON_2: return 8+12+6+1 ; break ;
  case GTV_GMSH_ELEMENT_PRISM_2: return 6+9+3 ; break ;
  case GTV_GMSH_ELEMENT_PYRAMID_2: return 5+8+1 ; break ;
  case GTV_GMSH_ELEMENT_POINT: return 1 ; break ;
  }
  return GTV_SUCCESS ;
}

static gint msh_read_element2(FILE *f, gint *i, gint *e,
			      gint *tags,
			      gint *nodes, gint *nn)

{
  gint j, nt ;
  
  if ( fscanf(f, "%d", i) != 1) return -1 ;
  if ( fscanf(f, "%d", e) != 1) return -1 ;

  *nn = msh_element_n_nodes(*e) ;

  if ( fscanf(f, "%d", &nt) != 1) return -1 ;

  for ( j = 0 ; j < nt ; j ++ ) 
    if ( fscanf(f, "%d", &(tags[j])) != 1) return -1 ;

  for ( j = 0 ; j < (*nn) ; j ++ )
    if ( fscanf(f, "%d", &nodes[j]) != 1 ) return -1 ;

  return GTV_SUCCESS ;
}

static gint msh_read_element1(FILE *f, gint *i, gint *e,
			      gint *rp, gint *re,
			      gint *nodes, gint *nn)

{
  gint j ;
  
  if ( fscanf(f, "%d", i) != 1) return -1 ;
  if ( fscanf(f, "%d", e) != 1) return -1 ;

  if ( fscanf(f, "%d", rp) != 1 ) return -1 ;
  if ( fscanf(f, "%d", re) != 1 ) return -1 ;
  if ( fscanf(f, "%d", nn) != 1 ) return -1 ;

  for ( j = 0 ; j < (*nn) ; j ++ )
    if ( fscanf(f, "%d", &nodes[j]) != 1 ) return -1 ;

  return GTV_SUCCESS ;
}

static gint msh_add_element(GtvVolume *v, GHashTable *h,
			    gint elem, gint *nodes, gint nn)

{
  GtsVertex *w[32] ;
  GtvCell *c ;

  g_debug("%s: adding element of type %d", __FUNCTION__, elem) ;

  switch (elem) {
  case GTV_GMSH_ELEMENT_POINT: return GTV_SUCCESS ; break ;
  case GTV_GMSH_ELEMENT_LINE: return GTV_SUCCESS ; break ;
  case GTV_GMSH_ELEMENT_LINE_2: return GTV_SUCCESS ; break ;
  case GTV_GMSH_ELEMENT_TRIANGLE: return GTV_SUCCESS ; break ;
  case GTV_GMSH_ELEMENT_TRIANGLE_2: return GTV_SUCCESS ; break ;
  case GTV_GMSH_ELEMENT_TETRAHEDRON:
    w[0] = g_hash_table_lookup(h, GINT_TO_POINTER(nodes[0])) ;
    w[1] = g_hash_table_lookup(h, GINT_TO_POINTER(nodes[1])) ;
    w[2] = g_hash_table_lookup(h, GINT_TO_POINTER(nodes[2])) ;
    w[3] = g_hash_table_lookup(h, GINT_TO_POINTER(nodes[3])) ;
    if ( (c = GTV_CELL(gtv_tetrahedron_from_vertices(w[0], w[1], w[2], w[3])))
	 == NULL )
      /*our vertex order is the opposite of gmsh's so swap w[0] and w[1]*/
      c = gtv_cell_new_from_vertices(gtv_cell_class(),
				     gtv_facet_class(),
				     gts_edge_class(),
				     w[1], w[0], w[2], w[3]) ;
    gtv_volume_add_cell(v, c) ;
    g_debug("%s: added cell with vertices (%p,%p,%p,%p)",
	    __FUNCTION__, w[1], w[0], w[2], w[3]) ;
      return GTV_SUCCESS ; break ;
    break ;
  default: 
    if ( elem <= 15 ) {
      g_debug("%s: unimplemented GMSH element type %d", 
	      __FUNCTION__, elem) ;
      return GTV_SUCCESS ; break ;
    } else {
      g_debug("%s: unrecognized GMSH element type %d", 
	      __FUNCTION__, elem) ;
      return -1 ; break ;
    }
  }

  return GTV_SUCCESS ;
}

static guint msh_read_file1(FILE *f, GtvVolume *v)

{
  gchar line[256] ;
  guint lineno = 2 ;
  gint i, j, np, ne, nv, elem, nn, data[32] ;
  gint rp, re ;
  GHashTable *h ;
  gdouble x, y, z ;

  if ( (nv = fscanf(f, "%d", &np)) != 1 ) {
    g_debug("%s: error reading number of vertices", __FUNCTION__) ;
    return lineno ;
  }
  lineno ++ ;

  h = g_hash_table_new(NULL, NULL) ;
  for ( i = 0 ; i < np ; i ++ ) {
    nv = fscanf(f, "%d %lg %lg %lg", &j, &x, &y, &z) ;
    if ( nv != 4 ) {
      g_debug("%s: error reading vertex at line %u", __FUNCTION__, lineno) ;
      return lineno ;    
    }
    if ( j <= 0 ) {
      g_debug("%s: vertex index %d out of range at line %u", 
	      __FUNCTION__, j, lineno) ;
      return lineno ;    
    }
    
    g_hash_table_insert(h, GINT_TO_POINTER(j), 
			gts_vertex_new(GTV_VOLUME(v)->vertex_class, 
				       x, y, z)) ;
    lineno ++ ;
  }

  g_debug("%s: %d vertices read", __FUNCTION__, np) ;

  nv = fscanf(f, "%s", line) ; 
  g_debug("%s: %s", __FUNCTION__, line) ;
  if ( strcmp(line, "$ENDNOD") || nv == 0) {
    g_debug("%s: no $ENDNOD marker found", __FUNCTION__) ;
    return lineno ;        
  }
  lineno ++ ;

  nv = fscanf(f, "%s", line) ;
  g_debug("%s: %s", __FUNCTION__, line) ;
  if ( strcmp(line, "$ELM") ) {
    g_debug("%s: no $ELM marker found", __FUNCTION__) ;
    return lineno ;        
  }
  lineno ++ ;

  nv = fscanf(f, "%d", &ne) ;
  if ( nv == 0 ) {
    g_debug("%s: error reading number of elements", __FUNCTION__) ;
    return lineno ;
  }
  lineno ++ ;

  for ( i = 0 ; i < ne ; i ++ ) {
    if ( msh_read_element1(f, &j, &elem, &rp, &re, data, &nn) != 0 ) {
      g_debug("%s: error reading element at line %u", __FUNCTION__, lineno) ;
      return lineno ;
    }
    if ( msh_add_element(v, h, elem, data, nn) != 0 ) {
      g_debug("%s: error adding element at line %u", __FUNCTION__, lineno) ;
      return lineno ;
    }
    lineno ++ ;
  }

  g_hash_table_destroy(h) ;  

  return GTV_SUCCESS ;
}


static guint msh_read_file2(FILE *f, GtvVolume *v)

{
  gchar line[256] ;
  guint lineno = 2 ;
  gint i, j, np, ne, nv, elem, ntags, data[32], tags[32] ;
  gint ft, ds ;
  gdouble version ;
  GHashTable *h ;
  gdouble x, y, z ;

  if ( (nv = fscanf(f, "%lg %d %d", &version, &ft, &ds) ) != 3 ) {
    g_debug("%s: error reading mesh format information", 
	    __FUNCTION__) ;
    return lineno ;
  }
  if ( version != 2.0 && version != 2.1 && version != 2.2 ) {
    g_debug("%s: file version (%lg) should be 2.0 or 2.1",  
	    __FUNCTION__, version) ;
    return lineno ;
  }
  lineno ++ ;

  g_debug("%s: file format %lg, file type %d, data size %d",  
	  __FUNCTION__, version, ft, ds) ;

  nv = fscanf(f, "%s", line) ;
  g_debug("%s: %s", __FUNCTION__, line) ;
  if ( strcmp(line, "$EndMeshFormat") || nv == 0) {
    g_debug("%s: no $EndMeshFormat marker found", __FUNCTION__) ;
    return lineno ;        
  }
  lineno ++ ;

  nv = fscanf(f, "%s", line) ;
  g_debug("%s: %s", __FUNCTION__, line) ;
  if ( strcmp(line, "$Nodes") || nv == 0) {
    g_debug("%s: no $Nodes marker found", __FUNCTION__) ;
    return lineno ;        
  }
  lineno ++ ;

  nv = fscanf(f, "%d", &np) ;
  if ( nv == 0 ) {
    g_debug("%s: error reading number of vertices", __FUNCTION__) ;
    return lineno ;
  }

  lineno ++ ;

  h = g_hash_table_new(NULL, NULL) ;
  for ( i = 0 ; i < np ; i ++ ) {
    nv = fscanf(f, "%d %lg %lg %lg", &j, &x, &y, &z) ;
    if ( nv != 4 ) {
      g_debug("%s: error reading vertex at line %u", __FUNCTION__, lineno) ;
      return lineno ;    
    }
    if ( j <= 0 ) {
      g_debug("%s: vertex index %d out of range at line %u", 
	      __FUNCTION__, j, lineno) ;
      return lineno ;    
    }
    
    g_hash_table_insert(h, GINT_TO_POINTER(j), 
			gts_vertex_new(GTV_VOLUME(v)->vertex_class, 
				       x, y, z)) ;
    lineno ++ ;
  }

  g_debug("%s: %d vertices read", __FUNCTION__, np) ;

  nv = fscanf(f, "%s", line) ; 
  g_debug("%s: %s", __FUNCTION__, line) ;
  if ( strcmp(line, "$EndNodes") || nv == 0) {
    g_debug("%s: no $EndNodes marker found", __FUNCTION__) ;
    return lineno ;        
  }
  lineno ++ ;

  nv = fscanf(f, "%s", line) ;
  g_debug("%s: %s", __FUNCTION__, line) ;
  if ( strcmp(line, "$Elements") ) {
    g_debug("%s: no $Elements marker found", __FUNCTION__) ;
    return lineno ;        
  }
  lineno ++ ;

  nv = fscanf(f, "%d", &ne) ;
  if ( nv == 0 ) {
    g_debug("%s: error reading number of elements", __FUNCTION__) ;
    return lineno ;
  } else 
    g_debug("%s: %d elements expected", __FUNCTION__, ne) ;
    
  lineno ++ ;

  for ( i = 0 ; i < ne ; i ++ ) {
    if ( msh_read_element2(f, &j, &elem, tags, data, &ntags) != 0 ) {
      g_debug("%s: error reading element at line %u", __FUNCTION__, lineno) ;
      return lineno ;
    }
    g_debug("%s: line: %d; element %d; type %d", 
	    __FUNCTION__, lineno, j, elem) ;
    if ( msh_add_element(v, h, elem, data, ntags) != 0 ) {
      g_debug("%s: error adding element at line %u", __FUNCTION__, lineno) ;
      return lineno ;
    }
    lineno ++ ;
  }

  g_hash_table_destroy(h) ;  

  return GTV_SUCCESS ;
}

/** 
 * Read a GMSH mesh file (.msh extension) from a file, adding its
 * elements to a ::GtvVolume.
 * 
 * @param f input file;
 * @param v a ::GtvVolume.
 * 
 * @return ::GTV_SUCCESS on success.
 */

guint gtv_volume_read_gmsh(FILE *f, GtvVolume *v)

{
  gchar line[256] ;
  gint gmsh_format, nf  ;
  
  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(f != NULL, GTV_NULL_ARGUMENT) ;

  gmsh_format = 0 ; nf = 1 ;

  while ( gmsh_format == 0 && nf != 0 ) {
    if ( (nf = fscanf(f, "%s", line)) != 0 ) {
      if ( strncmp(line, "$NOD", 4) == 0 ) gmsh_format = 1 ;
      if ( strncmp(line, "$MeshFormat", 11) == 0 ) gmsh_format = 2 ;
    }
  }

  if ( gmsh_format == 0 ) {
    g_warning("%s: unrecognized msh file format", __FUNCTION__) ;
    return GTV_UNKNOWN_FORMAT ;
  }

  g_debug("%s: GMSH file format: %d", __FUNCTION__, gmsh_format) ;

  if ( gmsh_format == 1 ) return msh_read_file1(f, v) ;

  if ( gmsh_format == 2 ) return msh_read_file2(f, v) ;

  g_assert_not_reached() ;

  return GTV_SUCCESS ;
}

static void gmsh_write_vertex(GtsVertex *v, gpointer *data)

{
  FILE *f = (FILE *)data[0] ;
  GHashTable *h = (GHashTable *)data[1] ;
  gint *n = (gint *)data[2] ;

  fprintf(f, "%d %g %g %g\n", (*n),
	  GTS_POINT(v)->x, GTS_POINT(v)->y, GTS_POINT(v)->z) ;

  g_hash_table_insert(h, v, GINT_TO_POINTER(*n)) ;

  (*n) ++ ;

  return ;
}

static void gmsh_write_cell1(GtvCell *c, gpointer *data)

{
  FILE *f = (FILE *)data[0] ;
  GHashTable *h = (GHashTable *)data[1] ;
  gint *n = (gint *)data[2] ;
  GtsVertex *v1, *v2, *v3, *v4 ;

  gtv_tetrahedron_vertices(GTV_TETRAHEDRON(c), &v1, &v2, &v3, &v4) ;

  fprintf(f, "%d 4 0 0 4", *n) ;
  fprintf(f, " %d %d %d %d\n",
	  GPOINTER_TO_INT(g_hash_table_lookup(h, v1)),
	  GPOINTER_TO_INT(g_hash_table_lookup(h, v2)),
	  GPOINTER_TO_INT(g_hash_table_lookup(h, v3)),
	  GPOINTER_TO_INT(g_hash_table_lookup(h, v4))) ;
  
  (*n) ++ ;

  return ;
}

static void gmsh_write_cell(GtvCell *c, gpointer *data)

{
  FILE *f = (FILE *)data[0] ;
  GHashTable *h = (GHashTable *)data[1] ;
  gint *n = (gint *)data[2] ;
  GtsVertex *v1, *v2, *v3, *v4 ;

  gtv_tetrahedron_vertices(GTV_TETRAHEDRON(c), &v1, &v2, &v3, &v4) ;

  fprintf(f, "%d 4 0", *n) ;
  fprintf(f, " %d %d %d %d\n",
	  GPOINTER_TO_INT(g_hash_table_lookup(h, v1)),
	  GPOINTER_TO_INT(g_hash_table_lookup(h, v2)),
	  GPOINTER_TO_INT(g_hash_table_lookup(h, v3)),
	  GPOINTER_TO_INT(g_hash_table_lookup(h, v4))) ;
  
  (*n) ++ ;

  return ;
}

/** 
 * Write a ::GtvVolume to file as a GMSH .msh (format 1.0, deprecated)
 * file.
 * 
 * @param v a ::GtvVolume;
 * @param f file to which \a v is to be written.
 * 
 * @return ::GTV_SUCCESS on success.
 */

gint gtv_volume_write_gmsh1(GtvVolume *v, FILE *f)

{
  gpointer data[4] ;
  GtvVolumeStats s ;
  gint n ;

  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(f != NULL, GTV_NULL_ARGUMENT) ;

  gtv_volume_stats(v, &s) ;

  data[0] = f ;
  data[1] = g_hash_table_new(NULL, NULL) ;
  data[2] = &n ;

  fprintf(f, "$NOD\n") ;
  fprintf(f, "%d\n", s.edges_per_vertex.n) ;
  n = 0 ;
  gtv_volume_foreach_vertex(v, (GtsFunc)gmsh_write_vertex, data) ;
  fprintf(f, "$ENDNOD\n") ;
  fprintf(f, "$ELM\n") ;
  fprintf(f, "%d\n", s.n_cells) ;
  n = 0 ;
  gtv_volume_foreach_cell(v, (GtsFunc)gmsh_write_cell1, data) ;
  fprintf(f, "$ENDELM\n") ;

  g_hash_table_destroy(data[1]) ;
  
  return GTV_SUCCESS ;
}

/** 
 * Write a ::GtvVolume to file as a GMSH .msh (format 2.0, favoured)
 * file.
 * 
 * @param v a ::GtvVolume;
 * @param f file to which \a v is to be written.
 * 
 * @return ::GTV_SUCCESS on success.
 */

gint gtv_volume_write_gmsh(GtvVolume *v, FILE *f)

{
  gpointer data[4] ;
  GtvVolumeStats s ;
  gint n ;

  g_return_val_if_fail(v != NULL, GTV_NULL_ARGUMENT) ;
  g_return_val_if_fail(GTV_IS_VOLUME(v), GTV_WRONG_TYPE) ;
  g_return_val_if_fail(f != NULL, GTV_NULL_ARGUMENT) ;

  gtv_volume_stats(v, &s) ;

  data[0] = f ;
  data[1] = g_hash_table_new(NULL, NULL) ;
  data[2] = &n ;

  fprintf(f, "$MeshFormat\n2.0 0 %lu\n$EndMeshFormat\n",
	  sizeof(gdouble)) ;

  n = 0 ;
  fprintf(f, "$Nodes\n%d\n", s.edges_per_vertex.n) ;
  gtv_volume_foreach_vertex(v, (GtsFunc)gmsh_write_vertex, data) ;
  fprintf(f, "$EndNodes\n") ;

  n = 0 ;
  fprintf(f, "$Elements\n") ;
  fprintf(f, "%d\n", s.n_cells) ;
  gtv_volume_foreach_cell(v, (GtsFunc)gmsh_write_cell, data) ;
  fprintf(f, "$EndElements\n") ;

  /*   fprintf(f, "$PhysicalNames\n0\n$EndPhysicalNames\n") ; */
  /*   fprintf(f, "$NodeData\n0\n$EndNodeData\n") ; */
  /*   fprintf(f, "$ElementData\n0\n$EndElementData\n") ; */
  /*   fprintf(f, "$ElementNodeData\n0\n$EndElementNodeData\n") ; */

  g_hash_table_destroy(data[1]) ;
  
  return GTV_SUCCESS ;
}

static void bbox_foreach_vertex (GtsPoint * p, GtsBBox * bb)
{
  if (p->x < bb->x1) bb->x1 = p->x;
  if (p->y < bb->y1) bb->y1 = p->y;
  if (p->z < bb->z1) bb->z1 = p->z;
  if (p->x > bb->x2) bb->x2 = p->x;
  if (p->y > bb->y2) bb->y2 = p->y;
  if (p->z > bb->z2) bb->z2 = p->z;

  return ;
}

/** 
 * Generate a bounding box containing a ::GtvVolume.
 * 
 * @param klass a ::GtsBBoxClass;
 * @param v a ::GtvVolume.
 * 
 * @return a ::GtsBBox which contains \a v. 
 */

GtsBBox *gtv_bbox_volume(GtsBBoxClass *klass, GtvVolume *v)

{
  GtsBBox *bbox ;

  g_return_val_if_fail(klass != NULL, NULL);
  g_return_val_if_fail(v != NULL, NULL);

  bbox = gts_bbox_new (klass, v, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  bbox->x1 = bbox->y1 = bbox->z1 = G_MAXDOUBLE;
  bbox->x2 = bbox->y2 = bbox->z2 = -G_MAXDOUBLE;

  gtv_volume_foreach_vertex(v, (GtsFunc)bbox_foreach_vertex, bbox) ;

  return bbox ;
}

/**
 * @}
 * 
 */
