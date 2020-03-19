#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/** Security mode */
#define USE_WEP					 (0) /*< Set to (1) to use WEP, and (0) to use OPEN */

/** AP mode Settings */
#define MAIN_WLAN_SSID           "DEMO_AP" /* < SSID */
#if USE_WEP
#define MAIN_WLAN_AUTH           M2M_WIFI_SEC_WEP /* < Security manner */
#define MAIN_WLAN_WEP_KEY        "1234567890" /* < Security Key in WEP Mode */
#define MAIN_WLAN_WEP_KEY_INDEX  (0)
#else
#define MAIN_WLAN_AUTH           M2M_WIFI_SEC_OPEN /* < Security manner */
#endif
#define MAIN_WLAN_CHANNEL        (6) /* < Channel number */

/** Wi-Fi Settings */
#define MAIN_WLAN_SSID                    "DEMO_AP" /**< Destination SSID */
#define MAIN_WIFI_M2M_PRODUCT_NAME        "CRRC"
#define MAIN_WIFI_M2M_SERVER_IP           0xC0A80101 /* 192.168.1.1 */
#define MAIN_WIFI_M2M_SERVER_PORT         (6666)
#define MAIN_WIFI_M2M_REPORT_INTERVAL     (1000)

#define MAIN_WIFI_M2M_CLIENT_IP           0xC0A80164 /* 192.168.1.100 */
#define MAIN_WIFI_M2M_CLIENT_PORT         (4000)

#define MAIN_WIFI_M2M_BUFFER_SIZE          1460

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H_INCLUDED */