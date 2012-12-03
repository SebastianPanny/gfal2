#pragma once
#ifndef GFAL_TRANSFER_PLUGIN_H
#define GFAL_TRANSFER_PLUGIN_H

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

/**
 * @file gfal_transfer_plugins.h
 *  gfal API for file transfers of the gfal2_transfer shared library.
 *  This API provides specials functions calls reserved for the gfals plugins
 *  @author Adrien Devresse 
 */

#include <transfer/gfal_transfer_types.h>
#include <transfer/gfal_transfer.h>

 
#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus



//
// full list of functions that are re-searched by GFAL 2.0 in the plugins
//

/** prototype for the url_check entry point : this entry point is mandatory !!!
 *
 * */
typedef int(*plugin_url_check2_call)(plugin_handle,  const char* src, const char* dst, gfal_url2_check check);

// prototype for the filecopy entry point in the plugins
typedef int (*plugin_filecopy_call)(plugin_handle, gfal2_context_t, gfalt_params_t, const char* src, const char* dst, GError** );


typedef const char * (*plugin_name_call)();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_GFAL2_TRANSFER_

