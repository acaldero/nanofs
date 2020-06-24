
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


#include "nanofs.h"


/*
 *  es: Estructura de datos en memoria
 *  en: Memory data structures
 */

// es: Metadatos leídos desde disco
// en: Metadata from disk
TypeSuperblock  sblock ;
TypeInodesDisk  inodes ;
TypeInodeMap    i_map ;
TypeBlockMap    b_map ;

// es: Metadatos extra de apoyo (que no van a disco)
// en: Extra support metadata (not to be stored on disk)
struct {
    int32_t  position ; // es: posición de lectura/escritura
                        // en: read/write seek position
    int8_t   is_open  ; // es: 0: falso, 1: verdadero
                        // en: 0: false, 1: true
} inodes_x [NUM_INODES] ;

int8_t is_mounted = 0 ; // es: 0: falso, 1: verdadero
                        // en: 0: false, 1: true


/*
 * es: Funciones auxiliares
 * en: Auxiliar functions
 */

int min_value ( int a, int b )
{
    return (a < b) ? a : b ;
}

int max_value ( int a, int b )
{
    return (a > b) ? a : b ;
}

int debug_print_sizeof ( void )
{
   printf("\n") ;
   printf("Size of data structures:\n") ;
   printf(" * Size of Superblock: %ld bytes.\n", sizeof(TypeSuperblock)) ;
   printf(" * Size of InodeDisk:  %ld bytes.\n", sizeof(TypeInodeDisk)) ;
   printf(" * Size of InodeMap:   %ld bytes.\n", sizeof(TypeInodeMap)) ;
   printf(" * Size of BlockMap:   %ld bytes.\n", sizeof(TypeBlockMap)) ;

   return 1 ;
}

int debug_print_superblock ( void )
{
   printf("\n") ;
   printf("SuperBlock:\n") ;
   printf(" * numMagic:\t\t0x%x\n",      sblock.numMagic) ;
   printf(" * numInodes:\t\t%d\n",       sblock.numInodes) ;
   printf(" * numInodesBlocks:\t%d\n",   sblock.numInodesBlocks) ;
   printf(" * inodesPerBlock:\t%d\n",    sblock.inodesPerBlock) ;
   printf(" * numDataBlocks:\t%d\n",     sblock.numDataBlocks) ;
   printf(" * firstMapsBlock:\t%d\n",    sblock.firstMapsBlock) ;
   printf(" * firstInodeBlock:\t%d\n",   sblock.firstInodeBlock) ;
   printf(" * firstDataBlock:\t%d\n",    sblock.firstDataBlock) ;
   printf(" * sizeDevice:\t\t%d\n",      sblock.sizeDevice) ;

   return 1 ;
}


/*
 * es: Funciones secundarias
 * en: Secondary functions
 */

int nanofs_ialloc ( void )
{
    int i;

    // es: buscar un i-nodo libre
    // en: search for a free i-node
    for (i=0; i<sblock.numInodes; i++)
    {
          if (i_map[i] == 0)
          {
              // es: inodo ocupado ahora
              // en: set inode to used
              i_map[i] = 1 ;

              // es: valores por defecto en el i-nodo
              // en: set default values for the inode
              memset(&(inodes[i]), 0, sizeof(TypeInodeDisk)) ;

              // es: devolver identificador de i-nodo
              // en: return the inode id.
              return i ;
          }
    }

    return -1;
}

int nanofs_alloc ( void )
{
    char b[BLOCK_SIZE];
    int i;

    // es: buscar un bloque de datos libre
    // en: search for a free data block
    for (i=0; i<sblock.numDataBlocks; i++)
    {
          if (b_map[i] == 0)
          {
              // es: bloque ocupado ahora
              // en: data block used now
              b_map[i] = 1 ;

              // es: valores por defecto en el bloque
              // en: default values for the block
              memset(b, 0, BLOCK_SIZE) ;
              bwrite(DISK, sblock.firstDataBlock + i, b) ;

              // es: devolver identificador del bloque
              // en: return the block id.
              return i ;
          }
    }

    return -1;
}

int nanofs_ifree ( int inodo_id )
{
    // es: comprobar validez de inodo_id
    // en: check inode id.
    if (inodo_id > sblock.numInodes) {
        return -1;
    }

    // es: liberar i-nodo
    // en: free i-node
    i_map[inodo_id] = 0;

    return -1;
}

