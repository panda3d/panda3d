/* libpnm4.c - pnm utility library part 4
**
** Copyright (C) 1988 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pnm.h"
#include "rast.h"

/*
** Semi-work-alike versions of some Sun pixrect routines.  Just enough
** for rasterfile reading and writing to work.
*/

struct pixrect*
mem_create( w, h, depth )
    int w, h, depth;
    {
    struct pixrect* p;
    struct mpr_data* m;

    p = (struct pixrect*) malloc( sizeof(struct pixrect) );
    if ( p == NULL )
	return NULL;
    p->pr_ops = NULL;
    p->pr_size.x = w;
    p->pr_size.y = h;
    p->pr_depth = depth;
    m = p->pr_data = (struct mpr_data*) malloc( sizeof(struct mpr_data) );
    if ( m == NULL )
	{
	free( p );
	return NULL;
	}
    /* According to the documentation, linebytes is supposed to be rounded
    ** up to a longword (except on 386 boxes).  However, this turns out
    ** not to be the case.  In reality, all of Sun's code rounds up to
    ** a short, not a long.
    */
    m->md_linebytes = ( w * depth + 15 ) / 16 * 2;
    m->md_offset.x = 0;
    m->md_offset.y = 0;
    m->md_flags = 0;
    m->md_image = (unsigned char*) malloc( m->md_linebytes * h );
    if ( m->md_image == NULL )
	{
	free( m );
	free( p );
	return NULL;
	}

    return p;
    }

void
mem_free( p )
    struct pixrect* p;
    {
    free( p->pr_data->md_image );
    free( p->pr_data );
    free( p );
    }

