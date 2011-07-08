/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */


#include <cgreen/cgreen.h>
#include "gfal_mds_mock_test.h" 
#include "mds/gfal_common_mds_layer.h"

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

int mds_mock_sd_get_se_types_and_endpoints(const char *host, char ***se_types, char ***se_endpoints,char *errbuf, int errbufsz){
	int ret = mock(host, se_types, se_endpoints, errbuf, errbufsz);
	if(ret){
		errno = ret;
		return -1;
	}
	*se_types= g_strdupv (define_se_types);
	*se_endpoints = g_strdupv (define_se_endpoints);
	return 0;	
}



int mds_mock_sd_get_lfc_endpoint(char **lfc_endpoint, char *errbuf, int errbufsz){
	int ret = mock(lfc_endpoint, errbuf, errbufsz);
	if(ret){
		errno = ret;
		return -1;
	}
	*lfc_endpoint = define_lfc_endpoint;
	return 0;		
	
}
