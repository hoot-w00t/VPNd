#include "connection.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

// allocate a connection_t to use with peers
connection_t *create_connection(int s, bool is_client)
{
    connection_t *conn = malloc(sizeof(connection_t));

    if (!conn)
        return NULL;
    conn->s = s;
    conn->is_client = is_client;
    return conn;
}

// free connection_t
void destroy_connection(connection_t *conn)
{
    free(conn);
}