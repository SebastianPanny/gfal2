#pragma once
#ifndef _GFAL2_TRANSFER_
#define _GFAL2_TRANSFER_

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



#include <transfer/gfal_transfer_types.h>
#include <global/gfal_global.h>
#include <logger/gfal_logger.h>
#include <common/gfal_constants.h>

 
#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus


/**
 * @file gfal_transfer.h
 *  gfal2 API for file transfers .
 *  This API provide :
 *    - third party file transfers
 *    - monitoring of the transfer
 *    - flow control for the transfer
 *  @author Adrien Devresse
 */

/*!
    \defgroup transfer_group File Transfer API
*/

/*!
    \addtogroup transfer_group
    @{
*/





/**
	initiate a new parameter handle
*/
gfalt_params_t gfalt_params_handle_new(GError ** err);

/**
  delete a created parameters handle
*/
void gfalt_params_handle_delete(gfalt_params_t params, GError ** err);


/**
    create a copy of a parameter handle
*/
gfalt_params_t gfalt_params_handle_copy(gfalt_params_t params, GError ** err);

//
// parameters management functions
// these functions provide a way to configure a context for a transfer
// they must be called before starting the transfer
//


// TIMEOUT OPTIONS
/**
 * define the maximum time acceptable for the file tranfer
 * */
gint gfalt_set_timeout(gfalt_params_t, guint64 timeout, GError** err);

/**
 * get the maximum connexion timeout
 **/
guint64 gfalt_get_timeout(gfalt_params_t handle, GError** err);

// STREAM OPTIONS
/**
 * define the maximum number of parallels connexion to use for the file tranfer
 * */
gint gfalt_set_nbstreams(gfalt_params_t, guint nbstreams, GError** err);

/**
 * get the maximum number of parallels streams to use for the transfer
 **/
guint gfalt_get_nbstreams(gfalt_params_t params, GError** err);

// SRM SPECIFIC OPTIONS
/**
  set the source spacetoken for SRM transfers
*/
gint gfalt_set_src_spacetoken(gfalt_params_t params, const char* srm_spacetoken, GError** err);

/**
  get the source spacetoken for SRM transfers
*/
gchar* gfalt_get_src_spacetoken(gfalt_params_t params, GError** err);

/**
  set the destination spacetoken for SRM transfers
*/
gint gfalt_set_dst_spacetoken(gfalt_params_t params, const char* srm_spacetoken, GError** err);

/**
  get the destination spacetoken for SRM transfers
*/
gchar* gfalt_get_dst_spacetoken(gfalt_params_t params, GError** err);

// CONSISTENCY OPTIONS

/**
 * set the policy in case of destination file already existing ( replace or cancel )
 * default : cancel
 * */
gint gfalt_set_replace_existing_file(gfalt_params_t, gboolean replace, GError** err);

/**
 *  get the policy in case of destination file already existing ( replace or cancel )
 * default : cancel
 * */
gboolean gfalt_get_replace_existing_file(gfalt_params_t,  GError** err);

/**
  force additional checksum verification between source and destination
  an Error is return by the copy function is case of checksum failure.
  @warning for safety reason, even in case of checksum failure the destination file is not removed.
*/
gint gfalt_set_checksum_check(gfalt_params_t, gboolean value, GError** err);

/**
  get the checksum verification boolean
*/
gboolean gfalt_get_checksum_check(gfalt_params_t, GError** err);

/**
  set an user-defined checksum for file content verification
  set NULL & NULL clear the current one.
  This function requires to enable global checksum verification with \ref gfalt_set_checksum_check

  @param param : parameter handle
  @param chktype : checksum type string ( MD5, ADLER32, CRC32, etc... )
  @param checksum : value of checksum in string format
  @param err : GError error report

*/
gint gfalt_set_user_defined_checksum(gfalt_params_t param, const gchar* chktype,
                                const gchar* checksum, GError** err);

/**
  get the current user-defined checksum for file content verification
  if current user-defined checksum is NULL, both of the buffer are set to empty string

*/
gint gfalt_get_user_defined_checksum(gfalt_params_t params, gchar* chktype_buff, size_t chk_type_len,
                                gchar* checksum_buff, size_t checksum_len, GError** err);


/**
 * set the user_data pointer for statefull usages.
 * */
gint gfalt_set_user_data(gfalt_params_t, gpointer user_data, GError** err);



/**
 * set the minimum among of time between two calls of gfalt_monitor_tfr
 * 
 * */
gint gfalt_set_callback_mperiod(gfalt_params_t, guint mtime, GError** err); // time in ms between two callback calls.
// etc ......     



     
/**
 * @brief define a callback for monitoring the current transfer
 * The default value is NULL and no monitoring will occures
*/
void gfalt_set_monitor_callback(gfalt_params_t params, gfalt_monitor_func callback);



//
// Main function for transfer launch 
//

/**
 *	@brief copy function
 *  start a synchronous copy of the file
 *  @param context : gfal context
 *  @param params parameter handle ( \ref gfalt_parameters_new )
 *  @param src source URL supported by GFAL
 *  @param dst destination URL supported by GFAL
*/
int gfalt_copy_file(gfal2_context_t context, gfalt_params_t params, const char* src, const char* dst, GError** err);


//
//  Monitoring and flow control functions
//

/**
 * cancel the current file copy
 * */
gint gfalt_copy_cancel(gfalt_transfer_status_t, GError** err);

/* // not implemented
gint gfalt_copy_pause(gfalt_transfer_status_t, GError ** err );
gint  gfalt_copy_resume(gfalt_transfer_status_t, GError ** err);

*/

/**
 * 
 * */
gint gfalt_copy_get_status(gfalt_transfer_status_t, GError ** err);
/**
 * get an estimation of the baudrate
 * */
gint gfalt_copy_get_baudrate(gfalt_transfer_status_t, GError ** err);
/**
 * get the current number of bytes transfered
 * */
size_t gfalt_copy_get_bytes_transfered(gfalt_transfer_status_t, GError ** err);
/**
 * get the elapsed tiem since the call to \ref gfalt_copy_file
 * */
time_t gfalt_copy_get_elapsed_time(gfalt_transfer_status_t, GError ** err);

/**
    @}
    End of the POSIX group
*/

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_GFAL2_TRANSFER_