int nanofs_free ( int block_id )
{
    // es: comprobar validez de block_id
    // en: check block id.
    if (block_id > sblock.numDataBlocks) {
        return -1;
    }

    // es: liberar bloque
    // en: free block
    b_map[block_id] = 0;

    return -1;
}

int nanofs_namei ( char *fname )
{
   int i;

   // es: buscar i-nodo con name <fname>
   // en: search an i-node with name <fname>
   for (i=0; i<sblock.numInodes; i++)
   {
         if (! strcmp(inodes[i].name, fname)) {
               return i;
         }
   }

   return -1;
}

int nanofs_bmap ( int inodo_id, int offset )
{
    int b[BLOCK_SIZE/4] ;
    int logic_block ;

    // es: comprobar validez de inodo_id
    // en: check inode id.
    if (inodo_id > sblock.numInodes) {
        return -1;
    }

    // es: bloque lógico de datos asociado
    // en: logical block
    logic_block = offset / BLOCK_SIZE ;
    if (logic_block > (BLOCK_SIZE/4)) {
        return -1 ;
    }

    // es: devolver referencia a bloque directo 
    // en: return direct block
    if (0 == logic_block) {
        return inodes[inodo_id].directBlock[0] ;
    }

    // es: devolver referencia dentro de bloque indirecto
    // en: return indirect block
    bread(DISK, sblock.firstDataBlock + inodes[inodo_id].indirectBlock, b) ;
    return b[logic_block - 1] ;
}


/*
 * es: Funciones auxiliares para mkfs, mount y umount
 * en: Auxiliar functions for mkfs, mount, and umount
 */

int nanofs_meta_readFromDisk ( void )
{
    char b[BLOCK_SIZE] ;

    // es: leer bloque 0 de disco en sblock
    // en: read block 0 from disk to sbloques[0]
    bread(DISK, 0, b) ;
    memmove(&(sblock), b, sizeof(TypeSuperblock)) ;

    // es: leer los bloques para el mapa de i-nodos y mapa de bloques de datos
    // en: read the blocks where the i-node map and block map is stored
    bread(DISK, sblock.firstMapsBlock, b) ;
    memmove(&(i_map), b,                      sizeof(TypeInodeMap)) ;
    memmove(&(b_map), b+sizeof(TypeInodeMap), sizeof(TypeBlockMap)) ;

    // es: leer los i-nodos a memoria
    // en: read i-nodes to memory
    int inodesLeftToRead = sblock.numInodes ;
    for (int blocksRead=0; (inodesLeftToRead > 0); blocksRead++)
    {
         int inodesRead   = blocksRead*sblock.inodesPerBlock ;
         int inodesToPack = min_value(inodesLeftToRead, sblock.inodesPerBlock) ;

         bread(DISK, sblock.firstInodeBlock+blocksRead, b) ;
         memmove(&(inodes[inodesRead]), b, inodesToPack*sizeof(TypeInodeDisk)) ;

         inodesLeftToRead -= inodesToPack ;
    }

    debug_print_sizeof() ;
    debug_print_superblock() ;

    return 1;
}

int nanofs_meta_writeToDisk ( void )
{
    char b[BLOCK_SIZE] ;

    // es: escribir bloque 0 de sblock a disco
    // en: write block 0 to disk from sbloques[0]
     memset(b, 0, BLOCK_SIZE) ;
    memmove(b, &(sblock), sizeof(TypeSuperblock)) ;
    bwrite(DISK, 0, b) ;

    // es: escribir los bloques para el mapa de i-nodos y el mapa de bloques de datos
    // en: write the blocks where the i-node map and block map is stored
     memset(b, 0, BLOCK_SIZE) ;
    memmove(b,                        &(i_map), sizeof(TypeInodeMap)) ;
    memmove(b + sizeof(TypeInodeMap), &(b_map), sizeof(TypeBlockMap)) ;
    bwrite(DISK, sblock.firstMapsBlock, b) ;

    // es: escribir los i-nodos a disco
    // en: write i-nodes to disk
    int inodesLeftToWrite = sblock.numInodes ;
    for (int blocksWritten=0; (inodesLeftToWrite > 0); blocksWritten++)
    {
         int inodesWritten = blocksWritten*sblock.inodesPerBlock ;
         int inodesToPack  = min_value(inodesLeftToWrite, sblock.inodesPerBlock) ;

          memset(b, 0, BLOCK_SIZE) ;
         memmove(b, &(inodes[inodesWritten]), inodesToPack*sizeof(TypeInodeDisk)) ;
         bwrite(DISK, sblock.firstInodeBlock+blocksWritten, b) ;

         inodesLeftToWrite -= inodesToPack ;
    }

    debug_print_sizeof() ;
    debug_print_superblock() ;

    return 1 ;
}

