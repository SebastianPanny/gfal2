#pragma once
/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */



extern char** define_se_endpoints;
extern char** define_se_types;


int mds_mock_sd_get_se_types_and_endpoints(const char *host, char ***se_types, char ***se_endpoints,char *errbuf, int errbufsz);
