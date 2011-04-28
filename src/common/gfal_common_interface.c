
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
 * @file gfal_common_interface.c
 * @brief Wrapper file for the NON "G" functions
 * @author Devresse Adrien
 * @version 2.0
 * @date 12/04/2011
 * */

/**
 * @brief get a initiated gfal_handle
 * Wrapper of gfal_initG for the use cases without the GError system
 * */ 
gfal_handle gfal_init(){
	return gfal_initG(NULL);	
}

/**
 * 
 * 
 * */
int gfal_get_async(gfal_handle handle, GList* surls){
	
	return gfal_get_asyncG(handle, surls
}


