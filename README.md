**WORK-IN-PROGRESS**

**Overview**

This project is a C++ in-memory asset registry library. Callers request assets by path and receive back a handle. Behind the scenes, an LRU cache controls which assets are resident in memory using reference-counted ownership. 
When handles go out of scope, the asset registry releases their memory automatically via RAII. 
The eviction policy is LRU (in this context, least recently freed) implemented with a doubly linked list paired with a hashmap of per-entry iterators for O(1) insertion and removal.
Built with modern C++ features including std::optional, std::span, std::views, std::filesystem, std::enable_shared_from_this, and RAII throughout, with a full unit test suite written in gTest.

I also built a small raylib visualization that demonstrates the cache in action: as the player moves through the world, a top-down view frustum determines which assets the registry manages. 
Textures in the view are loaded and managed by the registry; those outside are grayed out and handled by standard raylib texture loading. 
It's a small-scale implementation of the streaming patterns you'd find in larger engines: reference-counted ownership, a cache layer between raw data and GPU resources, and visibility-driven residency, intentionally scoped to isolate and demonstrate the core ideas in a context where you can see them working directly.

**Video**

As the character moves around the world, assets in view are loaded in using the cache. If the cache is full, eviction will occur based on an LRU policy. 

https://github.com/user-attachments/assets/8b65619c-1428-4e11-91c4-4e9a78c1e903

**Technical Details**

Written in C++ 23. gTest for unit tests. Raylib for visualization. Library code under `/lib`. Game code under `/game`.

**Next Steps**
- Make the library thread-safe
- Support for soft and hard asset dependencies
- Support for asset "bundles" and some sort of build-time asset bundling tool
- Option to load assets from remote (ex: CDN)
