
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

#ifndef _NANOFS_H
#define _NANOFS_H


#include "block.h"


/*
 *  es: (1) Estructura de datos 
 *  en: (1) Data types
 */

#define NUM_INODES         10
#define NUM_DATA_BLOCKS    20

#define T_FILE       1
#define T_DIRECTORY  2


// Superblock
typedef struct {
    uint32_t numMagic;	          /* Número mágico del superbloque (por ejemplo 0x12345) */
                                  /* Superblock magic number: 0x12345 */
    uint32_t numInodes; 	  /* Número de inodes en el dispositivo */
                                  /* Number of inodes in the device */
    uint32_t inodesPerBlock;      /* Número de inodos por bloque */
                                  /* Number of inodos per blocks */
    uint32_t numInodesBlocks;     /* Número de bloques de inodos en el disp. */
                                  /* Number of inodos blocks in the device */
    uint32_t numDataBlocks;       /* Número de bloques de datos en el disp. */
                                  /* Number of data blocks in the device */
    uint32_t firstMapsBlock;      /* Identificador del bloque donde se guarda los maps */
                                  /* Block id. where maps are stored */
    uint32_t firstInodeBlock;	  /* Identificador del bloque donde se empiezan a guardar los inodos */
                                  /* Block id. where first inodes are stored */
    uint32_t firstDataBlock;      /* 1º bloque de disco para datos tras metadatos */
                                  /* Block id. of the first data block */
    uint32_t sizeDevice;	  /* Tamaño total del disp. (en bytes) */
                                  /* Total size of the device in bytes */
} TypeSuperblock ;


// Inodes
typedef struct {
    uint32_t type;	               /* T_FILE o T_DIRECTORY */
	                               /* T_FILE or T_DIRECTORY */
    char name[50+1];	               /* Nombre del fichero/directorio asociado (termina en cero) */
	                               /* Name of the associated file/directory (end with '\0')*/
    uint16_t inodesInDir[NUM_INODES];  /* si (type == T_DIR) -> lista de los inodes del directorio */
	                               /* if (type == T_DIR) -> list of the inodes in directory */
    uint32_t size;	               /* Tamaño actual del fichero en bytes */
	                               /* Size in bytes */
     int16_t directBlock[1];           /* Número del bloque directo */
	                               /* Number of the direct block */
     int16_t indirectBlock;	       /* Número del bloque indirecto */
	                               /* Number of the indirect block */
} TypeInodeDisk;

typedef TypeInodeDisk TypeInodesDisk[NUM_INODES] ;


// inode map
typedef char TypeInodeMap[NUM_INODES] ;  /* 100…0 (usado: i_map[x]=1 | libre: i_map[x]=0) */
                                         /* 100…0 (used:  i_map[x]=1 | free:  i_map[x]=0) */


// data block map
typedef char TypeBlockMap[NUM_DATA_BLOCKS] ;  /* 000…0 (usado: b_map[x]=1 | libre: b_map[x]=0) */
                                              /* 000…0 (used:  b_map[x]=1 | free:  b_map[x]=0) */


/*
 *  es: (2) Interfaz
 *  en: (2) Interface
 */

int nanofs_mkfs   ( int dev_size ) ;

int nanofs_mount  ( void ) ;
int nanofs_umount ( void ) ;

int nanofs_open   ( char *name ) ;
int nanofs_close  ( int fd ) ;

int nanofs_creat  ( char *name ) ;
int nanofs_unlink ( char * name ) ;

int nanofs_read   ( int fd, char *buffer, int size ) ;
int nanofs_write  ( int fd, char *buffer, int size ) ;
int nanofs_lseek  ( int fd, int offset, int whence ) ;


#endif

