/*
 * 
 *  convenience function for the mocks or the srm interface
 * 
 */



#include "gfal_srm_mock_test.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>


static char buff_valid_dir[2048];
static char buff_noent_dir[2048];
static char buff_access_dir[2048];
static char buff_empty_dir[2048];

static char* tab[]= { "testdir001", "testdir002", "testdir003", "testdir004", NULL };

char** list_dir_srm(){
	return tab;
}

int count_dir_srm(){
	int i =0;
	while(tab[i] != NULL)
		++i;
	return i;
}


char* srm_valid_dir(){
	strcpy(buff_valid_dir, "hello");
	strcat(buff_valid_dir, "/grid/dteam/testdirvalid001");
	return buff_valid_dir;
}

char* srm_valid_empty_dir(){
	strcpy(buff_empty_dir, "hello");	
	strcat(buff_empty_dir, "/grid/dteam/testdirempty001");	
	return buff_empty_dir;
}

char* srm_noent_dir(){
	strcpy(buff_noent_dir, "hello");
	strcat(buff_noent_dir, "/grid/dteam/testdirinvalid001");
	return buff_noent_dir;
}

char* srm_noaccess_dir(){
	strcpy(buff_access_dir, "hello");
	strcat(buff_access_dir, "/grid/dteam/testdirinvalid0012");
	return buff_access_dir;
}


static void srm_context_mock_implem(struct srm_context *context,char *srm_endpoint,char *errbuf,int errbufsz,int verbose){
	return;
}

void mock_srm_context(){
	gfal_srm_external_call.srm_context_init = &srm_context_mock_implem;
}

void unmock_srm_context(){
	gfal_srm_external_call.srm_context_init= &srm_context_init;	
}



static int srm_ls_mock_implem(struct srm_context *context,
		struct srm_ls_input *input,struct srm_ls_output *output){
	int ret = -1,i;
	if( !context  || !input || !output){
		g_printerr(" bad call to the srm_ls mock");
		errno = EINVAL;
		return -1;
	} 
	if(input->nbfiles != 1)	{
		g_printerr(" bad number of file in the mock call");
		errno = EINVAL;
		return -1;		
	}
	if(!input->surls){
		g_printerr(" no valdi surl in the moch call");
		errno = EINVAL;
		return -1;		
	}
	if(input->numlevels != 1){
		g_printerr(" bad flag level in input");
		errno = EINVAL;
		return -1;			
	}
	if( strcmp(*input->surls, srm_valid_dir()) ==0  ){
		memset(output,0, sizeof(struct srm_ls_output));
		output->statuses = calloc(sizeof(struct srmv2_mdfilestatus),1);
		output->statuses->surl = strdup(*input->surls);
		output->statuses->nbsubpaths= 4;
		output->statuses->subpaths = calloc(sizeof(struct srmv2_mdfilestatus),4);
		char** list_dir = list_dir_srm();
		for(i=0; i< 4; ++i){
			output->statuses->subpaths[i].surl = strdup(list_dir[i]);
		}
		ret =0;
	}else if(strcmp(*input->surls, srm_valid_empty_dir()) ==0){
		memset(output,0, sizeof(struct srm_ls_output));
		output->statuses = calloc(sizeof(struct srmv2_mdfilestatus),1);
		output->statuses->surl = strdup(*input->surls);
		output->statuses->nbsubpaths= 0;
		output->statuses->subpaths = NULL;
		ret =0;
		
	}else if( strcmp(*input->surls, srm_noaccess_dir()) ==0 ){
		memset(output,0, sizeof(struct srm_ls_output));
		output->statuses = calloc(sizeof(struct srmv2_mdfilestatus),1);
		output->statuses->surl = strdup(*input->surls);
		output->statuses->explanation = strdup(" error enoent from mock srm");
		output->statuses->status= EACCES;	
		ret =0;	
	}else if(strncmp(*input->surls, "srm://",6) == 0){
		memset(output,0, sizeof(struct srm_ls_output));
		output->statuses = calloc(sizeof(struct srmv2_mdfilestatus),1);
		output->statuses->surl = strdup(*input->surls);
		output->statuses->explanation = strdup(" error eacces from mock srm");
		output->statuses->status= ENOENT;	
		ret =0;	
	}else{
		g_printerr(" incorrect surl input");
		errno = EINVAL;
		return -1;			
	}
	return ret;
}

void mock_srm_ls(){
	gfal_srm_external_call.srm_ls = &srm_ls_mock_implem;	
}

void unmock_srm_ls(){
	gfal_srm_external_call.srm_ls = &srm_ls;		
}




void mock_all_srm(){
	mock_srm_context();
	mock_srm_ls();
}


void unmock_all_srm(){
	unmock_srm_context();
	unmock_srm_ls();
}

