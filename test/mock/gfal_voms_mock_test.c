/**
 *  mock for the voms part
 * 
 * 
 * */
 
#include <cgreen/cgreen.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "gfal_voms_mock_test.h"

struct vomsdata defined_vd;
char defined_Error_Message[2048];


void voms_mock_VOMS_Destroy(struct vomsdata *vd){
	mock(vd);
	return;
}

struct vomsdata *voms_mock_VOMS_Init(char *voms, char *cert){
	int a = mock(voms, cert);
	if(a)
		return &defined_vd;
	return NULL;
}

char * voms_mock_VOMS_ErrorMessage(struct vomsdata *vd, int error, char *buffer, int len){
	int a = mock(vd, error, buffer, len);
	if(a)
		return defined_Error_Message;
	return NULL;
}

int voms_mock_VOMS_SetVerificationType(int type, struct vomsdata *vd, int *error){
	int a = mock(type, vd, error);
	return a;
}

int voms_mock_VOMS_RetrieveFromProxy(int how, struct vomsdata *vd, int *error){
	int a = mock(how, vd, error);
	return a;	
}
