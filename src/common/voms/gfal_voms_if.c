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
 * @file gfal_voms_if.c
 * @brief file containing the interface to the voms system
 * @author Devresse Adrien
 * @version 2.0
 * @date 12/04/2011
 * */
 
#include "gfal_voms_if.h"

typedef struct gfal_voms_info__{
	GArray* fqan;
	char* userdn;
	char* vo;
	
} gfal_voms_info_;

static *gfal_voms_info_ gfal_voms_info=NULL; 
 
int gfal_parse_vomsdataG (GError **)
{
    if (!vomsdataparsed) {
        struct vomsdata *vd;
        int i, len, error;
        char *pos, *p1, *p2;
        char errmsg[GFAL_ERRMSG_LEN];

        if ((vd = VOMS_Init ("", "")) == NULL ||
                !VOMS_SetVerificationType (VERIFY_NONE, vd, &error) ||
                !VOMS_RetrieveFromProxy (RECURSE_CHAIN, vd, &error)) {
            if (error != VERR_NOEXT) { /* error is not "VOMS extension not found!" */
                VOMS_ErrorMessage (vd, error, errmsg, GFAL_ERRMSG_LEN);
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][VOMS_RetrieveFromProxy][] %s", errmsg);
            }
            VOMS_Destroy (vd);
            return (-1);
        }
        else if (!vd->data || !vd->data[0]) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_parse_vomsdata][] Unable to get VOMS info from the proxy (Memory problem?)");
            return (-1);
        }

        gfal_userdn = strdup (vd->data[0]->user);
        gfal_vo = gfal_vo == NULL ? strdup (vd->data[0]->voname) : gfal_vo;

        for (i = 0; vd->data[0]->fqan[i] != NULL; ++i) {
            if ((gfal_fqan[i] = strdup (vd->data[0]->fqan[i])) == NULL)
                return (-1);

            pos = p1 = gfal_fqan[i];
            if (*p1 != '/') {
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_parse_vomsdata][] Invalid FQAN: %s", gfal_fqan[i]);
                return (-1);
            }

            /* 'Role=NULL' and 'Capability=NULL' are removed from the fqan */
            while (p1 != NULL) {
                p2 = strchr (p1 + 1, '/');
                if (p2) *p2 = '\0';

                if (!strstr (p1, "=NULL")) {
                    if (pos == p1)
                        pos = p2;
                    else {
                        len = strlen (p1);
                        memmove (pos, p1, len + 1);
                        pos += len;
                    }
                }

                p1 = p2;
                if (p1) *p1 = '/';
            }

            if (pos) *pos = '\0';
        }

        gfal_fqan[i] = NULL;
        gfal_nb_fqan = i;
        vomsdataparsed = 1;
        VOMS_Destroy (vd);
    }

    return (0);
}

char *gfal_get_userdnG (GError ** err)
{
	GError* tmp_err=NULL;
    if (gfal_voms_info == NULL){
        if( gfal_parse_vomsdata(&err) != 0){
			g_propagate_prefixed_error (err, tmp_err, "[get_userdn]");
			return NULL;
		}			
    }
    return strndup(gfal_voms_info->userdn,100);
}



char * gfal_get_voG (GError** err)
{
	char* gfal_vo=NULL;
	GError* tmp_err=NULL;
	
	if((gfal_vo = getenv ("LCG_GFAL_VO")) != NULL){	// check var env
		gfal_print_verbose(GFAL_VERBOSE_VERBOSE,"[gfal_get_voG] LCG_GFAL_VO env var defined, this VO will be used");
		return gfal_vo;
	}
	
	if(gfal_voms_info == NULL){			// obtain from the singleton
		if(gfal_parse_vomsdata (&tmp_err) != 0){
			g_propagate_prefixed_error(err,tmp_err,"[get_voG]");
			return NULL;
		}
	}
    return strndup(gfal_voms_info->vo,100);
}

GList* gfal_get_fqanG (GError**)
{
  	if(gfal_voms_info == NULL){			// obtain from the singleton
		if(gfal_parse_vomsdata (&tmp_err) != 0){
			g_propagate_prefixed_error(err,tmp_err,"[get_voG]");
			return NULL;
		}
	} 
    
    return gfal_voms_info->fqan;
}
