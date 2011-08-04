/**
 * @brief Simple and flexible cache system with timeout
 * @author Adev
 * @date 27/07/11
 * 
 * */

#define _GNU_SOURCE

#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "gcachemain.h"

static const guint64 max_list_len = MAX_LIST_LEN;

__thread gboolean skip_deletion = FALSE;

typedef struct _Internal_item{
	GDestroyNotify value_destroyer;
	void *item;
} Internal_item;

struct _GSimpleCache_Handle{
	GHashTable*  table;
	pthread_mutex_t mux;
};

static void gsimplecache_destroy_item_internal(gpointer a){
	Internal_item* i = (Internal_item*) a;
	if(skip_deletion == FALSE)
		i->value_destroyer(i->item);
	g_free(i);
}


static gboolean hash_strings_are_equals(gconstpointer a, gconstpointer b){
	return (strcmp((char*) a, (char*) b)== 0);
}

/**
 * Construct a new cache with a capacity of max_size bytes
 * */
GSimpleCache* gsimplecache_new(guint64 max_size){
	GSimpleCache* ret = (GSimpleCache*) g_new(struct _GSimpleCache_Handle,1);
	ret->table = g_hash_table_new_full(&g_str_hash, &hash_strings_are_equals, &free, &gsimplecache_destroy_item_internal );
	pthread_mutex_init(&ret->mux,NULL);
	return ret;
}

/**
 *  delete a cache object, all internals object are free
 * */
void gsimplecache_delete(GSimpleCache* cache){
	if(cache != NULL){
		pthread_mutex_lock(&cache->mux);
		g_hash_table_remove_all(cache->table);
		pthread_mutex_unlock(&cache->mux);
		pthread_mutex_destroy(&cache->mux);
		g_free(cache);
	}
}

inline void* gsimplecache_find_kstr_internal(GSimpleCache* cache, const char* key){
	Internal_item* ret = (Internal_item*) g_hash_table_lookup(cache->table, (gconstpointer) key);
	if(ret != NULL ){
		return ret->item;
	}	
	return NULL;
}

inline gboolean gsimplecache_remove_internal_kstr(GSimpleCache* cache, const char* key){
		return g_hash_table_remove(cache->table, (gconstpointer) key);	
}




inline void gsimplecache_add_item_internal(GSimpleCache* cache, char* dup_key, Internal_item* item){
	g_hash_table_insert(cache->table, dup_key, item);
}


/**
 * Add an item to the cache referenced by the Key key while a duration time (in sec )
 * After this duration, the item will be deleted by the value_destroyer function
 * @warning, the key is duplicated and free internally
 * */
void gsimplecache_add_item_kstr(GSimpleCache* cache, const char* key, void* item, GDestroyNotify value_destroyer){
	pthread_mutex_lock(&cache->mux);	
	Internal_item* it = g_new(struct _Internal_item,1);
	it->value_destroyer = value_destroyer;
	it->item = item;
	gsimplecache_add_item_internal(cache, strdup(key), it);
	pthread_mutex_unlock(&cache->mux);
}



/**
 * remove the item in the cache, return TRUE if removed else FALSE
 * destroy the internal item automatically
 * */
gboolean gsimplecache_remove_kstr(GSimpleCache* cache, const char* key){
	pthread_mutex_lock(&cache->mux);
	gboolean ret = gsimplecache_remove_internal_kstr(cache, key);
	pthread_mutex_unlock(&cache->mux);	
	return ret;
}

/**
 * find the item in the cache, return it if exist , else return NULL
 * the returned value must be free by hand
 * 
 * */
void* gsimplecache_take_kstr(GSimpleCache* cache, const char* key){
	pthread_mutex_lock(&cache->mux);	
	void* ret = gsimplecache_find_kstr_internal(cache, key);
	if(ret){
		skip_deletion = TRUE;
		gsimplecache_remove_internal_kstr(cache, key);	
		skip_deletion = FALSE;
	}
	pthread_mutex_unlock(&cache->mux);	
	return ret;
}


/**
 * search the item in the cache, return it if exist and is not outdated, else return NULL
 * 
 * */
void* gsimplecache_find_kstr(GSimpleCache* cache, const char* key){
	pthread_mutex_lock(&cache->mux);
	void* ret = gsimplecache_find_kstr_internal(cache, key);
	pthread_mutex_unlock(&cache->mux);	
	return ret;
}