int
pr_dump( p, out, colormap, type, copy_flag )
    struct pixrect* p;
    FILE* out;
    colormap_t* colormap;
    int type, copy_flag;
    {
    struct rasterfile h;
    int size, besize, count;
    unsigned char* beimage;
    unsigned char* bp;
    unsigned char c, pc;
    int i, j;

    h.ras_magic = RAS_MAGIC;
    h.ras_width = p->pr_size.x;
    h.ras_height = p->pr_size.y;
    h.ras_depth = p->pr_depth;

    h.ras_type = type;
    switch ( type )
	{
	case RT_OLD:
	pm_error( "old rasterfile type is not supported" );

	case RT_FORMAT_TIFF:
	pm_error( "tiff rasterfile type is not supported" );

	case RT_FORMAT_IFF:
	pm_error( "iff rasterfile type is not supported" );

	case RT_EXPERIMENTAL:
	pm_error( "experimental rasterfile type is not supported" );

	case RT_STANDARD:
	case RT_FORMAT_RGB:
	/* Ignore hP->ras_length. */
	h.ras_length = p->pr_size.y * p->pr_data->md_linebytes;
	break;

	case RT_BYTE_ENCODED:
	size = p->pr_size.y * p->pr_data->md_linebytes;
	bp = p->pr_data->md_image;
	beimage = (unsigned char*) malloc( size * 3 / 2 );	/* worst case */
	if ( beimage == NULL )
	    return PIX_ERR;
	besize = 0;
	count = 0;
	for ( i = 0; i < size; ++i )
	    {
	    c = *bp++;
	    if ( count > 0 )
		{
		if ( pc != c )
		    {
		    if ( count == 1 && pc == 128 )
			{
			beimage[besize++] = 128;
			beimage[besize++] = 0;
			count = 0;
			}
		    else if ( count > 2 || pc == 128 )
			{
			beimage[besize++] = 128;
			beimage[besize++] = count - 1;
			beimage[besize++] = pc;
			count = 0;
			}
		    else
			{
			for ( j = 0; j < count; ++j )
			    beimage[besize++] = pc;
			count = 0;
			}
		    }
		}
	    pc = c;
	    ++count;
	    if ( count == 256 )
		{
		beimage[besize++] = 128;
		beimage[besize++] = count - 1;
		beimage[besize++] = c;
		count = 0;
		}
	    }
	if ( count > 0 )
	    {
	    if ( count == 1 && c == 128 )
		{
		beimage[besize++] = 128;
		beimage[besize++] = 0;
		}
	    if ( count > 2 || c == 128 )
		{
		beimage[besize++] = 128;
		beimage[besize++] = count - 1;
		beimage[besize++] = c;
		}
	    else
		{
		for ( j = 0; j < count; ++j )
		    beimage[besize++] = c;
		}
	    }
	h.ras_length = besize;
	break;

	default:
	pm_error( "unknown rasterfile type" );
	}

    if ( colormap == NULL )
	{
	h.ras_maptype = RMT_NONE;
	h.ras_maplength = 0;
	}
    else
	{
	h.ras_maptype = colormap->type;
	switch ( colormap->type )
	    {
	    case RMT_EQUAL_RGB:
	    h.ras_maplength = colormap->length * 3;
	    break;

	    case RMT_RAW:
	    h.ras_maplength = colormap->length;
	    break;

	    default:
	    pm_error( "unknown colormap type" );
	    }
	}

    if ( pm_writebiglong( out, h.ras_magic ) == -1 )
	return PIX_ERR;
    if ( pm_writebiglong( out, h.ras_width ) == -1 )
	return PIX_ERR;
    if ( pm_writebiglong( out, h.ras_height ) == -1 )
	return PIX_ERR;
    if ( pm_writebiglong( out, h.ras_depth ) == -1 )
	return PIX_ERR;
    if ( pm_writebiglong( out, h.ras_length ) == -1 )
	return PIX_ERR;
    if ( pm_writebiglong( out, h.ras_type ) == -1 )
	return PIX_ERR;
    if ( pm_writebiglong( out, h.ras_maptype ) == -1 )
	return PIX_ERR;
    if ( pm_writebiglong( out, h.ras_maplength ) == -1 )
	return PIX_ERR;

    if ( colormap != NULL )
	{
	switch ( colormap->type )
	    {
	    case RMT_EQUAL_RGB:
	    if ( fwrite( colormap->map[0], 1, colormap->length, out ) !=
		 colormap->length )
		return PIX_ERR;
	    if ( fwrite( colormap->map[1], 1, colormap->length, out ) !=
		 colormap->length )
		return PIX_ERR;
	    if ( fwrite( colormap->map[2], 1, colormap->length, out ) !=
		 colormap->length )
		return PIX_ERR;
	    break;

	    case RMT_RAW:
	    if ( fwrite( colormap->map[0], 1, colormap->length, out ) !=
		 colormap->length )
		return PIX_ERR;
	    break;
	    }
	}

    switch ( type )
	{
	case RT_STANDARD:
	case RT_FORMAT_RGB:
	if ( fwrite( p->pr_data->md_image, 1, h.ras_length, out ) !=
	     h.ras_length )
	    return PIX_ERR;
	break;

	case RT_BYTE_ENCODED:
	if ( fwrite( beimage, 1, besize, out ) != besize )
	    {
	    free( beimage );
	    return PIX_ERR;
	    }
	free( beimage );
	break;
	}

    return 0;
    }

int
pr_load_header( in, hP )
    FILE* in;
    struct rasterfile* hP;
    {
    if ( pm_readbiglong( in, &(hP->ras_magic) ) == -1 )
	return PIX_ERR;
    if ( hP->ras_magic != RAS_MAGIC )
	return PIX_ERR;
    if ( pm_readbiglong( in, &(hP->ras_width) ) == -1 )
	return PIX_ERR;
    if ( pm_readbiglong( in, &(hP->ras_height) ) == -1 )
	return PIX_ERR;
    if ( pm_readbiglong( in, &(hP->ras_depth) ) == -1 )
	return PIX_ERR;
    if ( pm_readbiglong( in, &(hP->ras_length) ) == -1 )
	return PIX_ERR;
    if ( pm_readbiglong( in, &(hP->ras_type) ) == -1 )
	return PIX_ERR;
    if ( pm_readbiglong( in, &(hP->ras_maptype) ) == -1 )
	return PIX_ERR;
    if ( pm_readbiglong( in, &(hP->ras_maplength) ) == -1 )
	return PIX_ERR;
    return 0;
    }