int nanofs_meta_setDefault ( int dev_size )
{
    // es: inicializar a los valores por defecto del superbloque, mapas e i-nodos
    // en: set the default values of the superblock, inode map, etc.
    sblock.numMagic          = 0x12345 ; // ayuda a comprobar que se haya creado por nuestro mkfs
    sblock.numInodes         = NUM_INODES ;
    sblock.numInodesBlocks   = (NUM_INODES * sizeof(TypeInodeDisk) + BLOCK_SIZE - 1) / BLOCK_SIZE ;
    sblock.inodesPerBlock    = BLOCK_SIZE / sizeof(TypeInodeDisk) ;
    sblock.numDataBlocks     = NUM_DATA_BLOCKS ;
    sblock.firstMapsBlock    = 1 ;
    sblock.firstInodeBlock   = 2 ;
    sblock.firstDataBlock    = 1 + 1 + sblock.numInodesBlocks ; // 1:sb + 1:maps + 3:inodes
    sblock.sizeDevice        = dev_size ;

    for (int i=0; i<sblock.numInodes; i++) {
         i_map[i] = 0; // free
    }

    for (int i=0; i<sblock.numDataBlocks; i++) {
         b_map[i] = 0; // free
    }

    for (int i=0; i<sblock.numInodes; i++) {
         memset(&(inodes[i]), 0, sizeof(TypeInodeDisk) ) ;
    }

    return 1;
}

int nanofs_mount ( void )
{
    if (1 == is_mounted) {
        return -1 ;
    }

    // es: leer los metadatos del sistema de ficheros de disco a memoria
    // en: read the metadata file system from disk
    nanofs_meta_readFromDisk() ;

    // es: comprueba el número mágico
    // en: check magic number
    if (0x12345 != sblock.numMagic) {
        return -1 ;
    }

    // es: montar
    // en: mounted
    is_mounted = 1 ; // 0: falso, 1: verdadero

    return 1 ;
}

int nanofs_umount ( void )
{
    // es: si NO mountado -> error
    // en: if NOT mounted -> error
    if (0 == is_mounted) {
        return -1 ;
    }

    // es: si algún fichero está abierto -> error
    // en: if any file is open -> error
    for (int i=0; i<sblock.numInodes; i++) {
    if (1 == inodes_x[i].is_open)
        return -1 ;
    }

    // es: escribir los metadatos del sistema de ficheros de memoria a disco
    // en: write the metadata file system into disk
    nanofs_meta_writeToDisk() ;

    // es: desmontar
    // en: unmounted
    is_mounted = 0 ; // 0: falso, 1: verdadero

    return 1 ;
}

int nanofs_mkfs ( int dev_size )
{
    char b[BLOCK_SIZE];

    // es: establecer los valores por defecto en memoria
    // en: set default values in memory
    nanofs_meta_setDefault(dev_size) ;

    // es: escribir el sistema de ficheros inicial a disco
    // en: write the default file system into disk
    nanofs_meta_writeToDisk() ;

    // es: rellenar los bloques de datos con ceros
    // en: write empty data blocks
    memset(b, 0, BLOCK_SIZE) ;
    for (int i=0; i < sblock.numDataBlocks; i++) {
         bwrite(DISK, sblock.firstDataBlock + i, b) ;
    }

    return 1;
}


/*
 * es: Funciones principales
 * en: Main functions
 */

int nanofs_open ( char *name )
{
    int inodo_id ;

    // es: obtener inodo a partir del nombre
    // en: get inode id from name
    inodo_id = nanofs_namei(name) ;
    if (inodo_id < 0) {
        return inodo_id ;
    }

    inodes_x[inodo_id].position = 0 ;
    inodes_x[inodo_id].is_open  = 1 ;

    return inodo_id ;
}

int nanofs_close ( int fd )
{
     // es: comprobar parámetros
     // en: check params
     if ( (fd < 0) || (fd >= sblock.numInodes) )
     {
         return -1 ;
     }

     inodes_x[fd].position = 0 ;
     inodes_x[fd].is_open  = 0 ;

     return 1 ;
}

