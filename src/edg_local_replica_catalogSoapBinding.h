
#include "lrcH.h"
SOAP_NMAC struct Namespace namespaces_lrc[] =
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"ns1", "http://rls.data.edg.org", NULL, NULL},
	{"ns2", "http://attribute.util.data.edg.org", NULL, NULL},
	{"lrc", "urn:edg-local-replica-catalog", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
