#ifndef GIZMO_RUNTIME_H
#define GIZMO_RUNTIME_H

#include "gizmo_api.h"

int gizmo_runtime_create(gizmo_instance_t *out, const char *type_id,
    gizmo_gui_context_t *gui, gizmo_core_context_t *core, int pane_id);
void gizmo_runtime_shutdown(gizmo_instance_t *gizmo);
int gizmo_runtime_replace(gizmo_instance_t *gizmo, const char *new_type_id);
int gizmo_runtime_clone_state(const gizmo_instance_t *source, gizmo_instance_t *dest);

#endif
