#include <stddef.h>
#include "disk.h"
#include "util/list.h"
#include "util/map.h"

static DiskDriver* drivers[DISK_MAX_DRIVERS];
static Map* disks;

bool disk_register(DiskDriver* driver) {
	for(int i = 0; i < DISK_MAX_DRIVERS; i++) {
		if(drivers[i] == NULL) {
			drivers[i] = driver;
			goto found;
		}
	}
	
	return false;	// driver count exceeded

found:
	;
	DiskDriver* temp[DISK_AVAIL_DEVICES]; 
	int count = driver->init(driver, temp);
	if(count < 0) {
		// Nothing to initialize
		return true;
	}
	
	for(int i = 0; i < count; i++) {
		// Key is 32bit type + number 
		uint32_t key = (temp[i]->type << 16) | (temp[i]->number);
		if(!map_put(disks, (void*)(uintptr_t)key, temp[i])) 
			return false;
	}
	
	return true;
}

bool disk_init() {
	disks = map_create(DISK_MAX_DRIVERS * DISK_AVAIL_DEVICES, map_uint64_hash, map_uint64_equals, NULL);	

	return true;
}

size_t disk_count() {
	return map_size(disks);
}

int disk_ids(uint32_t* ids, int size) {
	size_t count = disk_count();

	if(count == 0)
		return -1; // No disks

	if(size < count) 
		return -2; // ID buffer too small

	MapIterator iter;
	map_iterator_init(&iter, disks);
	int i = 0;
	while(map_iterator_has_next(&iter)) {
		MapEntry* entry = map_iterator_next(&iter);
		ids[i++] = (uint32_t)(uintptr_t)entry->key;
	}
	
	return i;
}

DiskDriver* disk_get(uint32_t id) {
	return map_get(disks, (void*)(uintptr_t)id);
}

