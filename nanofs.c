
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


/*
 *  es: Interfaz servidor de bloques (block-read + block-write)
 *  en: Block Disk Interface
 */

#define DISK "tmpfile.dat"
#define BLOCK_SIZE 1024

int bread ( char *devname, int bid, void *buffer )
{
   FILE *fd1 ;
   int ret ;

   fd1 = fopen(devname, "r") ;
   fseek(fd1, bid*BLOCK_SIZE, SEEK_SET) ;
   ret = fread(buffer, BLOCK_SIZE, 1, fd1) ;
   fclose(fd1) ;
   return ret ;
}

int bwrite ( char *devname, int bid, void *buffer )
{
   FILE *fd1 ;
   int ret ;

   fd1 = fopen(devname, "r+") ;
   fseek(fd1, bid*BLOCK_SIZE, SEEK_SET) ;
   ret = fwrite(buffer, BLOCK_SIZE, 1, fd1) ;
   fclose(fd1) ;
   return ret ;
}


/*
 *  es: (1) Estructura de datos en disco
 *  en: (1) Disk layout structures
 */

#define NUM_INODES         10
#define NUM_DATA_BLOCKS    20
#define NUM_INODES_BLOCKS  (NUM_INODES*sizeof(TypeInodeDisk)/BLOCK_SIZE)
#define INODES_PER_BLOCK   (BLOCK_SIZE / sizeof(TypeInodeDisk))

#define T_FILE       1
#define T_DIRECTORY  2


// Superblock
typedef struct {
    uint32_t numMagic;	          /* Número mágico del superbloque (por ejemplo 0x12345) */
                                  /* Superblock magic number: 0x12345 */
    uint32_t numInodeMapBlocks;   /* Número de bloques de disco del mapa inodes */
                                  /* Number of block for the inode map */
    uint32_t numDataMapBlocks;    /* Número de bloques de disco del mapa  datos */
                                  /* Number of block for the data map */
    uint32_t numInodes; 	  /* Número de inodes en el dispositivo */
                                  /* Number of inodes in the device */
    uint32_t numInodesBlocks;     /* Número de bloques de inodos en el disp. */
                                  /* Number of inodos blocks in the device */
    uint32_t numDataBlocks;       /* Número de bloques de datos en el disp. */
                                  /* Number of data blocks in the device */
    uint32_t firstInodeBlock;	  /* 1º inodo del disp. (inodo raíz) */
                                  /* Block number of the first inode (root inode) */
    uint32_t firstDataBlock;      /* 1º bloque de disco para datos tras metadatos */
                                  /* Block number of the first data block */
    uint32_t sizeDevice;	  /* Tamaño total del disp. (en bytes) */
                                  /* Total size of the device in bytes */
} TypeSuperblock ;


// Inodes
typedef struct {
    uint32_t type;	              /* T_FILE o T_DIRECTORY */
	                              /* T_FILE or T_DIRECTORY */
    char name[200];	              /* Nombre del fichero/directorio asociado (termina en cero) */
	                              /* Name of the associated file/directory (end with '\0')*/
    uint16_t inodesContents[200];     /* si (type == T_DIR) -> lista de los inodes del directorio */
	                              /* if (type == T_DIR) -> list of the inodes in directory */
    uint32_t size;	              /* Tamaño actual del fichero en bytes */
	                              /* Size in bytes */
    uint32_t directBlock[1];          /* Número del bloque directo */
	                              /* Number of the direct block */
    uint32_t indirectBlock;	      /* Número del bloque indirecto */
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
 *  es: (2) Estructura de datos en memoria
 *  en: (2) Memory data structures
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
 *
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
    int bloque_logico ;

    // es: comprobar validez de inodo_id
    // en: check inode id.
    if (inodo_id > sblock.numInodes) {
        return -1;
    }

    // es: bloque lógico de datos asociado
    // en: logical block
    bloque_logico = offset / BLOCK_SIZE ;
    if (bloque_logico > (BLOCK_SIZE/4)) {
        return -1 ;
    }

    // es: devolver referencia a bloque directo 
    // en: return direct block
    if (0 == bloque_logico) {
        return inodes[inodo_id].directBlock[0] ;
    }

    // es: devolver referencia dentro de bloque indirecto
    // en: return indirect block
    bread(DISK, sblock.firstDataBlock + inodes[inodo_id].indirectBlock, b);
    return b[bloque_logico - 1] ;
}


