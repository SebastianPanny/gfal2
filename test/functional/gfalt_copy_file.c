/**
 * Compile command : gcc -o gfalt_copyfile gfalt_copyfile.c `pkg-config --libs --cflags gfal_transfer`
 */

#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <signal.h>
#include <transfer/gfal_transfer.h>

gfal2_context_t handle=NULL;


void event_callback(const gfalt_event_t e, gpointer user_data)
{
  static const char* side_str[] = {"SRC",
                                   "DST",
                                   "BTH"};

  printf("[%ld:%ld] %s %s\t%s\t%s\n",
         e->timestamp / 1000, e->timestamp % 1000,
         side_str[e->side],
         g_quark_to_string(e->domain), g_quark_to_string(e->stage),
         e->description);
}



// setup interrupt
void sigint_cancel(int param)
{
  printf("User pressed Ctrl+C\n");
  if(handle)
    gfal2_cancel(handle);
}


int main(int argc, char** argv){
    signal(SIGINT, &sigint_cancel);
	if( argc <3 ){
		printf(" Usage %s [src_url] [dst_url] \n",argv[0]);
		return 1;
	}
	GError * tmp_err = NULL; // classical GError/glib error management

	
	// initialize gfal
    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_TRACE_PLUGIN);
	 if( (handle = gfal2_context_new(&tmp_err)) == NULL ) {
		 printf(" bad initialization %d : %s.\n", tmp_err->code,tmp_err->message);
		 return -1;
	 } 
	 
	// Params
	gfalt_params_t params = gfalt_params_handle_new(NULL);
	gfalt_set_replace_existing_file(params, TRUE, NULL);
	gfalt_set_create_parent_dir(params, TRUE, NULL);

	// Register callback
	gfalt_set_event_callback(params, event_callback, &tmp_err);

	 // begin copy
    if(  gfalt_copy_file(handle, params, argv[1], argv[2], &tmp_err)   != 0){
         printf(" error while the file transfer %d : %s.\n", tmp_err->code,tmp_err->message);
         gfal2_context_free(handle);
		 return -1;		
	}else
		printf(" transfer sucessfull ! \n");
		
	gfal2_context_free(handle);
    return 0;
}

