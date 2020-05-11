
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


#include "block.h"


/*
 *  es: Interfaz servidor de bloques (block-read + block-write)
 *  en: Block Disk Interface
 */

int bread ( char *devname, int bid, void *buffer )
{
   FILE *fd1 ;
   int ret ;

   // 1) es: abre el dispositivo de disco "devname"
   // 1) en: open "devname" disk
      fd1 = fopen(devname, "r") ;
      if (NULL == fd1) {
          return -1 ;
      }

   // 2) es: salta al bloque bid y lo lee. Identificador de bloque empieza en cero.
   // 2) en: seek to the bid-th block and read it. Read operation starts at 0
      ret = fseek(fd1, bid*BLOCK_SIZE, SEEK_SET) ;
      if (0 == ret) { 
          ret = fread(buffer, BLOCK_SIZE, 1, fd1) ;
          fclose(fd1) ;
      }

   // 3) es: devuelve ok
   // 3) en: return ok
      return ret ;
}

int bwrite ( char *devname, int bid, void *buffer )
{
   FILE *fd1 ;
   int ret ;

   // 1) es: abre el dispositivo de disco "devname"
   // 1) en: open "devname" disk
      fd1 = fopen(devname, "r+") ;
      if (NULL == fd1) {
          return -1 ;
      }

   // 2) es: salta al bloque bid y lo escribe. Identificador de bloque empieza en cero.
   // 2) en: seek to the bid-th block and write it. Write operation starts at 0
      ret = fseek(fd1, bid*BLOCK_SIZE, SEEK_SET) ;
      if (0 == ret) { 
          ret = fwrite(buffer, BLOCK_SIZE, 1, fd1) ;
          fclose(fd1) ;
      }

   // 3) es: devuelve ok
   // 3) en: return ok
      return ret ;
}