/*
 *
 */

int nanofs_meta_readFromDisk ( void )
{
    char b[BLOCK_SIZE] ;

    // es: leer bloque 0 de disco en sblock
    // en: read block 0 from disk to sbloques[0]
    bread(DISK, 0, b) ;
    memmove(&(sblock), b, sizeof(TypeSuperblock)) ;

    // es: leer los bloques para el mapa de i-nodos
    // en: read the blocks where the i-node map is stored
    bread(DISK, 1, b) ;
    memmove(&(i_map), b, sizeof(TypeInodeMap)) ;

    // es: leer los bloques para el mapa de bloques de datos
    // en: read the blocks where the block map is stored
    bread(DISK, 2, b) ;
    memmove(&(b_map), b, sizeof(TypeBlockMap)) ;

    // es: leer los i-nodos a memoria
    // en: read i-nodes to memory
    for (int i=0; i<sblock.numInodesBlocks; i++) 
    {
         bread(DISK, sblock.firstInodeBlock+i, b) ;
         memmove(&(inodes[i*INODES_PER_BLOCK]), b, INODES_PER_BLOCK*sizeof(TypeInodeDisk)) ;
    }

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

    // es: escribir los bloques para el mapa de i-nodos
    // en: write the blocks where the i-node map is stored
    memset(b, 0, BLOCK_SIZE) ;
    memmove(b, &(i_map), sizeof(TypeInodeMap)) ;
    bwrite(DISK, 1, b) ;

    // es: escribir los bloques para el mapa de bloques de datos
    // en: write the blocks where the block map is stored
    memset(b, 0, BLOCK_SIZE) ;
    memmove(b, &(b_map), sizeof(TypeBlockMap)) ;
    bwrite(DISK, 2, b) ;

    // es: escribir los i-nodos a disco
    // en: write i-nodes to disk
    for (int i=0; i<sblock.numInodesBlocks; i++) 
    {
         memset(b, 0, BLOCK_SIZE) ;
         memmove(b, &(inodes[i*INODES_PER_BLOCK]), INODES_PER_BLOCK*sizeof(TypeInodeDisk)) ;
         bwrite(DISK, sblock.firstInodeBlock+i, b) ;
    }

    return 1 ;
}

