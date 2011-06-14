#pragma once
/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */
 
#include "gfal_mds_mock_test.h" 
#include "srm/gfal_common_srm_internal_layer.h"



char** list_dir_srm();

int count_dir_srm();

char* srm_valid_dir();

char* srm_noent_dir();

char* srm_noaccess_dir();


void mock_all_srm();
void unmock_all_srm();

void mock_srm_context();


void unmock_srm_context();

void mock_srm_ls();

void unmock_srm_ls();
