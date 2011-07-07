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
struct srmv2_pinfilestatus * defined_get_output=NULL;
struct srmv2_filestatus* defined_put_done=NULL;

void define_mock_stat_file_valid(char* surl, mode_t mode, int uid, int gid){
	defined_srm_ls_output.statuses= g_new0(struct srmv2_mdfilestatus,1);
	defined_srm_ls_output.statuses->surl = strdup(surl);
	memset(&defined_srm_ls_output.statuses->stat, 0, sizeof(struct stat));
	defined_srm_ls_output.statuses->stat.st_mode = mode;
	defined_srm_ls_output.statuses->stat.st_uid = uid;
	defined_srm_ls_output.statuses->stat.st_gid= gid;
	
}

void define_mock_stat_file_error(char* surl, int status, char* err){
	defined_srm_ls_output.statuses= g_new0(struct srmv2_mdfilestatus,1);
	defined_srm_ls_output.statuses->surl = strdup(surl);
	memset(&defined_srm_ls_output.statuses->stat, 0, sizeof(struct stat));
	defined_srm_ls_output.statuses->status = status;
	defined_srm_ls_output.statuses->explanation = strdup(err);
}

void define_mock_srmv2_filestatus(int number, char** surl, char** explanation, char** turl, int* status){
	int i;
	defined_srmv2_filestatus= calloc(sizeof(struct srmv2_filestatus), number);
	for(i=0; i < number; ++i){
		if(surl)
			defined_srmv2_filestatus[i].surl = strdup(surl[i]);
		if(explanation)
			defined_srmv2_filestatus[i].explanation = strdup(explanation[i]);
		if(turl)
			defined_srmv2_filestatus[i].turl = strdup(turl[i]);
		if(status)
			defined_srmv2_filestatus[i].status = status[i];
	}
}


void define_mock_srmv2_pinfilestatus(int number, char** surl, char** explanation, char** turl, int* status){
	int i;
	defined_get_output= calloc(sizeof(struct srmv2_pinfilestatus), number);
	for(i=0; i < number; ++i){
		if(surl)
			defined_get_output[i].surl = strdup(surl[i]);
		if(explanation)
			defined_get_output[i].explanation = strdup(explanation[i]);
		if(turl)
			defined_get_output[i].turl = strdup(turl[i]);
		if(status)
			defined_get_output[i].status = status[i];
	}
}

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
	if(a<0){
		errno = a;
		return -1;
	}
	return a;		
}
	
int srm_mock_srm_getpermission (struct srm_context *context,
	struct srm_getpermission_input *input,struct srm_getpermission_output *output){
	int a = mock(context, input, output);
	if(a<0){
		errno = a;
		return -1;
	}
	memcpy(output, &defined_srm_getpermission_output, sizeof(struct srm_getpermission_output));
	return a;			
}

int srm_mock_srm_check_permission(struct srm_context *context,
	struct srm_checkpermission_input *input,struct srmv2_filestatus **statuses){
	int a = mock(context, input, statuses);
	if(a <0){
		errno = a;
		return -1;
	}
	*statuses = defined_srmv2_filestatus; 
	return a;				
}

int srm_mock_srm_prepare_to_get(struct srm_context *context,
	struct srm_preparetoget_input *input,struct srm_preparetoget_output *output){
	int a = mock(context, input, output);
	if(a <0 ){
		errno = a;
		return -1;
	}
	output->filestatuses = defined_get_output;
	return a;				
			
}

int srm_mock_srm_put_done(struct srm_context *context,
		struct srm_putdone_input *input, struct srmv2_filestatus **statuses){
		int a = mock(context, input, statuses);
		if(a < 0){
			errno =a;
			return -1;
		}
	*statuses = defined_put_done;
	return a;		
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


// convenience functions
