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

#include <glib.h>
#include "gridftpmodule.h"
#include "gridftpwrapper.h"

#include <globus_gass_copy.h>
#include <globus_ftp_client.h>
#include <globus_ftp_client_debug_plugin.h>
#include <globus_ftp_client_throughput_plugin.h>


//GOnce my_once = G_ONCE_INIT;

static Glib::Quark GFAL_GRIDFTP_SCOPE_GLOBUS_INIT("GridFTPModule::init_globus");

static Glib::Mutex mux_globus_init;


// initialization
__attribute__((constructor))
void core_init()
{
#if  (!GLIB_CHECK_VERSION (2, 32, 0))
    if (!g_thread_supported())
    g_thread_init(NULL);
#endif
    Glib::init();
}

// gfunction prototype : gonce
static void* init_globus(gpointer data)
{
    Glib::Mutex::Lock l(mux_globus_init);
    globus_result_t result = GLOBUS_SUCCESS;
    if ((result = globus_module_activate(GLOBUS_GASS_COPY_MODULE))
            != GLOBUS_SUCCESS)
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GLOBUS_INIT,
                "Error globus init, globus gass", result);
    if ((result = globus_module_activate(GLOBUS_FTP_CLIENT_MODULE))
            != GLOBUS_SUCCESS)
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GLOBUS_INIT,
                "Error globus init, globus ftp", result);
    if ((result = globus_module_activate(GLOBUS_FTP_CLIENT_DEBUG_PLUGIN_MODULE))
            != GLOBUS_SUCCESS)
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GLOBUS_INIT,
                "Error globus init, globus ftp debug", result);
    if ((result = globus_module_activate(GLOBUS_FTP_CLIENT_THROUGHPUT_PLUGIN_MODULE))
            != GLOBUS_SUCCESS)
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GLOBUS_INIT,
                "Error globus init, globus ftp throughput plugin", result);
    return NULL;
}


static void* deinit_globus(gpointer data)
{
    Glib::Mutex::Lock l(mux_globus_init);
    globus_result_t result = GLOBUS_SUCCESS;
    if ((result = globus_module_deactivate(GLOBUS_GASS_COPY_MODULE))
            != GLOBUS_SUCCESS)
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GLOBUS_INIT,
                "Error globus deinit, globus gass", result);
    if ((result = globus_module_deactivate(GLOBUS_FTP_CLIENT_MODULE))
            != GLOBUS_SUCCESS)
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GLOBUS_INIT,
                "Error globus deinit, globus ftp", result);
    if ((result = globus_module_deactivate(
            GLOBUS_FTP_CLIENT_DEBUG_PLUGIN_MODULE)) != GLOBUS_SUCCESS)
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GLOBUS_INIT,
                "Error globus deinit, globus ftp debug", result);
    if ((result = globus_module_deactivate(
            GLOBUS_FTP_CLIENT_THROUGHPUT_PLUGIN_MODULE)) != GLOBUS_SUCCESS)
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GLOBUS_INIT,
                "Error globus deinit, globus ftp throughput plugin", result);
    return NULL;
}


GridFTPModule::GridFTPModule(GridFTPFactory* factory)
{
    init_globus(NULL);
    _handle_factory = factory;
}

GridFTPModule::~GridFTPModule()
{
    delete _handle_factory;
    deinit_globus(NULL);
}



