## configure gfal to load the plugin
##

export LFC_HOST=prod-lfc-shared-central.cern.ch
export LCG_GFAL_INFOSYS=lcg-bdii.cern.ch:2170
export GFAL_PLUGIN_LIST=libgfal_plugin_lfc.so:$GFAL_PLUGIN_LIST


