/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */


#include <cgreen/cgreen.h>
#include "gfal_mds_mock_test.h" 

char** define_se_endpoints;
char** define_se_types;
char* define_lfc_endpoint;

void define_mock_endpoints(char* endpoint){
	int i1;
	define_se_endpoints = calloc(sizeof(char*), 4);
	for(i1=0;i1 <3; ++i1)
		define_se_endpoints[i1]= strdup(endpoint);
	define_se_types= calloc(sizeof(char*), 4);
	char* types[] = { "srm_v1", "srm_v2", "srm_v1"};
	for(i1=0;i1 <3; ++i1)
		define_se_types[i1]= strdup(types[i1]);	
	
}


