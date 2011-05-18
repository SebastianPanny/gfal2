#pragma once
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
 * @file gfal_posix_local_file.c
 * @brief header file for the local access file map for the gfal_posix call
 * @author Devresse Adrien
 * @version 2.0
 * @date 06/05/2011
 * */
 
 
 
 gboolean gfal_check_local_url(const char* path, GError* err);
 
 
int gfal_local_access(const char *path, int amode, GError** err);
 
int gfal_local_chmod(const char* path, mode_t mode, GError** err);
 
int gfal_local_rename(const char* oldpath, const char* newpath, GError** err);
 
int gfal_local_stat(const char* path, struct stat* buf, GError ** err);
 
int gfal_local_lstat(const char* path, struct stat* buf, GError ** err);

int gfal_local_mkdir(const char* path, mode_t mode, GError** err);
