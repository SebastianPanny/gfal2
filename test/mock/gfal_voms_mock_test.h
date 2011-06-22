#pragma once
/*

*	auto-generated header file for file test/mock/gfal_lfc_mock_test.c 
 
*/

#include "voms/gfal_voms_if_layer.h"


extern struct vomsdata defined_vd;
extern char defined_Error_Message[2048];



void voms_mock_VOMS_Destroy(struct vomsdata *vd);

struct vomsdata *voms_mock_VOMS_Init(char *voms, char *cert);

char * voms_mock_VOMS_ErrorMessage(struct vomsdata *vd, int error, char *buffer, int len);

int voms_mock_VOMS_SetVerificationType(int type, struct vomsdata *vd, int *error);

int voms_mock_VOMS_RetrieveFromProxy(int how, struct vomsdata *vd, int *error);



