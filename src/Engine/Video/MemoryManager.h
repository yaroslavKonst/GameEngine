#ifndef _MEMORY_MANAGER_H
#define _MEMORY_MANAGER_H

#include <list>
#include <vector>
#include <map>
#include <set>
#include <vulkan/vulkan.h>

class IntervalStorage
{
public:
	IntervalStorage(size_t minLimit, size_t maxLimit);
	~IntervalStorage();

	bool Find(size_t length, size_t& begin);
	void Allocate(size_t begin, size_t length);
	void Free(size_t begin, size_t length);

	size_t Allocated()
	{
		return _allocated;
	}

private:
	struct Interval
	{
		size_t Begin;
		size_t End;

		std::map<size_t, Interval*>::iterator BeginIterator;
		std::map<size_t, Interval*>::iterator EndIterator;
		std::multimap<size_t, Interval*>::iterator LengthIterator;
	};

	std::map<size_t, Interval*> _intervalBegin;
	std::map<size_t, Interval*> _intervalEnd;
	std::multimap<size_t, Interval*> _intervalLength;

	std::set<Interval*> _intervals;

	size_t _minLimit;
	size_t _maxLimit;

	size_t _allocated;

	void AddInterval(Interval* interval);
	void RemoveInterval(Interval* interval);
};

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
		IntervalStorage* data;
		void* mapping;
	};

	VkDevice _device;
	std::list<PageDescriptor> _pages;
	uint32_t _pageSize;
	uint32_t _alignment;
	uint32_t _memoryTypeIndex;
	uint32_t _sectorCount;

	bool _mapped;

	void AddPage();
};

#endif
