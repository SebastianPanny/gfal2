/* 
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
* 
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at 
*
*    http://www.apache.org/licenses/LICENSE-2.0 
* 
* Unless required by applicable law or agreed to in writing, software 
* distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License.
*/


#include <cstring>

#include <transfer/gfal_transfer.h>
#include <common/gfal_common_errverbose.h>

#include "gridftpmodule.h"
#include "gridftp_plugin_main.h"


const char * gridftp_prefix = "gsiftp://";

extern "C"
{
	
static bool gridftp_check_url(const char* src){
	const size_t s_gftppref = strlen(gridftp_prefix);		
	if(s_gftppref < strlen(src) &&
		strncmp(src, gridftp_prefix, s_gftppref) ==0 )
		return TRUE;
	return FALSE;
}

/**
 * 
 *  implementation of the plugi_url_transfer_check for the gridFTP plugin
 *  Check if the gridFTP plugin is able to handle a given type of URL.
 * 
 * */
gboolean plugin_url_check2(plugin_handle handle, const char* src, const char* dst, gfal_url2_check type ){
	g_return_val_if_fail(handle != NULL,FALSE);
	gboolean res = FALSE;

	
    if( src != NULL && dst != NULL){
		if( type == GFAL_FILE_COPY 
            && gridftp_check_url(src)
            && gridftp_check_url(dst)
            ){
				res= TRUE;
			}
		
	}
	return res;
}

int plugin_url_check_with_gerror(plugin_handle handle, const char* src, plugin_mode check, GError ** err){
	return plugin_url_check(handle, src, check);
}

gboolean plugin_url_check(plugin_handle handle, const char* src, plugin_mode check){
	gboolean res = FALSE;
	if(gridftp_check_url(src) == true){
		switch(check){
			case GFAL_PLUGIN_ACCESS:
			case GFAL_PLUGIN_STAT:
			case GFAL_PLUGIN_LSTAT:
			case GFAL_PLUGIN_MKDIR:
			case GFAL_PLUGIN_CHMOD:
			case GFAL_PLUGIN_RMDIR:
			case GFAL_PLUGIN_OPENDIR:
			case GFAL_PLUGIN_UNLINK:
			case GFAL_PLUGIN_OPEN:
            case GFAL_PLUGIN_CHECKSUM:
            case GFAL_PLUGIN_RENAME:
				res = TRUE;
				break;
			default:
				break;
			
		}
	}
	return res;
	
}



}
