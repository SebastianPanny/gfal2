

/* unit test for common_srm */


#include <check.h>
#include "gfal_common.h"





START_TEST (test_create_srm_handle)
{
	GError* err=NULL;
	gfal_handle handle = gfal_initG(&err);
	fail_unless(handle != NULL, " handle is not properly allocated");
	fail_unless(err==NULL, " error must be NULL");
	gfal_handle_freeG(handle);
}
END_TEST

START_TEST(test_glist_to_surls)
{
		GList* list = g_list_append(NULL,"bob");
		list = g_list_append(list, "gilles");
		list = g_list_append(list, "john");
		int n = g_list_length(list);
		fail_unless(n == 3, " size error, see internal glist doc");
		char** surls =gfal_GList_to_tab(list);
		int i;
		for(i=0; i< n; ++i){
			char * str= list->data;
			fail_unless(strncmp(str,surls[i],100) == 0, " must be the same string");
			list = g_list_next(list);
		}
		fail_unless(surls[n]==NULL, " last element+1 must be null");
}
END_TEST


START_TEST(test_gfal_get_async_1)
{
	GError* err=NULL;
	gfal_handle handle = gfal_initG(&err);
	if(handle == NULL){
			gfal_release_GError(&err);
			fail(" handle fail to initiated");
	}
	GList* list = g_list_append(NULL,"srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/testfile002");
	int r = gfal_get_asyncG(handle,list,&err);
	if(r < 0){
			gfal_release_GError(&err);
			fail("must be a success");
			return;
	}	
	
}
END_TEST



START_TEST(test_gfal_check_surl)
{
	GError* err=NULL;
	int ret;
	fail_unless((ret = gfal_surl_checker("srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04/file75715ccc-1c54-4d18-8824-bdd3716a2b54",&err) ) == 0, " error in url parse");
	if(ret)
		gfal_release_GError(&err);
	fail_unless((ret = gfal_surl_checker("srm://g_rid-cer_t-03.rOMa1.in_fn.it/dpm/roma1.in_fn.it/home/dteam/generated/2006-07-04/file75715ccc-1c54-4d18-8824-bdd3716a2b54",&err) ) == 0, " error in url parse 2");
	if(ret)
		gfal_release_GError(&err);
	fail_if( (ret = gfal_surl_checker("http://google.com",&err ))== 0, " must fail , bad url");
}
END_TEST


START_TEST(test_gfal_full_endpoint_check)
{
	
	fail("hello");
}
	
