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


#include "gridftp_exist.h"
#include "gridftp_ifce_filecopy.h"

#include <externals/utils/uri_util.h>
#include <transfer/gfal_transfer_types_internal.h>

const Glib::Quark scope_filecopy("GridFTP::Filecopy");
const char * gridftp_checksum_transfer_config= "COPY_CHECKSUM_TYPE";

void gridftp_filecopy_delete_existing(GridFTP_session * sess, gfalt_params_t params, const char * url){
	const bool replace = gfalt_get_replace_existing_file(params,NULL);
	bool exist = gridftp_module_file_exist(sess, url);	
	if(exist){

		if(replace){
			gfal_log(GFAL_VERBOSE_TRACE, " File %s already exist, delete it for override ....",url); 
			gridftp_unlink_internal(sess, url, false);
			gfal_log(GFAL_VERBOSE_TRACE, " File %s deleted with success, proceed to copy ....",url); 									
		}else{
			char err_buff[GFAL_ERRMSG_LEN];
			snprintf(err_buff, GFAL_ERRMSG_LEN, " Destination already exist %s, Cancel", url);
			throw Gfal::CoreException(scope_filecopy, err_buff, EEXIST);
		}
	}
	
}


int gridftp_filecopy_copy_file_internal(GridFTPFactoryInterface * factory, gfalt_params_t params,
                                        const char* src, const char* dst){
    using namespace Gfal::Transfer;
    GError * tmp_err=NULL;

    const unsigned long timeout = gfalt_get_timeout(params, &tmp_err);
    Gfal::gerror_to_cpp(&tmp_err);
    std::auto_ptr<GridFTP_session> sess(factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(src)));

    gridftp_filecopy_delete_existing(sess.get(), params, dst);

    gfal_log(GFAL_VERBOSE_TRACE, "   [GridFTPFileCopyModule::filecopy] start gridftp transfer %s -> %s", src, dst);
    gfal_globus_result_t res = globus_gass_copy_url_to_url 	(sess->get_gass_handle(),
        (char*)src,
        GLOBUS_NULL,
        (char*)dst,
        GLOBUS_NULL
        );
    gfal_globus_check_result("GridFTPFileCopyModule::filecopy", res);
    return 0;

}

void gridftp_checksum_transfer_verify(const char * src_chk, const char* dst_chk, const char* user_defined_chk){
    if(*user_defined_chk == '\0'){
        if(strncasecmp(src_chk, dst_chk,GFAL_URL_MAX_LEN) != 0)
            throw Gfal::CoreException(scope_filecopy, Glib::ustring::compose("SRC and DST checksum are different %1 %2", src_chk, dst_chk),EIO);
    }else{
        if(strncasecmp(src_chk, dst_chk, GFAL_URL_MAX_LEN) != 0)
            throw Gfal::CoreException(scope_filecopy, Glib::ustring::compose("USER_DEFINE, SRC and DST checksum are different %1 %2 %3", user_defined_chk, src_chk, dst_chk),EIO);
    }
}


int GridftpModule::filecopy(gfalt_params_t params, const char* src, const char* dst){
    GError * tmp_err, *tmp_err_chk_src, *tmp_err_chk_copy, *tmp_err_chk_dst;
    tmp_err=tmp_err_chk_src=tmp_err_chk_dst=tmp_err_chk_copy=NULL;


    char checksum_src[GFAL_URL_MAX_LEN]={0};
    char checksum_dst[GFAL_URL_MAX_LEN]={0};
    char checksum_user_defined[GFAL_URL_MAX_LEN];
    char checksum_type_user_define[GFAL_URL_MAX_LEN];
    gboolean checksum_check = gfalt_get_checksum_check(params, &tmp_err);
    Gfal::gerror_to_cpp(&tmp_err);
    struct scoped_free{
        scoped_free() {checksum_algo=NULL;}
        ~scoped_free(){
            g_free(checksum_algo);
        }
        char *checksum_algo;
    }chk_algo; // automated destruction

    if(checksum_check){
        gfalt_get_user_defined_checksum(params, checksum_type_user_define, GFAL_URL_MAX_LEN,
                                        checksum_user_defined, GFAL_URL_MAX_LEN, &tmp_err); // fetch the user defined chk
        Gfal::gerror_to_cpp(&tmp_err);
        if(*checksum_user_defined == '\0' || *checksum_type_user_define == '\0'){ // check if user defined checksum exist
             chk_algo.checksum_algo = gfal2_get_opt_string(_handle_factory->get_handle(), GRIDFTP_CONFIG_GROUP, gridftp_checksum_transfer_config, &tmp_err);
            Gfal::gerror_to_cpp(&tmp_err);
            gfal_log(GFAL_VERBOSE_TRACE, "\t\tNo user defined checksum, fetch the default one from configuration ");
        }else{
            chk_algo.checksum_algo = g_strdup(checksum_type_user_define);
        }
        gfal_log(GFAL_VERBOSE_DEBUG, "\t\tChecksum Algorithm for transfer verification %s", chk_algo.checksum_algo);
    }

    #pragma omp parallel num_threads(2)
    {
        #pragma omp sections
        {
            #pragma omp section  // calc src checksum
            {
               CPP_GERROR_TRY
               if(checksum_check)
                    checksum(src, chk_algo.checksum_algo, checksum_src, GFAL_URL_MAX_LEN, 0,0);
               CPP_GERROR_CATCH(&tmp_err_chk_src);
            }
            #pragma omp section // start transfert and replace logic
            {
                  CPP_GERROR_TRY
                  gridftp_filecopy_copy_file_internal(_handle_factory, params, src, dst);
                  CPP_GERROR_CATCH(&tmp_err_chk_copy);
            }
        }
    }


    if(gfal_error_keep_first_err(&tmp_err,&tmp_err_chk_copy , &tmp_err_chk_src, &tmp_err_chk_dst, NULL)){
        Gfal::gerror_to_cpp(&tmp_err);
    }

    // calc checksum for dst
    if(checksum_check){
        checksum(dst, chk_algo.checksum_algo, checksum_dst, GFAL_URL_MAX_LEN, 0,0);
        gridftp_checksum_transfer_verify(checksum_src, checksum_dst, checksum_user_defined);
    }
    return 0;
}

extern "C"{

/**
 * initiaize a file copy from the given source to the given dest with the parameters params
 */
int plugin_filecopy(plugin_handle handle, gfal2_context_t context, gfalt_params_t params, const char* src, const char* dst, GError ** err){
	g_return_val_err_if_fail( handle != NULL && src != NULL
			&& dst != NULL , -1, err, "[plugin_filecopy][gridftp] einval params");

	GError * tmp_err=NULL;
	int ret = -1;
	gfal_log(GFAL_VERBOSE_TRACE, "  -> [gridftp_plugin_filecopy]");
	CPP_GERROR_TRY
		( static_cast<GridftpModule*>(handle))->filecopy(params, src, dst);
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfal_log(GFAL_VERBOSE_TRACE, "  [gridftp_plugin_filecopy]<-");
	G_RETURN_ERR(ret, tmp_err, err);
}


}
