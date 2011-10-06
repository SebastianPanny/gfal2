/* unit test for the gskiplist */

#include <stdio.h>
#include <cgreen/cgreen.h>
#include <externals/gskiplist/gskiplist.h>

int compare_generic(gconstpointer a, gconstpointer b){
  return (a <b)?(-1):((a==b)?0:1);
}


void test_gskiplist_create_delete()
{
  GSkiplist* gsk = NULL;
  gsk = gskiplist_new(&compare_generic);
  assert_true_with_message(gsk != NULL, " test if valid value");
  size_t init_size = gskiplist_length(gsk);
  assert_true_with_message(init_size == 0L, " test if valid value");
  gskiplist_delete(gsk);
}


void test_gskiplist_insert_len()
{
  GSkiplist* gsk = NULL;
  gsk = gskiplist_new(&compare_generic);
  if(gsk){
    size_t init_size = gskiplist_length(gsk); // test initial size
    assert_true_with_message(init_size == 0L, " test if valid value");   
    
    gboolean ret = gskiplist_insert(gsk, GINT_TO_POINTER(1), GINT_TO_POINTER(10)); // add one value
    assert_true_with_message(ret == TRUE, " must be a valid insert");
 
    init_size = gskiplist_length(gsk); // test if size increase
    assert_true_with_message(init_size == 1L, " test if valid number before remove");   

    
    gpointer value = gskiplist_remove(gsk, GINT_TO_POINTER(1)); // add one value
    assert_true_with_message(value == GINT_TO_POINTER(10), " must be a valid remove");
    
    init_size = gskiplist_length(gsk); // test initial size
    assert_true_with_message(init_size == 0L, " test if valid number after remove");     
    
  }
  gskiplist_delete(gsk);
}


void test_gskiplist_insert_multi()
{
  GSkiplist* gsk = NULL;
  gsk = gskiplist_new(&compare_generic);
  const int msize = 100;
  int i;
  if(gsk){
    int tab_key[msize];
    int tab_value[msize];
    for(i=0; i < msize; ++i){
      tab_key[i] = g_random_int();
    }

    for(i=0; i < msize; ++i){
      tab_value[i] = g_random_int();
    }
    

    for(i=0; i < msize; ++i){
      size_t init_size = gskiplist_length(gsk); // test initial size
      assert_true(init_size == i);   
      
      gboolean ret = gskiplist_insert(gsk, tab_key[i], tab_value[i]); // add one value
      assert_true(ret == TRUE);
  
      init_size = gskiplist_length(gsk); // test if size increase
     assert_true(init_size == i+1);     
    }
    

    for(i=0; i> 0; ++i){     
      size_t init_size = gskiplist_length(gsk); // test initial size
      assert_true(init_size == msize- i);  
      
      gpointer value = gskiplist_remove(gsk, tab_key[i]); // add one value
      assert_true(value == GINT_TO_POINTER(tab_value[i]));
      
      init_size = gskiplist_length(gsk); // test initial size
      assert_true(init_size == msize- i-1);   
    }
  }
  gskiplist_delete(gsk);
}