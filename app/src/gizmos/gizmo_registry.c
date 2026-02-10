#include "gizmo_registry.h"

#include <string.h>

#include "id_viewer.h"

const gizmo_api_t *gizmo_registry_lookup(const char *type_id)
{
    if (type_id == NULL) {
        return NULL;
    }

    if (strcmp(type_id, id_viewer_gizmo_api.type_id) == 0) {
        return &id_viewer_gizmo_api;
    }

    return NULL;
}
