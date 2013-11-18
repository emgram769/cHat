#include "network.h"

void initialize_network(void *network_settings){
    int port = ((struct network_data *)network_settings)->port;
    (void) port; /* for now */
    free(network_settings);
}


