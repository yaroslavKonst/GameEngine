#ifndef _MEMORY_MANAGER_H
#define _MEMORY_MANAGER_H

#include <list>
#include <vector>
#include <mutex>
#include <vulkan/vulkan.h>

class MemoryManager
{
public:
	struct Allocation
	{
		VkDeviceMemory Memory;
		uint32_t Size;
		uint32_t Offset;
		void* Mapping;
	};

	MemoryManager(
		VkDevice device,
		uint32_t pageSize,
		uint32_t memoryTypeIndex,
		uint32_t alignment,
		bool mapped = false);
	~MemoryManager();

	Allocation Allocate(uint32_t size);
	void Free(Allocation allocation);

private:
	struct PageDescriptor
	{
		VkDeviceMemory memory;
		std::vector<bool> data;
		void* mapping;
	};

	VkDevice _device;
	std::list<PageDescriptor> _pages;
	uint32_t _pageSize;
	uint32_t _alignment;
	uint32_t _memoryTypeIndex;
	uint32_t _sectorCount;

	bool _mapped;

	std::mutex _mutex;

	void AddPage();
};

#endif
