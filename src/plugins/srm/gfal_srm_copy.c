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
 * @file gfal_srm_copy.c
 * @brief file for the third party transfer implementation
 * @author Devresse Adrien
 * */
#include <omp.h>

#include <checksums/checksums.h>
#include <common/gfal_types.h>
#include <common/gfal_common_errverbose.h>
#include <file/gfal_file_api.h>
#include <transfer/gfal_transfer.h>
#include <transfer/gfal_transfer_plugins.h>
#include <uri/uri_util.h>

#include "gfal_srm_getput.h"
#include "gfal_srm_stat.h"
#include "gfal_srm_url_check.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_checksum.h"
#include "gfal_srm_mkdir.h"

GQuark srm_quark_3rd_party(){
    return g_quark_from_static_string("srm_plugin::filecopy");
}

GQuark srm_domain() {
  return g_quark_from_static_string("SRM");
}

int srm_plugin_get_3rdparty(plugin_handle handle, gfalt_params_t params, const char * surl,
                            char* buff, size_t s_buff,
                            GError ** err){
	GError * tmp_err=NULL;
	int res = -1;
	if( srm_check_url(surl) ){
        gfal_log(GFAL_VERBOSE_TRACE, "\t\tGET surl -> turl dst resolution start");
        if( (res =gfal_srm_get_rd3_turl(handle, params, surl, buff , s_buff, NULL,  err)) == 0){
            gfal_log(GFAL_VERBOSE_TRACE, "\t\tGET surl -> turl dst resolution ended : %s -> %s", surl, buff);	    
		}
	}else{
		res =0;
		g_strlcpy(buff, surl, s_buff);
        gfal_log(GFAL_VERBOSE_TRACE, "\t\tno SRM resolution needed on %s", surl);
	}	
	G_RETURN_ERR(res, tmp_err, err);		
}

int srm_plugin_delete_existing_copy(plugin_handle handle, gfalt_params_t params, 
									const char * surl, GError ** err){
	GError * tmp_err=NULL;
	int res = 0;
	const gboolean replace = gfalt_get_replace_existing_file(params, NULL);
	if(replace){
        struct stat st_dest_file;
        if( (res = gfal_srm_statG(handle, surl, &st_dest_file, &tmp_err)) == 0 ){
            gfal_log(GFAL_VERBOSE_TRACE, "   %s found, delete in order to replace it", surl);
            if( ( res = gfal_srm_unlinkG(handle, surl, &tmp_err)) ==0){
                gfal_log(GFAL_VERBOSE_TRACE, "   %s deleted with sucess", surl);
            }
        }
        if(tmp_err && tmp_err->code == ENOENT){
            gfal_log(GFAL_VERBOSE_TRACE, " %s dest does not exist, no over-write needed, begin copy", surl);
            g_clear_error(&tmp_err);
            res = 0;
        }
	}

	G_RETURN_ERR(res, tmp_err, err);		
}

// create the parent directory 
// return 0 if nothing or not requested
// return 1 if creation has been done
// return < 0 in case of error
int srm_plugin_create_parent_copy(plugin_handle handle, gfalt_params_t params, 
									const char * surl, GError ** err){
	GError * tmp_err=NULL;
	int res = -1;
	const gboolean create_parent = gfalt_get_create_parent_dir(params, NULL);
	if(create_parent){
		char * path_dir = g_strdup(surl);
		char *p = path_dir + strlen(path_dir)-1;
		while(*p == '/'){ // remote trailing /
			*p='\0';
			p--;
		}
		const unsigned int pref_len = GFAL_PREFIX_SRM_LEN;
		while(*p  != '/' && (path_dir + pref_len) < p)
			p--;
		if( (path_dir + pref_len) < p){ 
			*p='\0';
			gfal_log(GFAL_VERBOSE_TRACE, " try to create parent dir : %s for %s", path_dir, surl);
			res = gfal_srm_mkdir_recG(handle, path_dir, 0755, &tmp_err);
			if(res == 0)
				gfal_log(GFAL_VERBOSE_TRACE, "parent path %s created with success", path_dir);			
		}else{
			g_set_error(&tmp_err, srm_quark_3rd_party(), EINVAL, "Invalid srm url %s",surl);
			res= -1;
		}
		g_free(path_dir);
	}else{
		res = 0;
	}
	G_RETURN_ERR(res, tmp_err, err);		
}

