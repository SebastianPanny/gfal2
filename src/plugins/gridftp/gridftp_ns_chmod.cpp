/*
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


#include "gridftp_namespace.h"
#include <exceptions/cpp_to_gerror.hpp>


static Glib::Quark GFAL_GRIDFTP_SCOPE_CHMOD("GridFTPModule::chmod");


void GridFTPModule::chmod(const char* path, mode_t mode)
{
    if (path == NULL)
        throw Glib::Error(GFAL_GRIDFTP_SCOPE_CHMOD, EINVAL,
                "Invalid arguments path or mode ");
    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridFTPModule::chmod] ");

    GridFTPSessionHandler handler(_handle_factory, path);
    GridFTPRequestState req(&handler);

    globus_result_t res = globus_ftp_client_chmod(req.handler->get_ftp_client_handle(),
            path, mode, req.handler->get_ftp_client_operationattr(),
            globus_ftp_client_done_callback, &req);
    gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_CHMOD, res);
    // wait for answer
    req.wait(GFAL_GRIDFTP_SCOPE_CHMOD);

    gfal_log(GFAL_VERBOSE_TRACE, " <- [GridFTPModule::chmod] ");

}


extern "C" int gfal_gridftp_chmodG(plugin_handle handle, const char* path,
        mode_t mode, GError** err)
{
    g_return_val_err_if_fail(handle != NULL && path != NULL, -1, err,
            "[gfal_gridftp_chmodG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_chmod]");
    CPP_GERROR_TRY
                (static_cast<GridFTPModule*>(handle))->chmod(path, mode);
                ret = 0;
            CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_chmod]<-");
    G_RETURN_ERR(ret, tmp_err, err);
}
