#pragma once
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


 /*
  *
   file gfal_common_lfc.h
   brief header file for the lfc plugin module
   author Adrien Devresse
 */



#define GFAL_LFC_LIBRARY_NAME	"liblfc.so.1"
#define GFAL_LFC_PREFIX "lfn:"
#define GFAL_LFC_PREFIX2 "lfc://"
#define GFAL_LFC_GUID_PREFIX "guid:"
#define GFAL_LFC_GUID_PREFIX_LEN 5
#define GFAL_LFC_PREFIX_LEN 4
#define LFC_XATTR_GUID "user.guid"
#define LFC_XATTR_SURLS "user.replicas"
#define LFC_MAX_XATTR_LEN 2048
#define LFC_BUFF_SIZE 2048

#define LFC_PARAMETER_NAMESPACE "lfc"
#define LFC_PARAMETER_HOST "host"

#include <gsimplecache/gcachemain.h>

#include <gfal_plugins_api.h>

// LFC plugin GQuark
GQuark gfal2_get_plugin_lfc_quark();

// protos

gboolean gfal_checker_guid(const char* guid, GError** err);

gfal_plugin_interface lfc_initG(gfal2_context_t, GError**);


gboolean gfal_lfc_check_lfn_url(plugin_handle handle, const char* lfn_url, plugin_mode mode, GError** err);

char ** lfc_getSURLG(plugin_handle handle, const char * path, GError** err);

void lfc_set_session_timeout(int timeout);

int url_converter(plugin_handle handle, const char * url, char** host,
        char** path, GError** err);