int nanofs_creat ( char *name )
{
    int inodo_id ;

    // es: comprueba si existe el fichero
    // en: check file exist
    inodo_id = nanofs_namei(name) ;
    if (inodo_id >= 0) {
        return -1 ;
    }

    inodo_id = nanofs_ialloc() ;
    if (inodo_id < 0) {
        return inodo_id ;
    }

    strcpy(inodes[inodo_id].name, name) ;
    inodes[inodo_id].type           = T_FILE ;
    inodes[inodo_id].directBlock[0] = -5 ;
    inodes_x[inodo_id].position = 0 ;
    inodes_x[inodo_id].is_open  = 1 ;

    return inodo_id ;
}

int nanofs_unlink ( char * name )
{
     int inodo_id ;

     // es: obtener inodo a partir del nombre
     // en: get inode id from name
     inodo_id = nanofs_namei(name) ;
     if (inodo_id < 0) {
         return inodo_id ;
     }

     nanofs_free(inodes[inodo_id].directBlock[0]) ;
     memset(&(inodes[inodo_id]), 0, sizeof(TypeInodeDisk)) ;
     nanofs_ifree(inodo_id) ;

    return 1 ;
}

int nanofs_read ( int fd, char *buffer, int size )
{
     char b[BLOCK_SIZE] ;

     // es: comprobar parámetros
     // en: check params
     if ( (fd < 0) || (fd >= sblock.numInodes) )
     {
         return -1 ;
     }

     int readed = 0 ;
     while (size > readed)
     {
         // es: obtener bloque
         // en: get block
         int position_within_block = inodes_x[fd].position % BLOCK_SIZE ;
         int to_read  = BLOCK_SIZE - position_within_block ;
             to_read  = (to_read > size) ? size : to_read ;
         int block_id = nanofs_bmap(fd, inodes_x[fd].position) ;

         // es: lee-escribe bloque + toma porción pedida por el usuario
         // en: read-write  block  + get portion requested by user
         bread(DISK, sblock.firstDataBlock+block_id, b) ;
         memmove(buffer+readed, b+position_within_block, to_read) ;

         inodes_x[fd].position = inodes_x[fd].position + to_read ;
         readed = readed + to_read ;
     }

     return readed ;
}

int nanofs_write ( int fd, char *buffer, int size )
{
     char b[BLOCK_SIZE] ;

     // es: comprobar parámetros
     // en: check params
     if ( (fd < 0) || (fd >= sblock.numInodes) )
     {
         return -1 ;
     }

     int written = 0 ;
     while (size > written)
     {
         // es: obtener bloque
         // en: get block
         int position_within_block = inodes_x[fd].position % BLOCK_SIZE ;
         int to_write = BLOCK_SIZE - position_within_block ;
             to_write = (to_write > size) ? size : to_write ;

         int block_id = nanofs_bmap(fd, inodes_x[fd].position) ;
         if (-5 == block_id) {
             block_id = nanofs_alloc() ;
             if (block_id < 0) {
                 return -1 ;
             }
             inodes[fd].directBlock[0] = block_id ;
         }

         // es: lee bloque + toma porción pedida por el usuario
         // en: read block + get portion requested by user
         bread(DISK, sblock.firstDataBlock+block_id, b) ;
         memmove(b+position_within_block, buffer+written, to_write) ;
         bwrite(DISK, sblock.firstDataBlock+block_id, b) ;

         inodes_x[fd].position = inodes_x[fd].position + to_write ;
           inodes[fd].size     = max_value(inodes_x[fd].position, inodes[fd].size) ;
         written = written + to_write ;
     }

     return written ;
}

int nanofs_lseek ( int fd, int offset, int whence )
{
     // es: comprobar parámetros
     // en: check params
     if ( (fd < 0) || (fd >= sblock.numInodes) )
     {
         return -1 ;
     }

     // es: salta a la posici'on pedida
     // en: seek to the requested offset
     switch (whence) 
     {
         case SEEK_SET:
              inodes_x[fd].position = offset ;
              break ;
         case SEEK_CUR:
              inodes_x[fd].position = inodes_x[fd].position + offset ;
              break ;
         case SEEK_END:
              inodes_x[fd].position = inodes[fd].size + offset ;
              break ;
     }

     // es: devolver posici'on resultante
     // en: return current position
     return inodes_x[fd].position ;
}

