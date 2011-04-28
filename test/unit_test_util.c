/*
 * 
 * Simple tools for unit tests
 * 
 * */


#include <glib.h>
#include <stdio.h> 
 
gboolean check_GList_Result_String(GList* list, char** example){	// return true if two string list are the same
	GList *tmp_list=list;
	while(tmp_list != NULL){
		if( tmp_list->data != NULL &&  *example != NULL){
			if(strcmp(tmp_list->data, *example) != 0){
				g_printerr(" error, the two string are different : %s, %s \n", tmp_list->data, *example);
				return FALSE;
			} 
		} else {
			if(tmp_list->data != *example){
					g_printerr(" one string is NULL but not the other : %s, %s \n", tmp_list->data, *example);
					return FALSE;
			}		
		}
		tmp_list= g_list_next(tmp_list);
		example++;
	}
	return TRUE;
	
} 



gboolean check_GList_Result_int(GList* list, int* example){	// return true if two string list are the same
	GList *tmp_list=list;
	while(tmp_list != NULL){
		if( GPOINTER_TO_INT(tmp_list->data) != *example){
			g_printerr(" error, the two integer are different : %d, %d \n", GPOINTER_TO_INT(tmp_list->data), *example);
			return FALSE;
		}
		tmp_list= g_list_next(tmp_list);
		example++;
	}
	return TRUE;
	
} 
