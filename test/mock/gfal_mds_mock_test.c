/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */



#include "gfal_mds_mock_test.h" 
#include "mds/gfal_common_mds_layer.h"

/*
const char* surl_valid_endpoint = "srm://testendpointvalid.fr";


const char* surl_invalid_endpoint = "srm://testendpointinvalid.fr";

char* valid_endpoint_value(){
	return "testendpointvalid.fr";
}


void mock_all_mds(){
	mock_endpointres_mock();
	
}

void unmock_all_mds(){
	unmock_endpointres_mock()	;
}


int mock_sd_get_se_types_and_endpoints(const char *host, char ***se_types, char ***se_endpoints,char *errbuf, int errbufsz){
	int ret = -1;
	if( strcmp(host, valid_endpoint_value()) == 0){
		*se_types = calloc(sizeof(char*), 2);
		**se_types = strdup("srm_v2");
		*se_endpoints= calloc(sizeof(char*), 2);
		**se_endpoints = strdup( valid_endpoint_value());
		ret = 0;		
	}
	//g_printerr("host %s ", host);
	return ret;
}

void mock_endpointres_mock(){
	gfal_mds_external_call.sd_get_se_types_and_endpoints = &mock_sd_get_se_types_and_endpoints;
}


void unmock_endpointres_mock(){
	gfal_mds_external_call.sd_get_se_types_and_endpoints = &sd_get_se_types_and_endpoints;
}

*/

