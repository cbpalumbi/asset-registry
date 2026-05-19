**Overview**
This project is a C++ in-memory asset registry library. Callers request assets by path and receive back a handle. Behind the scenes, an LRU cache controls which assets are resident in memory using reference-counted ownership. 
When handles go out of scope, the asset registry releases their memory automatically.
To demonstrate the system in a real context, I built a small raylib game that visualizes the cache in action: as the player moves through the world, a top-down view frustum determines which assets the cache manages. 
Textures inside the frustum are loaded and evicted automatically as the frustum moves; those outside it are grayed out and handled by standard raylib texture loading. 
It's a small-scale implementation of the streaming patterns you'd find in larger engines: reference-counted ownership, a cache layer between raw data and GPU resources, and visibility-driven residency, intentionally scoped to isolate and demonstrate the core ideas in a context where you can see them working directly.

**Video**
As the character moves around the world, assets in view are loaded in using the cache. If the cache is full, eviction will occur based on an LRU policy. 

https://github.com/user-attachments/assets/4c16547b-4bf7-4486-853f-209b27095724

**Technical Details**
Written in C++ 23. gTest for unit tests. Raylib for visualization. Library code under `/lib`. Game code under `/game`.
