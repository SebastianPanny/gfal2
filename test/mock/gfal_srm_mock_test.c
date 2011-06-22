/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */



#include "gfal_srm_mock_test.h"
#include <string.h>
#include <cgreen/cgreen.h>
#include <errno.h>
#include <stdlib.h>


struct srm_ls_output defined_srm_ls_output;
struct srm_rmdir_output defined_srm_rmdir_output;
struct srm_getpermission_output defined_srm_getpermission_output;
struct srmv2_filestatus* defined_srmv2_filestatus=NULL;


void srm_mock_srm_context_init(struct srm_context *context,char *srm_endpoint,char *errbuf,int errbufsz,int verbose){
	mock(context, srm_endpoint, errbuf, errbufsz, verbose);
}

int srm_mock_srm_ls(struct srm_context *context,
	struct srm_ls_input *input,struct srm_ls_output *output){
	int a = mock(context, input, output);
	if(a){
		errno = a;
		return -1;
	}
	memcpy(output, &defined_srm_ls_output, sizeof(struct srm_ls_output));
	return 0;
}
	
int srm_mock_srm_rmdir(struct srm_context *context,
	struct srm_rmdir_input *input,struct srm_rmdir_output *output){
	int a = mock(context, input, output);
	if(a){
		errno = a;
		return -1;
	}
	memcpy(output, &defined_srm_rmdir_output, sizeof(struct srm_rmdir_output));
	return 0;		
}
	
int srm_mock_srm_mkdir(struct srm_context *context,
	struct srm_mkdir_input *input){
	int a = mock(context, input);
	if(a){
		errno = a;
		return -1;
	}
	return 0;		
}
	
int srm_mock_srm_getpermission (struct srm_context *context,
	struct srm_getpermission_input *input,struct srm_getpermission_output *output){
	int a = mock(context, input, output);
	if(a){
		errno = a;
		return -1;
	}
	memcpy(output, &defined_srm_getpermission_output, sizeof(struct srm_getpermission_output));
	return 0;			
}

int srm_mock_srm_check_permission(struct srm_context *context,
	struct srm_checkpermission_input *input,struct srmv2_filestatus **statuses){
	int a = mock(context, input, statuses);
	if(a){
		errno = a;
		return -1;
	}
	*statuses = defined_srmv2_filestatus; 
	return 0;				
}	
	
void srm_mock_srm_srmv2_pinfilestatus_delete(struct srmv2_pinfilestatus*  srmv2_pinstatuses, int n){
	mock(srmv2_pinstatuses, n);	
}

void srm_mock_srm_srmv2_mdfilestatus_delete(struct srmv2_mdfilestatus* mdfilestatus, int n){
	mock(mdfilestatus, n);
}

void srm_mock_srm_srmv2_filestatus_delete(struct srmv2_filestatus*  srmv2_statuses, int n){
	mock(srmv2_statuses, n);
}

void srm_mock_srm_srm2__TReturnStatus_delete(struct srm2__TReturnStatus* status){
	mock(status);
}