int nanofs_meta_setDefault ( void )
{
    // es: inicializar a los valores por defecto del superbloque, mapas e i-nodos
    // en: set the default values of the superblock, inode map, etc.
    sblock.numMagic          = 0x12345 ; // ayuda a comprobar que se haya creado por nuestro mkfs
    sblock.numInodes         = NUM_INODES ;
    sblock.numInodesBlocks   = NUM_INODES_BLOCKS ;
    sblock.numInodeMapBlocks = 1 ;
    sblock.numDataMapBlocks  = 1 ;
    sblock.firstInodeBlock   = 1 ;
    sblock.numDataBlocks     = NUM_DATA_BLOCKS ;
    sblock.firstDataBlock    = 12 ;
    sblock.sizeDevice        = 32 ;

    for (int i=0; i<sblock.numInodes; i++) {
         i_map[i] = 0; // free
    }

    for (int i=0; i<sblock.numDataBlocks; i++) {
         b_map[i] = 0; // free
    }

    for (int i=0; i<sblock.numInodes; i++) {
         memset(&(inodes[i]), 0, sizeof(TypeInodeDisk) );
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

int nanofs_mkfs ( void )
{
    char b[BLOCK_SIZE];

    // es: establecer los valores por defecto en memoria
    // en: set default values in memory
    nanofs_meta_setDefault() ;

    // es: escribir el sistema de ficheros inicial a disco
    // en: write the default file system into disk
    nanofs_meta_writeToDisk() ;

    // es: rellenar los bloques de datos con ceros
    // en: write empty data blocks
    memset(b, 0, BLOCK_SIZE);
    for (int i=0; i < sblock.numDataBlocks; i++) {
         bwrite(DISK, sblock.firstDataBlock + i, b) ;
    }

    return 1;
}


/*
 *
 */

int nanofs_open ( char *name )
{
    int inodo_id ;

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
     if (fd < 0) {
         return fd ;
     }

     inodes_x[fd].position = 0 ;
     inodes_x[fd].is_open  = 0 ;

     return 1 ;
}

int nanofs_creat ( char *name )
{
    int b_id, inodo_id ;

    inodo_id = nanofs_ialloc() ;
    if (inodo_id < 0) {
        return inodo_id ;
    }

    b_id = nanofs_alloc();
    if (b_id < 0) {
        nanofs_ifree(inodo_id) ;
        return b_id ;
    }

    strcpy(inodes[inodo_id].name, name) ;
    inodes[inodo_id].type           = T_FILE ;
    inodes[inodo_id].directBlock[0] = b_id ;
    inodes_x[inodo_id].position = 0 ;
    inodes_x[inodo_id].is_open  = 1 ;

    return inodo_id ;
}

int nanofs_unlink ( char * name )
{
     int inodo_id ;

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
     int b_id ;

     if (inodes_x[fd].position+size > inodes[fd].size) {
         size = inodes[fd].size - inodes_x[fd].position ;
     }
     if (size <= 0) {
         return 0 ;
     }

     b_id = nanofs_bmap(fd, inodes_x[fd].position) ;
     bread(DISK, sblock.firstDataBlock+b_id, b) ;
     memmove(buffer, b+inodes_x[fd].position, size) ;
     inodes_x[fd].position += size ;

     return size ;
}

int nanofs_write ( int fd, char *buffer, int size )
{
     char b[BLOCK_SIZE] ;
     int b_id ;

     if (inodes_x[fd].position+size > BLOCK_SIZE) {
         size = BLOCK_SIZE - inodes_x[fd].position ;
     }
     if (size <= 0) {
         return 0 ;
     }

     b_id = nanofs_bmap(fd, inodes_x[fd].position) ;
     bread(DISK, sblock.firstDataBlock+b_id, b) ;
     memmove(b+inodes_x[fd].position, buffer, size) ;
     bwrite(DISK, sblock.firstDataBlock+b_id, b) ;
     inodes_x[fd].position += size ;
     inodes[fd].size       += size ;

     return size ;
}


/*
 *
 */

int main()
{
   int   ret = 1 ;
   int   fd  = 1 ;
   char *str1 = "hola mundo..." ;
   char  str2[20] ;

   printf("\n") ;
   printf("Size of data structures:\n") ;
   printf(" * Size of Superblock: %ld bytes.\n", sizeof(TypeSuperblock)) ;
   printf(" * Size of InodeDisk:  %ld bytes.\n", sizeof(TypeInodeDisk)) ;
   printf(" * Size of InodeMap:   %ld bytes.\n", sizeof(TypeInodeMap)) ;
   printf(" * Size of BlockMap:   %ld bytes.\n", sizeof(TypeBlockMap)) ;

   printf("\n") ;
   printf("Tests:\n") ;

   //
   // mkfs-mount
   //
   if (ret != -1)
   {
       printf(" * nanofs_mkfs() -> ") ;
       ret = nanofs_mkfs() ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_mount() -> ") ;
       ret = nanofs_mount() ;
       printf("%d\n", ret) ;
   }

   //
   // creat-write-close
   //
   if (ret != -1)
   {
       printf(" * nanofs_creat('test1.txt') -> ") ;
       ret = fd = nanofs_creat("test1.txt") ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_write(%d,'%s',%ld) -> ", ret, str1, strlen(str1)) ;
       ret = nanofs_write(fd, str1, strlen(str1)) ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_close(%d) -> ", ret) ;
       ret = nanofs_close(fd) ;
       printf("%d\n", ret) ;
   }

   //
   // open-read-close
   //
   if (ret != -1)
   {
       printf(" * nanofs_open('test1.txt') -> ") ;
       ret = fd = nanofs_open("test1.txt") ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       memset(str2, 0, 20) ;
       printf(" * nanofs_read(%d,'%s',%d) -> ", ret, str2, 13) ;
       ret = nanofs_read(fd, str2, 13) ;
       printf("%d (%s)\n", ret, str2) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_close(%d) -> ", ret) ;
       ret = nanofs_close(fd) ;
       printf("%d\n", ret) ;
   }

   //
   // unlink-umount
   //
   if (ret != -1)
   {
       printf(" * nanofs_unlink('test1.txt') -> ") ;
       ret = nanofs_unlink("test1.txt") ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_umount() -> ") ;
       ret = nanofs_umount() ;
       printf("%d\n", ret) ;
   }

   return 0 ;
}

