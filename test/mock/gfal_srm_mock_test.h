#pragma once
/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */
 
 
#include "gfal_common_srm_internal_layer.h"



extern char** list_dir_srm;

extern const char* srm_valid_dir;

extern const char* srm_noent_dir;

extern const char* srm_noaccess_dir;


void mock_srm_context();


void unmock_srm_context();
