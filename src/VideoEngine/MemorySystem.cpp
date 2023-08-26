#include "MemorySystem.h"

#include "../Logger/logger.h"

// 128 MB for page.
// 1 MB for domain page.
#define PAGE_SIZE 1048576 * 64 * 4
#define DOMAIN_PAGE_SIZE 1048576

MemorySystem::MemorySystem(VkDevice device)
{
	_device = device;
}

MemorySystem::~MemorySystem()
{
	for (auto& manager : _managers) {
		delete manager.second;
	}

	for (auto& managers : _domains) {
		Logger::Verbose() << "Destroying domain " << managers.first;

		for (auto& manager : managers.second) {
			delete manager.second;
		}
	}
}

MemorySystem::Allocation MemorySystem::Allocate(
	uint32_t size,
	AllocationProperties properties,
	uint32_t domain)
{
	Domain* managers = &_managers;

	uint32_t pageSize = PAGE_SIZE;

	if (domain > 0) {
		if (_domains.find(domain) == _domains.end()) {
			_domains[domain] = Domain();
		}

		managers = &_domains[domain];
		pageSize = DOMAIN_PAGE_SIZE;
	}

	if (managers->find(properties) == managers->end()) {
		Logger::Verbose() <<
			"Requested alignment " << properties.Alignment;

		(*managers)[properties] = new MemoryManager(
			_device,
			pageSize,
			properties.MemoryTypeIndex,
			properties.Alignment,
			properties.Mapped);
	}

	Allocation allocation;

	MemoryManager::Allocation alloc =
		(*managers)[properties]->Allocate(size);

	allocation.Memory = alloc.Memory;
	allocation.Size = alloc.Size;
	allocation.Offset = alloc.Offset;
	allocation.Properties = properties;
	allocation.Mapping = alloc.Mapping;

	return allocation;
}

void MemorySystem::Free(Allocation allocation, uint32_t domain)
{
	Domain* managers = &_managers;

	if (domain > 0) {
		managers = &_domains[domain];
	}

	MemoryManager::Allocation alloc;
	alloc.Memory = allocation.Memory;
	alloc.Size = allocation.Size;
	alloc.Offset = allocation.Offset;

	(*managers)[allocation.Properties]->Free(alloc);
}
