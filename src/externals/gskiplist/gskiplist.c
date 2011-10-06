/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#include <time.h>
#include "gskiplist.h"
// proto
gpointer gskiplist_get_first_value_internal(GSkiplist* sk);
gpointer gskiplist_remove_internal(GSkiplist* sk, gpointer key);
gboolean gskiplist_insert_internal(GSkiplist* sk, gpointer key, gpointer value);
gpointer gskiplist_search_internal(GSkiplist* sk, gpointer key);
GSkipNode* gskiplist_create_node(GSkiplist* sk, gpointer key, gpointer data, guint size);
int gskiplist_compare(GSkiplist* sk, GSkipNode* a, gpointer  b);
int gskiplist_compare_node(GSkiplist* sk, GSkipNode* a, GSkipNode*  b);
guint gskiplist_get_random_size(GSkiplist * sk);


const int max_size=32;

/**
 * create a new skip list
 * */
GSkiplist* gskiplist_new(GCompareFunc func){
  GSkiplist* skip = g_new0(GSkiplist, 1);
  skip->cmp_func = func;
  g_static_rw_lock_init(&skip->lock);
  skip->head_node = gskiplist_create_node(skip, NULL, NULL, 32);
  return skip;
}


/**
 * delete an existing skip list
 */
void gskiplist_delete(GSkiplist* sk){
  if(sk){
    g_static_rw_lock_writer_lock (&sk->lock);
    g_static_rw_lock_writer_unlock(&sk->lock);   
    g_free(sk);
  }
}

/**
 * Insert an element in the SkipList sk 
 * replace an existing one if already exist
 * @return return true if insert else False if replaced
 */
gboolean gskiplist_insert(GSkiplist* sk, gpointer key, gpointer value)
{
  g_assert(sk != NULL);
  g_static_rw_lock_writer_lock(&sk->lock);
  gboolean res = gskiplist_insert_internal(sk, key, value);
  g_static_rw_lock_writer_unlock(&sk->lock);
  return res;  
}

size_t gskiplist_length(GSkiplist* sk){
    g_assert(sk != NULL);
    return sk->length;
}

gboolean gskiplist_insert_internal(GSkiplist* sk, gpointer key, gpointer value){
  g_assert(sk != NULL);
  GSkipNode* new_node = gskiplist_create_node(sk, key, value, gskiplist_get_random_size(sk));
  GSkipNode* node, * prec_node;
  node = prec_node = sk->head_node;
  int cmp_value;
  for(int i = max_size-1; i >= 0; --i){

    while(node != NULL){ // walk in a level list to search a given value
      cmp_value = gskiplist_compare_node(sk, new_node, node);
      if( cmp_value < 0){ // inferior to the next value -> insert
	if( i < new_node->height){
	  new_node->link[i].next = node;
	  prec_node->link[i].next = new_node;
	}
	break;
      }
      if( cmp_value == 0){ // equal to the next value to the next value -> insert and delete the old
	for(; i>= 0; --i){
	  if( i < new_node->height){
	    new_node->link[i].next = node->link[i].next;
	    prec_node->link[i].next = new_node;
	  }else{
	    prec_node->link[i].next = node->link[i].next;
	  }
	}

	g_free(node);
	return FALSE;
      }
      prec_node = node;
      node = node->link[i].next;
      
    }
    
    if(node == NULL) // end of the list, add queue
      prec_node->link[i].next= new_node;
    
  }
  sk->length++; 
  return TRUE;
  
}

/**
 *  remove a element from the list with the given key,
 *  return the value if found else NULL
 * */
gpointer gskiplist_remove(GSkiplist* sk, gpointer key){
  g_assert(sk != NULL);
  g_static_rw_lock_writer_lock(&sk->lock);
  GSkipNode* node = sk->head_node->link[0].next;
  while( node != NULL){
      GSkipNode* prev_node = node;
      node = node->link[0].next;
      g_free(prev_node);
  }
  gpointer res = gskiplist_remove_internal(sk, key);
  g_static_rw_lock_writer_unlock(&sk->lock);
  return res;   
}

gpointer gskiplist_remove_internal(GSkiplist* sk, gpointer key){
  GSkipNode* node, * prec_node;
  node = prec_node = sk->head_node; 
  int cmp_value;
  for(int i = max_size-1; i >= 0; --i){

    while(node  != NULL){ // walk in a level list to search a given value
      cmp_value = gskiplist_compare(sk, node, key);
      
      if( cmp_value == 0){ // equal to the next value to the next value -> insert and delete the old
	for(; i>= 0; --i){
	    prec_node->link[i].next = node->link[i].next;
	}
	gpointer resu = node->data;
	sk->length--;
	g_free(node);
	return resu;
      }
      prec_node = node;
      node = node->link[i].next;
      
    }
  }
    
  return NULL;
}


/**
 * Search an element and return the value if found else NULL pointer
 */
gpointer gskiplist_search(GSkiplist* sk, gpointer key)
{
  g_assert(sk != NULL);
  g_static_rw_lock_reader_lock(&sk->lock);
  gpointer res= gskiplist_search_internal(sk, key);
  g_static_rw_lock_reader_lock(&sk->lock);
  return res;
}

gpointer gskiplist_search_internal(GSkiplist* sk, gpointer key){
  GSkipNode* node;
  node = sk->head_node;
  int cmp_value;
  for(int i = max_size-1; i >= 0; --i){

    while(node  != NULL){ // walk in a level list to search a given value
      cmp_value = gskiplist_compare(sk, node, key);
      if( cmp_value == 0){ // equal to the next value to the next value -> insert and delete the old
	  return node->data;
      }
      node = node->link[i].next;
      
    }
  }
  return NULL;
  
}

/**
 * Return the first value of the list
 */
gpointer gskiplist_get_first_value(GSkiplist* sk)
{
  g_assert(sk != NULL);
  g_static_rw_lock_reader_lock(&sk->lock);
  gpointer res= gskiplist_get_first_value_internal(sk);
  g_static_rw_lock_reader_lock(&sk->lock);
  return res;
}

gpointer gskiplist_get_first_value_internal(GSkiplist* sk){
  GSkipNode* firstn = sk->head_node->link[0].next;
  return (firstn)?(firstn->data):NULL;
}



GSkipNode* gskiplist_create_node(GSkiplist* sk, gpointer key, gpointer data, guint size){
  const size_t memsize = sizeof(GSkipNode) + sizeof(GSkipLink)*size;
  GSkipNode* ret = g_malloc0(memsize);
  ret->data = data;
  ret->key = key;
  ret->height = size;  
  return ret;
}


int gskiplist_compare_node(GSkiplist* sk, GSkipNode* a, GSkipNode*  b){
  if( a == sk->head_node)
    return -1;
  if( b==  sk->head_node)
    return 1;
  return sk->cmp_func(a->key, b->key);
}


int gskiplist_compare(GSkiplist* sk, GSkipNode* a, gpointer  b){
  if( a == sk->head_node)
    return -1;
  return sk->cmp_func(a->key, b);
}



/**
 * return the height for a skip list point [1..]
 */
guint gskiplist_get_random_size(GSkiplist * sk){
  guint res_rand = g_random_int();
  int ret =1;
  while(res_rand & 0x01){ // improve ret until last bit of rand is 0
    ret++;
    res_rand >>=1;
  }
  return ret;
}
