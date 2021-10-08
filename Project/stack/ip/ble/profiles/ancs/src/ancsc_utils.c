#include "ancsc_utils.h"
#include "ke_timer.h"
#include "co_utils.h"

#define ANCS_ENC_UINT8(p,u8)    {*(p)++ = (uint8_t)(u8);}
#define ANCS_ENC_UINT16(p,u16)  {*(p)++ = (uint8_t)(u16); *(p)++ = (uint8_t)((u16) >> 8);}
#define ANCS_ENC_UINT32(p,u32)  {*(p)++ = (uint8_t)(u32); *(p)++ = (uint8_t)((u32) >> 8); *(p)++ = (uint8_t)((u32) >> 16); *(p)++ = (uint8_t)((u32) >> 24);}
#define ANCS_DEC_UINT32(buf)    ( ((uint32_t) ((uint8_t *)(buf))[0]) | (((uint32_t)((uint8_t *)(buf))[1]) << 8) | (((uint32_t)((uint8_t *)(buf))[2]) << 16) | (((uint32_t) ((uint8_t *)(buf))[3])) << 24)

static uint8_t  m_parse;
static struct ancs_ntf_attr_rec m_ancs_data;

enum {
    PARSE_ST_IDLE,
    PARSE_ST_HEADER,
    PARSE_ST_ITEMS,
    PARSE_ST_SUCCESS,
    PARSE_ST_FAIL,
};

void ancs_parse_ntf_ind(struct ancsc_notification_ind *ind, const uint8_t *value, uint16_t length)
{
    ASSERT_ERR(length == sizeof(struct ancsc_notification_ind));
	ind->event_id = *value++;
	ind->event_flags = *value++;
	ind->category_id = *value++;
	ind->category_count = *value++;
	ind->notification_uid = *(uint32_t *)value;
}

static uint8_t get_max_item_len(uint8_t attr)
{
    switch (attr) {
        case ANCS_NTF_ATTR_ID_APP_IDENTIFIER: return ANCS_APPID_CAP;
        case ANCS_NTF_ATTR_ID_TITLE:          return ANCS_TITLE_CAP;
        case ANCS_NTF_ATTR_ID_SUBTITLE:       return ANCS_SUBTITLE_CAP;
        case ANCS_NTF_ATTR_ID_MESSAGE:        return ANCS_MSG_CAP;
        case ANCS_NTF_ATTR_ID_DATE:           return ANCS_DATE_CAP;
        default:                              return 0;
    }
}

static uint8_t* get_buf(struct ancs_ntf_attr_rec* data, uint8_t attr)
{
    switch (attr) {
        case ANCS_NTF_ATTR_ID_APP_IDENTIFIER: return data->app_id;
        case ANCS_NTF_ATTR_ID_TITLE:          return data->title;
        case ANCS_NTF_ATTR_ID_SUBTITLE:       return data->subtitle;
        case ANCS_NTF_ATTR_ID_MESSAGE:        return data->msg;
        case ANCS_NTF_ATTR_ID_DATE:           return data->date;
        default:                              return NULL;
    }
}

struct ancs_ntf_attr_rec* ancs_parse_ntf_attr_ind(const uint8_t *value, uint16_t len)
{
    static uint8_t header[3];
    static uint8_t header_len;
    static uint8_t item_recv_len;
    uint8_t i = 0; // index of value
    while (len > 0) {
        if (m_parse == PARSE_ST_IDLE) {
            memset(&m_ancs_data, 0, sizeof(struct ancs_ntf_attr_rec));
            if (len < 5 || *value != ANCS_COMMAND_ID_GET_NTF_ATTR) { // format error
                m_parse = PARSE_ST_IDLE;
                memset(&m_ancs_data, 0, sizeof(struct ancs_ntf_attr_rec));
                return &m_ancs_data;
            }
            i++; len--; // skip id
            m_ancs_data.notif_uid = ANCS_DEC_UINT32(&value[i]); // decode uid
            i += 4; len -= 4;
            header_len = 0;
            m_parse = PARSE_ST_HEADER;
        }
        if (m_parse == PARSE_ST_HEADER) {
            if (header_len + len  < 3) {
                memcpy(&header[header_len], &value[i], len);
                header_len += len;
                i += len;
                return NULL; // more data needed
            } else {
                memcpy(&header[header_len], &value[i], 3 - header_len);
                i += 3 - header_len; len -= 3 - header_len;
                header_len += 3 - header_len;
                uint16_t item_len = header[1] + (header[2] << 8);
                if (item_len == 0) { // length of attribute is 0, nothing to parse
                    header_len = 0;
                } else if (header[0] != ANCS_NTF_ATTR_ID_APP_IDENTIFIER
                           && get_max_item_len(header[0]) < item_len) { // length exceed capability
                    m_parse = PARSE_ST_IDLE;
                    memset(&m_ancs_data, 0, sizeof(struct ancs_ntf_attr_rec));
                    return &m_ancs_data;
                } else {
                    uint8_t* p = get_buf(&m_ancs_data, header[0]);
                    if (!p) { // error: attr not support
                        m_parse = PARSE_ST_IDLE;
                        memset(&m_ancs_data, 0, sizeof(struct ancs_ntf_attr_rec));
                        return &m_ancs_data;
                    } else {
                        *(p - 1) = 0; // reset length of attribute
                        item_recv_len = 0;
                        m_parse = PARSE_ST_ITEMS;
                    }
                }
            }
        }
        if (m_parse == PARSE_ST_ITEMS) {
            uint8_t* p = get_buf(&m_ancs_data, header[0]);
            if (header_len == 0 || !p) {
                m_parse = PARSE_ST_IDLE;
                memset(&m_ancs_data, 0, sizeof(struct ancs_ntf_attr_rec));
                return &m_ancs_data;
            }
            uint8_t* recv_len = p - 1;
            uint16_t item_len = header[1] + (header[2] << 8);

            uint8_t max_item_len = get_max_item_len(header[0]);
            uint8_t parse_len = item_len - *recv_len;
            parse_len  = parse_len < len ? parse_len : len;
            uint8_t copy_len = parse_len < (max_item_len - *recv_len) ? parse_len : (max_item_len - *recv_len);
            memcpy(&p[*recv_len], &value[i], copy_len);
            i += parse_len; len -= parse_len;
            *recv_len += copy_len;
            item_recv_len += parse_len;
            if (item_recv_len == item_len) {
                if (m_ancs_data.msg_len == item_len) {
                    m_parse = PARSE_ST_IDLE;
                    return &m_ancs_data;
                } else {
                    header_len = 0;
                    m_parse = PARSE_ST_HEADER;
                }
            }
        }
    }
    return NULL;
}


