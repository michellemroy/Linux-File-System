# Linux-File-System
Custom Linux File System made with FUSE Library.
Multi tier folders. Allows for creation, reading, deleleting and editing of files and folders. Persistence of the files and folders is also allowed. The filesystem is created inside a root(mounting point) folder which needs to be created inside the same folder that contains the code(filesys.c).

To run:

1.) First Compile with :
gcc filesys.c -o filesys `pkg-config fuse --cflags --libs`

2.)Mount the file system with :
./filesys -f [mount point] //mount point is the directory to mount the filesystem to eg. root

3.)to run it open/cd into the filesytem and run commands via terminal or via he filebrowser.
