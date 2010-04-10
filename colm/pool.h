/*
 *  Copyright 2010 Adrian Thurston <thurston@complang.org>
 */

/*  This file is part of Colm.
 *
 *  Colm is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  Colm is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with Colm; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#ifndef _POOL_H
#define _POOL_H

#include <iostream>

using std::cerr;
using std::endl;
using std::ostream;

/* Allocation, number of items. */
#define FRESH_BLOCK 8128                    

struct PoolBlock
{
	void *data;
	PoolBlock *next;
};

struct PoolItem
{
	PoolItem *next;
};

struct PoolAlloc
{
	PoolBlock *head;
	long nextel;
	PoolItem *pool;
	int sizeofT;
};

void initPoolAlloc( PoolAlloc *poolAlloc, int sizeofT );

struct Program;
typedef struct _Kid Kid;
typedef struct _Tree Tree;
typedef struct _ParseTree ParseTree;
typedef struct _ListEl ListEl;
typedef struct _MapEl MapEl;
typedef struct _Head Head;
typedef struct _Location Location;

Kid *kidAllocate( Program *prg );
void kidFree( Program *prg, Kid *el );
void kidClear( Program *prg );
long kidNumLost( Program *prg );

Tree *treeAllocate( Program *prg );
void treeFree( Program *prg, Tree *el );
void treeClear( Program *prg );
long treeNumLost( Program *prg );

ParseTree *parseTreeAllocate( Program *prg );
void parseTreeFree( Program *prg, ParseTree *el );
void parseTreeClear( Program *prg );
long parseTreeNumLost( Program *prg );

ListEl *listElAllocate( Program *prg );
void listElFree( Program *prg, ListEl *el );
void listElClear( Program *prg );
long listElNumLost( Program *prg );

MapEl *mapElAllocate( Program *prg );
void mapElFree( Program *prg, MapEl *el );
void mapElClear( Program *prg );
long mapElNumLost( Program *prg );

Head *headAllocate( Program *prg );
void headFree( Program *prg, Head *el );
void headClear( Program *prg );
long headNumLost( Program *prg );

Location *locationAllocate( Program *prg );
void locationFree( Program *prg, Location *el );
void locationClear( Program *prg );
long locationNumLost( Program *prg );

#endif
