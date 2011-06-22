#pragma once
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file gfal_voms_if_layer.h
 * @brief file containing the layer for the voms external call (mock purpose )
 * @author Devresse Adrien
 * @version 2.0
 * @date 22/06/2011
 * */


#include "voms_apic.h"

struct _gfal_voms_external{
	void (*VOMS_Destroy)(struct vomsdata *vd);
	struct vomsdata *(*VOMS_Init)(char *voms, char *cert);
	char *(*VOMS_ErrorMessage)(struct vomsdata *vd, int error, char *buffer, int len);
	int (*VOMS_SetVerificationType)(int type, struct vomsdata *vd, int *error);
	int (*VOMS_RetrieveFromProxy)(int how, struct vomsdata *vd, int *error);
};


extern struct _gfal_voms_external gfal_voms_external;
