//C basic headers
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

extern uint16_t m_conn_handle;

void ble_init(void);
void advertising_start(bool erase_bonds);
void delete_bonds(void);

void ble_conn_close(void);
void ble_conn_restart(void);
void ble_conn_toggle(void);