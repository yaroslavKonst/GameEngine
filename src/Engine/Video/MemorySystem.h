#ifndef _MEMORY_SYSTEM_H
#define _MEMORY_SYSTEM_H

#include <map>

#include "MemoryManager.h"
#include "PhysicalDeviceSupport.h"
#include "../Sync/mutex.h"

class MemorySystem
{
public:
	struct AllocationProperties
	{
		uint32_t Alignment;
		uint32_t MemoryTypeIndex;
		bool Mapped;

		bool operator<(const AllocationProperties& properties) const
		{
			if (Alignment != properties.Alignment) {
				return Alignment < properties.Alignment;
			}

			if (MemoryTypeIndex != properties.MemoryTypeIndex) {
				return
					MemoryTypeIndex <
					properties.MemoryTypeIndex;
			}

			return Mapped < properties.Mapped;
		}
	};

	struct Allocation
	{
		VkDeviceMemory Memory;
		uint32_t Size;
		uint32_t Offset;
		void* Mapping;
		AllocationProperties Properties;

	};

	MemorySystem(VkDevice device);
	~MemorySystem();

	Allocation Allocate(
		uint32_t size,
		AllocationProperties properties,
		uint32_t domain = 0);
	void Free(Allocation allocation, uint32_t domain = 0);

private:
	typedef std::map<AllocationProperties, MemoryManager*> Domain;
	VkDevice _device;

	std::map<uint32_t, Domain> _domains;
	Domain _managers;

	Sync::Mutex _mutex;

	size_t _maxAllocatedMemory;
	size_t _totalAllocatedMemory;
};

#endif
