MemoryManager
=============

A memory manager which provides support for Dynamic Caching in WSE.

The `MemoryManager` module is responsible for maniplulating a pre-allocated
memory space.

The call chain: ListHandler --> CacheFrame --> MemoryManager

Notes:

1) This Frame is multi-thread safe.

2) The `MemoryManager` module is transparent to the user (who use this Frame).

