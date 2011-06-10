/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */



#include "gfal_srm_mock_test.h"




char** list_dir_srm= { "testdir001", "testdir002", "testdir003", "testdir004", NULL };


extern const char* srm_valid_dir = "srm://mockendpoint.fr/grid/dteam/testdirvalid001";

extern const char* srm_noent_dir = "srm://mockendpoint.fr/grid/dteam/testdirinvalid001";

extern const char* srm_noaccess_dir = "srm://mockendpoint.fr/grid/dteam/testdirinvalid0012";


static void srm_context_mock_implem(struct srm_context *context,char *srm_endpoint,char *errbuf,int errbufsz,int verbose){
	return;
}

void mock_srm_context(){
	gfal_srm_external_call.srm_context_init = &srm_context_mock_implem;
}

void unmock_srm_context(){
	gfal_srm_external_call.srm_context_init= &srm_context_init;	
}


int srm_ls_mock_implem(struct srm_context *context,
		struct srm_ls_input *input,struct srm_ls_output *output){
			
}

void mock_srm_ls(){
	gfal_srm_external_call.srm_ls = &srm_ls_mock_implem;	
}

void unmock_srm_ls(){
	gfal_srm_external_call.srm_ls = &srm_ls;		
}




void mock_all_srm(){
	mock_srm_context();
	mock_srm_ls();
}


void unmock_all_srm(){
	unmock_srm_context();
	unmock_srm_ls();
}

