ifeq ($(PROJ_PATH),)
include feature_debug.mk
else
include $(PROJ_PATH)/feature_debug.mk
endif

MTK_SENSOR_BIO_MODULE = SOLTEAMOPTO_VCA003


# bt module enable
MTK_BT_ENABLE                       = y
MTK_BLE_ONLY_ENABLE                 = n
MTK_BT_HFP_ENABLE                   = n
MTK_BT_AVRCP_ENABLE                 = n
MTK_BT_AVRCP_ENH_ENABLE             = n
MTK_BT_A2DP_ENABLE                  = n
MTK_BT_PBAP_ENABLE                  = n
MTK_BT_SPP_ENABLE                   = y
