
/*
 *  Copyright 2016-2020 Alejandro Calderon Mateos (ARCOS.INF.UC3M.ES)
 *
 *  This file is part of nanofs (nano-filesystem).
 *
 *  nanofs is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  nanofs is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with nanofs.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _BLOCK_H
#define _BLOCK_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define DISK        "disk.dat"
#define BLOCK_SIZE  1024

int bread  ( char *devname, int bid, void *buffer ) ;
int bwrite ( char *devname, int bid, void *buffer ) ;


#endif

