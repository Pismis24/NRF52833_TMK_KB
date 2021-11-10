#pragma once

#include "report.h"
#include "app_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd_hid_generic.h"

#ifdef EXTRAKEY_ENABLE
#define APP_USBD_EXTRA_HID_REPORT_DSC()       \
    {                                          \
        0x05, 0x01,        /* Usage Page (Generic Desktop Ctrls)*/ \
        0x09, 0x80,        /* Usage (Sys Control)*/ \
        0xA1, 0x01,        /* Collection (Application)*/ \
        0x85, 0x02,        /*   Report ID (2)*/ \
        0x15, 0x01,        /*   Logical Minimum (1)*/ \
        0x26, 0xB7, 0x00,  /*   Logical Maximum (183)*/ \
        0x19, 0x01,        /*   Usage Minimum (Pointer)*/ \
        0x29, 0xB7,        /*   Usage Maximum (Sys Display LCD Autoscale)*/ \
        0x75, 0x10,        /*   Report Size (16)*/ \
        0x95, 0x01,        /*   Report Count (1)*/ \
        0x81, 0x00,        /*   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)*/ \
        0xC0,              /* End Collection*/ \
        0x05, 0x0C,        /* Usage Page (Consumer)*/ \
        0x09, 0x01,        /* Usage (Consumer Control)*/ \
        0xA1, 0x01,        /* Collection (Application)*/ \
        0x85, 0x03,        /*   Report ID (3)*/ \
        0x15, 0x01,        /*   Logical Minimum (1)*/ \
        0x26, 0x9C, 0x02,  /*   Logical Maximum (668)*/ \
        0x19, 0x01,        /*   Usage Minimum (Consumer Control)*/ \
        0x2A, 0x9C, 0x02,  /*   Usage Maximum (AC Distribute Vertically)*/ \
        0x75, 0x10,        /*   Report Size (16)*/ \
        0x95, 0x01,        /*   Report Count (1)*/ \
        0x81, 0x00,        /*   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)*/ \
        0xC0,              /* End Collection*/ \
    }
APP_USBD_HID_GENERIC_SUBCLASS_REPORT_DESC(extrakey_desc, APP_USBD_EXTRA_HID_REPORT_DSC());
#endif

#ifdef MOUSEKEY_ENABLE
#define APP_USBD_HID_MOUSE_REPORT_DSC(bcnt)                            \
    {                                                                 \
        0x05, 0x01,        /* Usage Page (Generic Desktop Ctrls)*/ \
        0x09, 0x02,        /* Usage (Mouse)*/ \
        0xA1, 0x01,        /* Collection (Application)*/ \
        0x09, 0x01,        /*   Usage (Pointer)*/ \
        0xA1, 0x00,        /*   Collection (Physical)*/ \
        0x05, 0x09,        /*     Usage Page (Button)*/ \
        0x19, 0x01,        /*     Usage Minimum (0x01)*/ \
        0x29, bcnt,        /*     Usage Maximum (bcnt)*/ \
        0x15, 0x00,        /*     Logical Minimum (0)*/ \
        0x25, 0x01,        /*     Logical Maximum (1)*/ \
        0x75, 0x01,        /*     Report Size (1)*/ \
        0x95, bcnt,        /*     Report Count (bcnt)*/ \
        0x81, 0x02,        /*     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)*/ \
        0x75, (8 - bcnt),        /*     Report Size (8 - bcnt)*/ \
        0x95, 0x01,        /*     Report Count (1)*/ \
        0x81, 0x03,        /*     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)*/ \
        0x05, 0x01,        /*     Usage Page (Generic Desktop Ctrls)*/ \
        0x09, 0x30,        /*     Usage (X)*/ \
        0x09, 0x31,        /*     Usage (Y)*/ \
        0x15, 0x81,        /*     Logical Minimum (-127)*/ \
        0x25, 0x7F,        /*     Logical Maximum (127)*/ \
        0x75, 0x08,        /*     Report Size (8)*/ \
        0x95, 0x02,        /*     Report Count (2)*/ \
        0x81, 0x06,        /*     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)*/ \
        0x09, 0x38,        /*     Usage (Wheel)*/ \
        0x15, 0x81,        /*     Logical Minimum (-127)*/ \
        0x25, 0x7F,        /*     Logical Maximum (127)*/ \
        0x35, 0x00,        /*     Physical Minimum (0)*/ \
        0x45, 0x00,        /*     Physical Maximum (0)*/ \
        0x75, 0x08,        /*     Report Size (8)*/ \
        0x95, 0x01,        /*     Report Count (1)*/ \
        0x81, 0x06,        /*     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)*/ \
        0x05, 0x0C,        /*     Usage Page (Consumer)*/ \
        0x0A, 0x38, 0x02,  /*     Usage (AC Pan)*/ \
        0x15, 0x81,        /*     Logical Minimum (-127)*/ \
        0x25, 0x7F,        /*     Logical Maximum (127)*/ \
        0x75, 0x08,        /*     Report Size (8)*/ \
        0x95, 0x01,        /*     Report Count (1)*/ \
        0x81, 0x06,        /*     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)*/ \
        0xC0,              /*   End Collection*/ \
        0xC0,              /* End Collection*/ \
    }
