#pragma once
/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */



extern char** define_se_endpoints;
extern char** define_se_types;
extern char* define_lfc_endpoint;

// convenience functions

void define_mock_endpoints(char* endpoint);


// mocks

int mds_mock_sd_get_se_types_and_endpoints(const char *host, char ***se_types, char ***se_endpoints,char *errbuf, int errbufsz);


int mds_mock_sd_get_lfc_endpoint(char **lfc_endpoint, char *errbuf, int errbufsz);
