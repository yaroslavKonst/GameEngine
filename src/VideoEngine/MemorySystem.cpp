#include "MemorySystem.h"

#include "../Logger/logger.h"

// 16 MB for page.
#define PAGE_SIZE 1048576 * 64

MemorySystem::MemorySystem(VkDevice device)
{
	_device = device;
}

MemorySystem::~MemorySystem()
{
	for (auto& manager : _managers) {
		delete manager.second;
	}
}

MemorySystem::Allocation MemorySystem::Allocate(
	uint32_t size,
	AllocationProperties properties)
{
	if (_managers.find(properties) == _managers.end()) {
		Logger::Verbose() <<
			"Requested alignment " << properties.Alignment;

		_managers[properties] = new MemoryManager(
			_device,
			PAGE_SIZE,
			properties.MemoryTypeIndex,
			properties.Alignment);
	}

	Allocation allocation;

	MemoryManager::Allocation alloc =
		_managers[properties]->Allocate(size);

	allocation.Memory = alloc.Memory;
	allocation.Size = alloc.Size;
	allocation.Offset = alloc.Offset;
	allocation.Properties = properties;

	return allocation;
}

void MemorySystem::Free(Allocation allocation)
{
	MemoryManager::Allocation alloc;
	alloc.Memory = allocation.Memory;
	alloc.Size = allocation.Size;
	alloc.Offset = allocation.Offset;

	_managers[allocation.Properties]->Free(alloc);
}
