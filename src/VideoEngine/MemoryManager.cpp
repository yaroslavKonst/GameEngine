#include "MemoryManager.h"

#include <stdexcept>

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

	AddPage();
}

MemoryManager::~MemoryManager()
{
	for (auto& page : _pages) {
		vkFreeMemory(_device, page.memory, nullptr);
	}
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
}

MemoryManager::Allocation MemoryManager::Allocate(uint32_t size)
{
	if (size > _pageSize)
	{
		throw std::runtime_error(
			"Memory allocation is greater than page size.");
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

	return allocation;
}

void MemoryManager::Free(Allocation allocation)
{
	uint32_t sectors = (allocation.Size - 1) / _alignment + 1;

	for (auto& page : _pages) {
		if (page.memory == allocation.Memory) {
			uint32_t startSector =
				allocation.Offset / _alignment;

			for (uint32_t idx = 0; idx < sectors; ++idx) {
				page.data[idx + startSector] = false;
			}

			return;
		}
	}

	throw std::runtime_error("Tried to free invalid memory block.");
}