// prepare srm destination for 3rd party copy
int srm_plugin_prepare_dest_put(plugin_handle handle, gfal2_context_t context,
					gfalt_params_t params,  const char * surl, GError ** err){
	GError * tmp_err=NULL;
	int res = -1;						
    if( (res = srm_plugin_delete_existing_copy(handle, params, surl, &tmp_err)) == 0
        &&  (res = srm_plugin_create_parent_copy(handle, params, surl, &tmp_err) ) == 0){
	}
	G_RETURN_ERR(res, tmp_err, err);								
}

int srm_plugin_put_3rdparty(plugin_handle handle, gfal2_context_t context,
                    gfalt_params_t params,  const char * surl,  size_t file_size_surl,
					char* buff, size_t s_buff, 
					char** reqtoken, GError ** err){
	GError * tmp_err=NULL;
	int res = -1;
	
	if( srm_check_url(surl)){
        gfal_log(GFAL_VERBOSE_TRACE, "\t\tPUT surl -> turl src resolution start ");
        if( (res = srm_plugin_prepare_dest_put(handle, context, params, surl, &tmp_err)) ==0){
            if(( res= gfal_srm_put_rd3_turl(handle, params, surl, file_size_surl, buff , s_buff, reqtoken,  &tmp_err))==0)
                gfal_log(GFAL_VERBOSE_TRACE, "\t\tPUT surl -> turl src resolution ended : %s -> %s", surl, buff);
		}
	}else{
        res =1;
		g_strlcpy(buff, surl, s_buff);
		gfal_log(GFAL_VERBOSE_TRACE, "		no SRM resolution needed on %s", surl);			
	}
	G_RETURN_ERR(res, tmp_err, err);	
}

int srm_plugin_check_checksum(plugin_handle handle, gfal2_context_t context,
                              gfalt_params_t params,
                              gboolean allow_empty_checksum,
                              const char* src, char* buff_chk, GError ** err){

    char buff_user_defined[GFAL_URL_MAX_LEN]={0};
    char buff_user_defined_type[GFAL_URL_MAX_LEN]={0};
    char * chk_type=NULL;
    GError * tmp_err=NULL;
    int res=0;

    if(gfalt_get_checksum_check(params, &tmp_err)){
       gfal_log(GFAL_VERBOSE_TRACE,"\t\tCompute SRM checksum for %s",src);
       gfalt_get_user_defined_checksum(params, buff_user_defined_type, GFAL_URL_MAX_LEN,
                                        buff_user_defined, GFAL_URL_MAX_LEN, NULL); // fetch the user defined chk
       const gboolean user_defined = (*buff_user_defined!='\0' && *buff_user_defined_type!='\0');

       if(!user_defined){
           chk_type = gfal2_get_opt_string(context, srm_config_group,srm_config_transfer_checksum,&tmp_err);
           gfal_log(GFAL_VERBOSE_TRACE, "\t\tNo checksum type defined by user, take it from configuration : %s", chk_type);
       }else{
           chk_type=g_strdup(buff_user_defined_type);
       }

       if (chk_type)
       {
           res = gfal_srm_checksumG_fallback(handle, src, chk_type,
                       buff_chk, GFAL_URL_MAX_LEN,
                       0, 0,
                       !allow_empty_checksum,
                       &tmp_err);

           if ((res != 0 || buff_chk[0] == '\0') && allow_empty_checksum)
           {
               gfal_log(GFAL_VERBOSE_VERBOSE, "\t\tNo checksum returned, but SRM plugin set to tolerate empty checksums on the source");
               res = 0;
               if (tmp_err)
                   g_error_free(tmp_err);
               tmp_err = NULL;
           }
           else if (res == 0 && user_defined && gfal_compare_checksums(buff_user_defined, buff_chk, GFAL_URL_MAX_LEN ) != 0)
           {
               g_set_error(&tmp_err, srm_quark_3rd_party(), EIO,
                           "Checksum of %s and user defined checksum do not match %s %s",
                           src, buff_chk, buff_user_defined);
               res = -1;
           }

           g_free(chk_type);
       }
    }
    G_RETURN_ERR(res, tmp_err, err);
}


