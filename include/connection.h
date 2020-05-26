#include "structs.h"

#ifndef _VPND_CONNECTION
#define _VPND_CONNECTION

connection_t *create_connection(int s, bool is_client);
void destroy_connection(connection_t *conn);

#endif