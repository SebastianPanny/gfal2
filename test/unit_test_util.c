/*
 * 
 * Simple tools for unit tests
 * 
 * */


#include <glib.h>
#include <stdio.h> 
 
gboolean check_GList_Result_String(GList* list, char** example){	// return true if two string list are the same
	GList *tmp_list=list;
	while(list != NULL){
		if(strcmp(tmp_list->data, *example) != 0)
			g_printerr(" error, the two string are different : %s, %s ", tmp_list->data, *example);
			return FALSE;
		tmp_list= g_list_next(list);
		example++;
	}
	return TRUE;
	
} 
