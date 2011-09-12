#pragma once

/* unit test for common_plugin */

#include <stdio.h>
#include "lfc/lfc_ifce_ng.h"

struct lfc_ops* find_lfc_ops(gfal_handle handle, GError** err);

void test_mock_lfc(gfal_handle handle, GError** err);

void test_get_cat_type();



void test_env_var();


void test_plugin_access_file();



void test_plugin_url_checker();







void test__plugin_stat();


void test__plugin_lstat();
