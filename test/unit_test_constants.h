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
 
 
 // constant to define for succefull unit test
#define TEST_LFC_VALID_ACCESS "lfn:/grid/dteam/hello001"		// this file must be a lfc file with read access and no write access
#define TEST_LFC_NOEXIST_ACCESS "lfn:/grid/dteam/PSGmarqueUNbut" // this file must never exist.
#define TEST_LFC_URL_SYNTAX_ERROR "jardiland.com"

#define TEST_GUID_VALID_ACCESS "guid:ae571282-81ea-41af-ac3c-8b1a084bfc82"
#define TEST_GUID_NOEXIST_ACCESS "guid:ae595782-81be-40af-ac3c-8b1a084bfc82"
