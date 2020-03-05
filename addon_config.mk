meta:
	ADDON_NAME = ofxOrbbecAstra
	ADDON_DESCRIPTION = OF wrapper Orbbec Astra SDK
	ADDON_AUTHOR = Matt Felsen
	ADDON_TAGS = "orbbec" "astra" "depth camera"
	ADDON_URL = https://github.com/mattfelsen/ofxOrbbecAstra

common:
	ADDON_INCLUDES  = libs/astra/include/
	ADDON_INCLUDES += src

vs:
	ADDON_LIBS  = libs/astra/lib/vs/x64/astra.lib
	ADDON_LIBS += libs/astra/lib/vs/x64/astra_core.lib
	ADDON_LIBS += libs/astra/lib/vs/x64/astra_core_api.lib
	ADDON_LIBS += libs/astra/lib/vs/x64/Plugins/astra-device-sdk.lib
	ADDON_LIBS += libs/astra/lib/vs/x64/Plugins/openni2/OpenNI2.lib
	ADDON_LIBS += libs/astra/lib/vs/x64/Plugins/openni_sensor.lib
	ADDON_LIBS += libs/astra/lib/vs/x64/Plugins/orbbec_hand.lib
	ADDON_LIBS += libs/astra/lib/vs/x64/Plugins/orbbec_xs.lib
	ADDON_LIBS += libs/astra/lib/vs/x64/Shiny-static.lib
	ADDON_DLLS_TO_COPY = copy_to_bin64_dir
