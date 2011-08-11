## configure gfal to load the plugin
##

setenv LFC_HOST	"prod-lfc-shared-central.cern.ch"
setenv GFAL_PLUGIN_LIST "libgfal_plugin_lfc.so:$GFAL_PLUGIN_LIST"
setenv LCG_GFAL_INFOSYS "lcg-bdii.cern.ch:2170"

