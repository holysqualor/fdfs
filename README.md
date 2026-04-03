# FDFS


**Fractal Distribution File System** is a file system that uses a fractal/recursive distribution algorithm. Initially, the project was conceived as a standalone file system, but during implementation it became clear that the concept is more suitable as a modification of an existing file system.


The algorithm evenly distributes files across the disk, minimizing fragmentation and drive wear. For the algorithm to be most effective, the file system must support extents.
