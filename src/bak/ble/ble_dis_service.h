#include <stdint.h>
#include <string.h>

//Device Discription
#define MANUFACTURER_NAME                   "NordicSemiconductor"                      /**< Manufacturer. Will be passed to Device Information Service. */

#define PNP_ID_VENDOR_ID_SOURCE             0x02                                       /**< Vendor ID Source. */
#define PNP_ID_VENDOR_ID                    0x1915                                     /**< Vendor ID. */
#define PNP_ID_PRODUCT_ID                   0xEEEE                                     /**< Product ID. */
#define PNP_ID_PRODUCT_VERSION              0x0001                                     /**< Product Version. */

//DIS service function
void dis_service_init(void);