#pragma once
/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */



extern const char* surl_valid_endpoint ;





void mock_all_mds();



void mock_endpointres_mock();


void unmock_endpointres_mock();

char* valid_endpoint_value();
