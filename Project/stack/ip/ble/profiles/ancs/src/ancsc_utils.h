/**
 ****************************************************************************************
 *
 * @file ancsc_utils.h
 *
 * @brief Header file - ANCS - Client Role.
 *
 * Copyright (C) Huntersun 2018-2019
 *
 *
 ****************************************************************************************
 */

#ifndef _ANCSC_UTILS_H_
#define _ANCSC_UTILS_H_

/**
 ****************************************************************************************
 * @addtogroup ACNSC Client
 * @ingroup ANCS
 * @brief ANCS Client
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#include <stdint.h>
#include <stdbool.h>
#include "prf_utils.h"
#include "ancsc_task.h"
#include "ancs_common.h"


struct ancs_ntf_attr_rec
{
	uint32_t notif_uid;
    
	uint8_t date_len;
	uint8_t date[ANCS_DATE_CAP];
	uint8_t app_id_len;
	uint8_t app_id[ANCS_APPID_CAP];
	uint8_t title_len;
	uint8_t title[ANCS_TITLE_CAP];
	uint8_t subtitle_len;
	uint8_t subtitle[ANCS_SUBTITLE_CAP];
	uint8_t msg_len;
	uint8_t msg[ANCS_MSG_CAP];
};

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


void ancs_parse_ntf_ind(struct ancsc_notification_ind *ind, const uint8_t *value, uint16_t length);
struct ancs_ntf_attr_rec* ancs_parse_ntf_attr_ind(const uint8_t *value, uint16_t length);


#endif /* _ANCSC_UTILS_H_ */