int srm_compare_checksum_transfer(gfalt_params_t params, const char* src, const char* dst,
                                  char* src_buff_checksum,
                                  char* dst_buff_checksum, GError** err){
    int res = 0;

    if(gfalt_get_checksum_check(params, err)){
        if( gfal_compare_checksums(src_buff_checksum, dst_buff_checksum, GFAL_URL_MAX_LEN) !=0){
            g_set_error(err, srm_quark_3rd_party(),EIO, "Checksum of %s and %s does not match %s %s",
                        src, dst, src_buff_checksum, dst_buff_checksum);
            res = -1;
        }else{
            res = 0;
        }
    }
    return res;
}

int plugin_filecopy(plugin_handle handle, gfal2_context_t context,
					gfalt_params_t params, 
					const char* src, const char* dst, GError ** err){
	g_return_val_err_if_fail( handle != NULL && src != NULL
			&& dst != NULL , -1, err, "[plugin_filecopy][gridftp] einval params");	
	
	gfal_log(GFAL_VERBOSE_TRACE, "  -> [srm_plugin_filecopy] ");
    GError * tmp_err=NULL;
    int res = -1;
    gboolean put_waiting= FALSE;
    char buff_turl_src[GFAL_URL_MAX_LEN]={0};
    char buff_src_checksum[GFAL_URL_MAX_LEN]={0};
    char buff_turl_dst[GFAL_URL_MAX_LEN]={0};
    char buff_dst_checksum[GFAL_URL_MAX_LEN]={0};
	char* reqtoken = NULL;
    gfalt_params_t params_turl = gfalt_params_handle_copy(params, &tmp_err);  // create underlying protocol parameters
    gfalt_set_checksum_check(params_turl, FALSE,NULL); // disable already does actions

    gboolean allow_empty_source_checksum = gfal2_get_opt_boolean(context, "SRM PLUGIN", "ALLOW_EMPTY_SOURCE_CHECKSUM", NULL);

    GError * tmp_err_get, *tmp_err_put,*tmp_err_chk_src, *tmp_err_cancel;
    tmp_err_chk_src= tmp_err_get = tmp_err_put = tmp_err_cancel= NULL;

    plugin_trigger_event(params, srm_domain(),
                         GFAL_EVENT_NONE, GFAL_EVENT_PREPARE_ENTER,
                         "");



       // PARALLELIZABLE SECTIONS
        {
            plugin_trigger_event(params, srm_domain(),
                                 GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_ENTER,
                                 "");
            srm_plugin_check_checksum(handle, context, params,
                                      allow_empty_source_checksum,
                                      src, buff_src_checksum,
                                      &tmp_err_chk_src);
            plugin_trigger_event(params, srm_domain(),
                                 GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_EXIT,
                                 "");
        }

        {
            srm_plugin_get_3rdparty(handle, params, src, buff_turl_src, GFAL_URL_MAX_LEN, &tmp_err_get);
        }

        {
            int ret_put =-1;
            struct stat st_src;
            memset(&st_src, 0, sizeof(  struct stat));
            if( gfal2_stat(context, src, &st_src, &tmp_err_put) !=0){
               st_src.st_size =0;
               gfal_log(GFAL_VERBOSE_DEBUG, "Fail to stat src SRM url %s to determine file size, try with file_size=0, error %s",
                        src, tmp_err_put->message);
               g_clear_error(&tmp_err_put);
            }

            ret_put = srm_plugin_put_3rdparty(handle, context, params, dst, st_src.st_size,
                                              buff_turl_dst, GFAL_URL_MAX_LEN, &reqtoken, &tmp_err_put);
            if(!tmp_err_put && reqtoken != NULL)
                put_waiting = TRUE;
            if(ret_put == 0){ // srm resolution done to turl, do not check dest -> already done
                gfalt_set_replace_existing_file(params_turl,FALSE, NULL);
                gfalt_set_strict_copy_mode(params_turl, TRUE, NULL);
            }
        }


    gfal_srm_check_cancel(context, &tmp_err_cancel);

    plugin_trigger_event(params, srm_domain(),
                         GFAL_EVENT_NONE, GFAL_EVENT_PREPARE_EXIT,
                         "");

   if( !gfal_error_keep_first_err(&tmp_err, &tmp_err_get, &tmp_err_chk_src, &tmp_err_put, &tmp_err_cancel, NULL) ){ // do the first resolution

            if(!tmp_err){
                res = gfalt_copy_file(context, params_turl, buff_turl_src, buff_turl_dst, &tmp_err);
                if( res == 0 && put_waiting){
                    gfal_log(GFAL_VERBOSE_TRACE, "\ttransfer executed, execute srm put done"); // commit transaction

                    plugin_trigger_event(params, srm_domain(),
                                         GFAL_EVENT_DESTINATION, GFAL_EVENT_CLOSE_ENTER,
                                         "%s", dst);

                    res= gfal_srm_putdone_simple(handle, dst, reqtoken, &tmp_err);
                    if(res ==0){
                        put_waiting = FALSE;

                        plugin_trigger_event(params, srm_domain(),
                                             GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_ENTER,
                                             "");
                        // try to get result checksum
                        res = srm_plugin_check_checksum(handle, context, params, FALSE, dst, buff_dst_checksum, &tmp_err);
                        if(res == 0 && (buff_src_checksum[0] != '\0' || !allow_empty_source_checksum))
                        {
                            res= srm_compare_checksum_transfer(params, src, dst,
                                                              buff_src_checksum,
                                                              buff_dst_checksum,
                                                              &tmp_err);
                        }

                        plugin_trigger_event(params, srm_domain(),
                                             GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_EXIT,
                                             "");
                    }

                    plugin_trigger_event(params, srm_domain(),
                                         GFAL_EVENT_DESTINATION, GFAL_EVENT_CLOSE_EXIT,
                                         "%s", dst);
                }

            }
    }

    if(put_waiting){ // abort request
           gfal_log(GFAL_VERBOSE_TRACE, "\tCancel PUT request for %s", dst);
           GError * tmp_err_cancel=NULL;
           srm_abort_request_plugin(handle, dst, reqtoken, &tmp_err_cancel);
           // log silent error
           if(tmp_err_cancel)
               gfal_log(GFAL_VERBOSE_DEBUG, " Error while canceling put on %s: %s", dst, tmp_err_cancel->message);
          // clear the trash file silently
           gfal_srm_unlinkG(handle, dst,NULL);
    }

    gfalt_params_handle_delete(params_turl, NULL);
	gfal_log(GFAL_VERBOSE_TRACE, " [srm_plugin_filecopy] <-");	
	G_RETURN_ERR(res, tmp_err, err);		
}


int srm_plugin_filecopy(plugin_handle handle, gfal2_context_t context,
                    gfalt_params_t params,
                    const char* src, const char* dst, GError ** err){
    return plugin_filecopy(handle, context, params, src, dst, err);
}