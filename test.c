
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


int debug_test_mkfs_mount_umount ()
{
   int  ret = 1 ;

   printf("\n") ;
   printf("Tests: mkfs + mount + umount\n") ;

   if (ret != -1)
   {
       printf(" * nanofs_mkfs(32) -> ") ;
       ret = nanofs_mkfs(32) ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_mount() -> ") ;
       ret = nanofs_mount() ;
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

int debug_test_mount_creat_write_close_umount ()
{
   int  ret = 1 ;
   int  fd  = 1 ;

   printf("\n") ;
   printf("Tests: mount + creat + write + close + umount\n") ;

   if (ret != -1)
   {
       printf(" * nanofs_mount() -> ") ;
       ret = nanofs_mount() ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_creat('test1.txt') -> ") ;
       ret = fd = nanofs_creat("test1.txt") ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       char *str1 = "hola mundo..." ;

       printf(" * nanofs_write(%d,'%s',%ld) -> ", ret, str1, strlen(str1)) ;
       ret = nanofs_write(fd, str1, strlen(str1)) ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_close(%d) -> ", fd) ;
       ret = nanofs_close(fd) ;
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

int debug_test_mount_open_read_close_unlink_umount ()
{
   int   ret = 1 ;
   int   fd  = 1 ;
   char  str2[20] ;

   printf("\n") ;
   printf("Tests: mount + open + read + close + unlink + umount\n") ;

   if (ret != -1)
   {
       printf(" * nanofs_mount() -> ") ;
       ret = nanofs_mount() ;
       printf("%d\n", ret) ;
   }

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
       printf(" * nanofs_close(%d) -> ", fd) ;
       ret = nanofs_close(fd) ;
       printf("%d\n", ret) ;
   }

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


int main()
{
   debug_test_mkfs_mount_umount() ;
   debug_test_mount_creat_write_close_umount() ;
   debug_test_mount_open_read_close_unlink_umount() ;

   return 0 ;
}

