#include <stdint.h>
#include <stdbool.h>

extern uint16_t          m_conn_handle;

void ble_init(void);
void advertising_start(bool erase_bonds);
void runtime_erase_bond(void);

void ble_conn_close(void);
void ble_conn_restart(void);
void ble_conn_toggle(void);