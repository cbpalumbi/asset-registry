This project is an in-memory asset registry and cache written in C++ 20, designed to manage the lifecycle of assets with precise control over memory usage and eviction. 
Assets are loaded from disk into a fixed-capacity cache and exposed to consumers via RAII-managed AssetRef handles, which automatically notify the registry when they are released. 
The eviction policy is LRU (in this context, least recently freed) implemented with a doubly linked list paired with a hashmap of per-entry iterators for O(1) insertion and removal.
Built with modern C++ features including std::optional, std::span, std::views, std::filesystem, std::enable_shared_from_this, and RAII throughout, with a full unit test suite written in gTest.

I was inspired by asset loading systems in game engines, such as Unity Addressables and Unreal Engine Asset Manager. Future work includes support for asynchronous loads and a visualization tool to see cache usage in real time.
