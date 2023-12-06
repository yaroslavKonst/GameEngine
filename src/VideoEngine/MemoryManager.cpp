#include "MemoryManager.h"

#include <stdexcept>

#include "../Logger/logger.h"

MemoryManager::MemoryManager(
	VkDevice device,
	uint32_t pageSize,
	uint32_t memoryTypeIndex,
	uint32_t alignment,
	bool mapped)
{
	_device = device;
	_pageSize = (pageSize / alignment + 1) * alignment;
	_alignment = alignment;
	_sectorCount = _pageSize / _alignment;
	_memoryTypeIndex = memoryTypeIndex;

	_mapped = mapped;

	AddPage();

	Logger::Verbose() << "New memory manager. Index: " <<
		_memoryTypeIndex << ". Alignment: " << _alignment <<
		". Mapped: " << mapped;

	Logger::Verbose() << "Page size " << _pageSize;
}

MemoryManager::~MemoryManager()
{
	uint32_t leakedSectors = 0;

	for (auto& page : _pages) {
		if (_mapped) {
			vkUnmapMemory(
				_device,
				page.memory);
		}

		vkFreeMemory(_device, page.memory, nullptr);

		leakedSectors += page.data->Allocated();

		delete page.data;
	}

	Logger::Verbose() <<
		"Freed memory manager. Index: " << _memoryTypeIndex <<
		", alignment: " << _alignment << ", mapped: " << _mapped <<
		". Leaks: " << leakedSectors;
}

void MemoryManager::AddPage()
{
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = _pageSize;
	allocInfo.memoryTypeIndex = _memoryTypeIndex;

	PageDescriptor page;
	page.data = new IntervalStorage(0, _sectorCount - 1);

	VkResult res = vkAllocateMemory(
		_device,
		&allocInfo,
		nullptr,
		&page.memory);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate device memory.");
	}

	if (_mapped) {
		vkMapMemory(
			_device,
			page.memory,
			0,
			_pageSize,
			0,
			&page.mapping);
	}

	_pages.push_back(page);

	Logger::Verbose() <<
		"New page for memory manager with index " <<
		_memoryTypeIndex << ", alignment " << _alignment;
}

MemoryManager::Allocation MemoryManager::Allocate(uint32_t size)
{
	if (size > _pageSize)
	{
		throw std::runtime_error(
			std::string("Memory allocation size ") +
			std::to_string(size) + " is greater than page size " +
			std::to_string(_pageSize));
	}

	uint32_t requiredSectors = (size - 1) / _alignment + 1;

	for (auto& page : _pages) {
		size_t begin;
		bool spaceFound = page.data->Find(requiredSectors, begin);

		if (spaceFound) {
			page.data->Allocate(begin, requiredSectors);

			Allocation allocation;
			allocation.Memory = page.memory;
			allocation.Offset = _alignment * begin;
			allocation.Size = size;

			if (_mapped) {
				allocation.Mapping =
					(char*)page.mapping +
					allocation.Offset;
			} else {
				allocation.Mapping = nullptr;
			}

			return allocation;
		}
	}

	AddPage();

	_pages.back().data->Allocate(0, requiredSectors);

	Allocation allocation;
	allocation.Memory = _pages.back().memory;
	allocation.Offset = 0;
	allocation.Size = size;

	if (_mapped) {
		allocation.Mapping = _pages.back().mapping;
	} else {
		allocation.Mapping = nullptr;
	}

	return allocation;
}

void MemoryManager::Free(Allocation allocation)
{
	uint32_t sectors = (allocation.Size - 1) / _alignment + 1;

	for (auto& page : _pages) {
		if (page.memory == allocation.Memory) {
			uint32_t startSector =
				allocation.Offset / _alignment;

			page.data->Free(startSector, sectors);
			return;
		}
	}

	throw std::runtime_error("Tried to free invalid memory block.");
}

IntervalStorage::IntervalStorage(size_t minLimit, size_t maxLimit)
{
	_minLimit = minLimit;
	_maxLimit = maxLimit;

	_allocated = 0;

	Interval* interval = new Interval;
	interval->Begin = _minLimit;
	interval->End = _maxLimit;

	AddInterval(interval);
}

IntervalStorage::~IntervalStorage()
{
	for (Interval* interval : _intervals) {
		delete interval;
	}
}

void IntervalStorage::AddInterval(Interval* interval)
{
	size_t begin = interval->Begin;
	size_t end = interval->End;

	interval->BeginIterator =
		_intervalBegin.insert({begin, interval}).first;
	interval->EndIterator = _intervalEnd.insert({end, interval}).first;
	interval->LengthIterator = _intervalLength.insert(
		{end - begin + 1, interval});

	_intervals.insert(interval);
}

void IntervalStorage::RemoveInterval(Interval* interval)
{
	_intervals.erase(interval);
	_intervalBegin.erase(interval->BeginIterator);
	_intervalEnd.erase(interval->EndIterator);
	_intervalLength.erase(interval->LengthIterator);
}

bool IntervalStorage::Find(size_t length, size_t& begin)
{
	std::multimap<size_t, Interval*>::iterator intervalIt =
		_intervalLength.lower_bound(length);

	if (intervalIt == _intervalLength.end()) {
		return false;
	}

	begin = intervalIt->second->Begin;
	return true;
}

void IntervalStorage::Allocate(size_t begin, size_t length)
{
	_allocated += length;

	Interval* oldInterval = _intervalBegin[begin];
	RemoveInterval(oldInterval);

	size_t end = begin + length - 1;

	if (end == oldInterval->End) {
		delete oldInterval;
		return;
	}

	Interval* newInterval = new Interval;
	newInterval->Begin = end + 1;
	newInterval->End = oldInterval->End;

	delete oldInterval;

	AddInterval(newInterval);
}

void IntervalStorage::Free(size_t begin, size_t length)
{
	_allocated -= length;

	size_t end = begin + length - 1;

	bool hasPrevInterval =
		begin > _minLimit &&
		_intervalEnd.find(begin - 1) != _intervalEnd.end();

	if (hasPrevInterval) {
		Interval* interval = _intervalEnd[begin - 1];
		RemoveInterval(interval);
		begin = interval->Begin;
		delete interval;
	}

	bool hasNextInterval =
		end < _maxLimit &&
		_intervalBegin.find(end + 1) != _intervalBegin.end();

	if (hasNextInterval) {
		Interval* interval = _intervalBegin[end + 1];
		RemoveInterval(interval);
		end = interval->End;
		delete interval;
	}

	Interval* newInterval = new Interval;
	newInterval->Begin = begin;
	newInterval->End = end;
	AddInterval(newInterval);
}
