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



void mock_srm_context(){
	gfal_srm_external_call.srm_context_init = &mock_srm_context;
}

void unmock_srm_context(){
	gfal_srm_external_call.srm_context_init= &srm_context_init;	
}



