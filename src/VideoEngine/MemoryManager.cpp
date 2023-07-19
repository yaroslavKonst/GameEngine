#include "MemoryManager.h"

#include <stdexcept>

#include "../Logger/logger.h"

MemoryManager::MemoryManager(
	VkDevice device,
	uint32_t pageSize,
	uint32_t memoryTypeIndex,
	uint32_t alignment)
{
	_device = device;
	_pageSize = (pageSize / alignment + 1) * alignment;
	_alignment = alignment;
	_sectorCount = _pageSize / _alignment;
	_memoryTypeIndex = memoryTypeIndex;

	AddPage();

	Logger::Verbose(
		std::string("Created memory manager for index ") +
		std::to_string(_memoryTypeIndex) + ", alignment " +
		std::to_string(_alignment));

	Logger::Verbose(
		std::string("Page size ") + std::to_string(_pageSize));
}

MemoryManager::~MemoryManager()
{
	uint32_t leakedSectors = 0;

	for (auto& page : _pages) {
		vkFreeMemory(_device, page.memory, nullptr);

		for (bool sector : page.data) {
			if (sector) {
				++leakedSectors;
			}
		}
	}

	Logger::Verbose(
		std::string("Destroyed memory manager for index ") +
		std::to_string(_memoryTypeIndex) + ", alignment " +
		std::to_string(_alignment) + ". Leaks: " +
		std::to_string(leakedSectors));
}

void MemoryManager::AddPage()
{
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = _pageSize;
	allocInfo.memoryTypeIndex = _memoryTypeIndex;

	PageDescriptor page;
	page.data = std::vector<bool>(_sectorCount, false);

	VkResult res = vkAllocateMemory(
		_device,
		&allocInfo,
		nullptr,
		&page.memory);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate device memory.");
	}

	_pages.push_back(page);

	Logger::Verbose(
		std::string("New page for memory manager with index ") +
		std::to_string(_memoryTypeIndex) + ", alignment " +
		std::to_string(_alignment));
}

MemoryManager::Allocation MemoryManager::Allocate(uint32_t size)
{
	_mutex.lock();

	if (size > _pageSize)
	{
		_mutex.unlock();
		throw std::runtime_error(
			std::string("Memory allocation size ") +
			std::to_string(size) + " is greater than page size " +
			std::to_string(_pageSize));
	}

	uint32_t requiredSectors = (size - 1) / _alignment + 1;

	for (auto& page : _pages) {
		uint32_t sectorIndex = 0;
		uint32_t sectorSpan = 0;

		while (sectorIndex < _sectorCount) {
			if (!page.data[sectorIndex]) {
				++sectorSpan;
			} else {
				sectorSpan = 0;
			}

			++sectorIndex;

			if (sectorSpan == requiredSectors) {
				while (sectorSpan > 0) {
					--sectorSpan;
					--sectorIndex;

					page.data[sectorIndex] = true;
				}

				Allocation allocation;
				allocation.Memory = page.memory;
				allocation.Offset = _alignment * sectorIndex;
				allocation.Size = size;

				_mutex.unlock();
				return allocation;
			}
		}
	}

	AddPage();

	for (uint32_t sector = 0; sector < requiredSectors; ++sector) {
		_pages.back().data[sector] = true;
	}

	Allocation allocation;
	allocation.Memory = _pages.back().memory;
	allocation.Offset = 0;
	allocation.Size = size;

	_mutex.unlock();
	return allocation;
}

void MemoryManager::Free(Allocation allocation)
{
	_mutex.lock();
	uint32_t sectors = (allocation.Size - 1) / _alignment + 1;

	for (auto& page : _pages) {
		if (page.memory == allocation.Memory) {
			uint32_t startSector =
				allocation.Offset / _alignment;

			for (uint32_t idx = 0; idx < sectors; ++idx) {
				page.data[idx + startSector] = false;
			}

			_mutex.unlock();
			return;
		}
	}

	_mutex.unlock();
	throw std::runtime_error("Tried to free invalid memory block.");
}
