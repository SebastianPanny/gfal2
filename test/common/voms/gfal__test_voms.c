/* unit test for common_errverbose */


#include <cgreen/cgreen.h>

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include "voms/gfal_voms_if.h"
#include "../../mock/gfal_voms_mock_test.h"
#include "unit_test_constants.h"

#define VO_TEST	"dteam"
#define DN_TEST "/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=adevress/CN=714425/CN=Adrien Devresse"
#define FQAN_TEST "/dteam"



void voms_mock_init(){
	gfal_voms_destroy();
#if USE_MOCK
	gfal_voms_external.VOMS_Init = &voms_mock_VOMS_Init;
	gfal_voms_external.VOMS_Destroy = &voms_mock_VOMS_Destroy;
	gfal_voms_external.VOMS_ErrorMessage = &voms_mock_VOMS_ErrorMessage;
	gfal_voms_external.VOMS_SetVerificationType = &voms_mock_VOMS_SetVerificationType;
	gfal_voms_external.VOMS_RetrieveFromProxy = &voms_mock_VOMS_RetrieveFromProxy;
#endif
}

void voms_mock_configure(){
#if USE_MOCK
	g_strlcpy(defined_Error_Message, "Voms mock internal error", 2048);
	always_return(voms_mock_VOMS_ErrorMessage, 1);
	will_respond(voms_mock_VOMS_Init, 1, want_string(voms, ""), want_string(cert, ""));
	will_respond(voms_mock_VOMS_SetVerificationType, 1, want_non_null(vd));
	will_respond(voms_mock_VOMS_RetrieveFromProxy, 1, want_non_null(vd));
	defined_vd.data = calloc(sizeof(struct voms*),2);
	defined_vd.data[0] = calloc(sizeof(struct voms),1);
	defined_vd.data[0]->fqan= calloc(sizeof(char*), 3);
	defined_vd.data[0]->fqan[0]= strdup(FQAN_TEST);	
	defined_vd.data[0]->fqan[1]= strdup(FQAN_TEST);
	defined_vd.data[0]->user= strdup(DN_TEST);	
	defined_vd.data[0]->voname = strdup(VO_TEST);	
	will_respond(voms_mock_VOMS_Destroy, 0, want_non_null(vd));
	
#endif	
	
}


void voms_mock_configure_all(){
#if USE_MOCK
	g_strlcpy(defined_Error_Message, "Voms mock internal error", 2048);
	always_return(voms_mock_VOMS_ErrorMessage, 1);
	always_return(voms_mock_VOMS_Init, 1);
	always_return(voms_mock_VOMS_SetVerificationType, 1);
	always_return(voms_mock_VOMS_RetrieveFromProxy, 1);
	defined_vd.data = calloc(sizeof(struct voms*),2);
	defined_vd.data[0] = calloc(sizeof(struct voms),1);
	defined_vd.data[0]->fqan= calloc(sizeof(char*), 3);
	defined_vd.data[0]->fqan[0]= strdup(FQAN_TEST);	
	defined_vd.data[0]->fqan[1]= strdup(FQAN_TEST);
	defined_vd.data[0]->user= strdup(DN_TEST);	
	defined_vd.data[0]->voname = strdup(VO_TEST);	
	always_return(voms_mock_VOMS_Destroy, 0);
	
#endif	
	
}




// check if initiated properly
// need a voms proxy environnement
void test_voms_parse_args()
{
	GError* err=NULL;
	voms_mock_init();
	voms_mock_configure();
	int r =gfal_parse_vomsdataG(&err);

	assert_true_with_message(r==0 && err == NULL,  "return must be null if success\n");
	gfal_check_GError(&err);	
}


// check if  same resutl after 2 times 
void test_voms_info_is_null()
{
	GError* err=NULL;
	voms_mock_init();
	voms_mock_configure();
	int r =gfal_parse_vomsdataG(&err);
	assert_true_with_message(r==0 && err==NULL, "return must be null if success\n");
	if(gfal_check_GError(&err))
		return;
	r =gfal_parse_vomsdataG(&err);
	assert_true_with_message(r==0 && err==NULL, "return must be null if success\n");
	if(gfal_check_GError(&err))
		return;

	
}



 
void test_voms_info_test_vo()
{
	GError* err=NULL;
	voms_mock_init();
	voms_mock_configure();
	char* ret =gfal_get_voG(&err);
	assert_true_with_message(ret!=NULL && err==NULL,  "return must be not null if success\n");
	if(gfal_check_GError(&err))
		return;

	assert_true_with_message(strings_are_equal(ret, VO_TEST), " must be the defined vo");	
	free(ret);
}




void test_voms_get_userdnG()
{
	GError* err=NULL;
	voms_mock_init();
	voms_mock_configure();
	char* ret =gfal_get_userdnG(&err);
	assert_true_with_message(ret != NULL && err ==NULL, " return must be a valid string");
	if( gfal_check_GError(&err))
		return;

	assert_true_with_message(strings_are_equal(ret, DN_TEST), " must be the defined vo");
	free(ret);
}




void test_voms_get_fqan()
{
	GError* err=NULL;
	voms_mock_init();
	voms_mock_configure();
	GList* ret =gfal_get_fqanG(&err);
	assert_true_with_message(ret != NULL && err==NULL, "return must be not null if success");
	if(gfal_check_GError(&err))
		return;
	int i;
	const int len = g_list_length (ret);
	for(i=0; i < len; i++){
		char* str= (char*) ret->data;
		assert_true_with_message(  strstr(str, FQAN_TEST) != NULL, " must contain the vo");
		ret = g_list_next(ret);
	}
}