APP_USBD_HID_GENERIC_SUBCLASS_REPORT_DESC(mouse_desc, APP_USBD_HID_MOUSE_REPORT_DSC(5));
#endif

#define APP_USBD_HID_KBD_REPORT_DSC()                                        \
    {                                                                        \
        0x05, 0x01,     /* USAGE_PAGE (Generic Desktop)                   */ \
            0x09, 0x06, /* USAGE (Keyboard)                               */ \
            0xa1, 0x01, /* COLLECTION (Application)                       */ \
            0x05, 0x07, /*   USAGE_PAGE (Keyboard)                        */ \
            0x19, 0xe0, /*   USAGE_MINIMUM (Keyboard LeftControl)         */ \
            0x29, 0xe7, /*   USAGE_MAXIMUM (Keyboard Right GUI)           */ \
            0x15, 0x00, /*   LOGICAL_MINIMUM (0)                          */ \
            0x25, 0x01, /*   LOGICAL_MAXIMUM (1)                          */ \
            0x75, 0x01, /*   REPORT_SIZE (1)                              */ \
            0x95, 0x08, /*   REPORT_COUNT (8)                             */ \
            0x81, 0x02, /*   INPUT (Data,Var,Abs)                         */ \
            0x95, 0x01, /*   REPORT_COUNT (1)                             */ \
            0x75, 0x08, /*   REPORT_SIZE (8)                              */ \
            0x81, 0x03, /*   INPUT (Cnst,Var,Abs)                         */ \
            0x95, 0x05, /*   REPORT_COUNT (5)                             */ \
            0x75, 0x01, /*   REPORT_SIZE (1)                              */ \
            0x05, 0x08, /*   USAGE_PAGE (LEDs)                            */ \
            0x19, 0x01, /*   USAGE_MINIMUM (Num Lock)                     */ \
            0x29, 0x05, /*   USAGE_MAXIMUM (Kana)                         */ \
            0x91, 0x02, /*   OUTPUT (Data,Var,Abs)                        */ \
            0x95, 0x01, /*   REPORT_COUNT (1)                             */ \
            0x75, 0x03, /*   REPORT_SIZE (3)                              */ \
            0x91, 0x03, /*   OUTPUT (Cnst,Var,Abs)                        */ \
            0x95, 0x06, /*   REPORT_COUNT (6)                             */ \
            0x75, 0x08, /*   REPORT_SIZE (8)                              */ \
            0x15, 0x00, /*   LOGICAL_MINIMUM (0)                          */ \
            0x25, 0x65, /*   LOGICAL_MAXIMUM (101)                        */ \
            0x05, 0x07, /*   USAGE_PAGE (Keyboard)                        */ \
            0x19, 0x00, /*   USAGE_MINIMUM (Reserved (no event indicated))*/ \
            0x29, 0x65, /*   USAGE_MAXIMUM (Keyboard Application)         */ \
            0x81, 0x00, /*   INPUT (Data,Ary,Abs)                         */ \
            0xc0        /* END_COLLECTION                                 */ \
    }
APP_USBD_HID_GENERIC_SUBCLASS_REPORT_DESC(kbd_desc, APP_USBD_HID_KBD_REPORT_DSC());