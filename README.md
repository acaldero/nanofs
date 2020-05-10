# nanofs
nano file system: Operating System Design example of minimum file system

## Compile
  * make clean
  * make compile

## Execute included example
  * make createdisk
  * ./nanofs

```bash
     Tests: mkfs + mount + umount
     * nanofs_mkfs(32) -> 1
     * nanofs_mount() -> 1
     * nanofs_umount() -> 1
     Tests: mount + creat + write + close + umount
     * nanofs_mount() -> 1
     * nanofs_creat('test1.txt') -> 0
     * nanofs_write(0,'hola mundo...',13) -> 13
     * nanofs_close(0) -> 1
     * nanofs_umount() -> 1
     Tests: mount + open + read + close + unlink + umount
     * nanofs_mount() -> 1
     * nanofs_open('test1.txt') -> 0
     * nanofs_read(0,'',13) -> 13 (hola mundo...)
     * nanofs_close(0) -> 1
     * nanofs_unlink('test1.txt') -> 1
     * nanofs_umount() -> 1
     Size of data structures:
     * Size of Superblock: 36 bytes.
     * Size of InodeDisk:  84 bytes.
     * Size of InodeMap:   10 bytes.
     * Size of BlockMap:   20 bytes.
     SuperBlock:
     * numMagic:		0x12345
     * numInodes:		10
     * numInodesBlocks:		1
     * inodesPerBlock:		12
     * numDataBlocks:		20
     * firstMapsBlock:		1
     * firstInodeBlock:		2
     * firstDataBlock:		3
     * sizeDevice:		32

