//C basic headers
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

extern uint16_t m_conn_handle;

void ble_init(void);
void advertising_start(bool erase_bonds);
void delete_bonds(void);