int
pr_load_colormap( in, hP, colormap )
    FILE* in;
    struct rasterfile* hP;
    colormap_t* colormap;
    {
    if ( colormap == NULL || hP->ras_maptype == RMT_NONE )
	{
	int i;

	for ( i = 0; i < hP->ras_maplength; ++i )
	    if ( getc( in ) == EOF )
		return PIX_ERR;
	}
    else
	{
	colormap->type = hP->ras_maptype;
	switch ( hP->ras_maptype )
	    {
	    case RMT_EQUAL_RGB:
	    colormap->length = hP->ras_maplength / 3;
	    colormap->map[0] = (unsigned char*) malloc( colormap->length );
	    if ( colormap->map[0] == NULL )
		return PIX_ERR;
	    colormap->map[1] = (unsigned char*) malloc( colormap->length );
	    if ( colormap->map[1] == NULL )
		{
		free( colormap->map[0] );
		return PIX_ERR;
		}
	    colormap->map[2] = (unsigned char*) malloc( colormap->length );
	    if ( colormap->map[2] == NULL )
		{
		free( colormap->map[0] );
		free( colormap->map[1] );
		return PIX_ERR;
		}
	    if ( fread( colormap->map[0], 1, colormap->length, in ) != colormap->length ||
	         fread( colormap->map[1], 1, colormap->length, in ) != colormap->length ||
	         fread( colormap->map[2], 1, colormap->length, in ) != colormap->length )
		{
		free( colormap->map[0] );
		free( colormap->map[1] );
		free( colormap->map[2] );
		return PIX_ERR;
		}
	    break;

	    case RMT_RAW:
	    colormap->length = hP->ras_maplength;
	    colormap->map[0] = (unsigned char*) malloc( colormap->length );
	    if ( colormap->map[0] == NULL )
		return PIX_ERR;
	    colormap->map[2] = colormap->map[1] = colormap->map[0];
	    if ( fread( colormap->map[0], 1, hP->ras_maplength, in ) != hP->ras_maplength )
		{
		free( colormap->map[0] );
		return PIX_ERR;
		}
	    break;

	    default:
	    pm_error( "unknown colormap type" );
	    }
	}
    return 0;
    }

struct pixrect*
pr_load_image( in, hP, colormap )
    FILE* in;
    struct rasterfile* hP;
    colormap_t* colormap;
    {
    struct pixrect* p;
    unsigned char* beimage;
    register unsigned char* bep;
    register unsigned char* bp;
    register unsigned char c;
    int i;
    register int j, count;

    p = mem_create( hP->ras_width, hP->ras_height, hP->ras_depth );
    if ( p == NULL )
	return NULL;

    switch ( hP->ras_type )
	{
	case RT_OLD:
	pm_error( "old rasterfile type is not supported" );

	case RT_FORMAT_TIFF:
	pm_error( "tiff rasterfile type is not supported" );

	case RT_FORMAT_IFF:
	pm_error( "iff rasterfile type is not supported" );

	case RT_EXPERIMENTAL:
	pm_error( "experimental rasterfile type is not supported" );

	case RT_STANDARD:
	case RT_FORMAT_RGB:
	/* Ignore hP->ras_length. */
	i = p->pr_size.y * p->pr_data->md_linebytes;
	if ( fread( p->pr_data->md_image, 1, i, in ) != i )
	    {
	    mem_free( p );
	    return NULL;
	    }
	break;

	case RT_BYTE_ENCODED:
	beimage = (unsigned char*) malloc( hP->ras_length );
	if ( beimage == NULL )
	    {
	    mem_free( p );
	    return NULL;
	    }
	if ( fread( beimage, 1, hP->ras_length, in ) != hP->ras_length )
	    {
	    mem_free( p );
	    free( beimage );
	    return NULL;
	    }
	bep = beimage;
	bp = p->pr_data->md_image;
	for ( i = 0; i < hP->ras_length; )
	    {
	    c = *bep++;
	    if ( c == 128 )
		{
		count = ( *bep++ ) + 1;
		if ( count == 1 )
		    {
		    *bp++ = 128;
		    i += 2;
		    }
		else
		    {
		    c = *bep++;
		    for ( j = 0; j < count; ++j )
			*bp++ = c;
		    i += 3;
		    }
		}
	    else
		{
		*bp++ = c;
		++i;
		}
	    }
	free( beimage );
	break;

	default:
	pm_error( "unknown rasterfile type" );
	}

    return p;
    }
