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

gfal_file gfal_file_new (const char *, const char *, int, char *, int);
int gfal_file_free (gfal_file);
const char *gfal_file_get_catalog_name (gfal_file);
const char *gfal_file_get_replica (gfal_file);
int gfal_file_get_replica_errcode (gfal_file);
const char *gfal_file_get_replica_errmsg (gfal_file);
int gfal_file_set_replica_error (gfal_file, int, const char *);
int gfal_file_set_turl_error (gfal_file, int, const char *);
int gfal_file_next_replica (gfal_file);
char *gfal_generate_lfn (char *, int);
char *gfal_generate_guid (char *, int);
