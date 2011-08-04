#pragma once
/**
 * @brief header for GCache
 * @author Adev
 * @date 27/07/11
 * 
 * */

#include <glib.h>


#define MAX_LIST_LEN 20000

typedef struct _GSimpleCache_Handle GSimpleCache;

GSimpleCache* gsimplecache_new(guint64 max_size);

void gsimplecache_delete(GSimpleCache* cache);

int gcache_add_item_kstr(const char* key, void* item, GDestroyNotify value_destroyer, long max_duration);

void* gsimplecache_find_kstr(GSimpleCache* cache, const char* key);

void* gsimplecache_take_kstr(GSimpleCache* cache, const char* key);

gboolean gsimplecache_remove_kstr(GSimpleCache* cache, const char* key);

void gsimplecache_add_item_kstr(GSimpleCache* cache, const char* key, void* item, GDestroyNotify value_destroyer);
