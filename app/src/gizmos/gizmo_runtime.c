#include "gizmo_runtime.h"

#include <string.h>

#include "gizmo_registry.h"

static void copy_string(char *dest, size_t dest_size, const char *source)
{
    if (dest == NULL || dest_size == 0) {
        return;
    }
    if (source == NULL) {
        dest[0] = '\0';
        return;
    }
    SDL_strlcpy(dest, source, dest_size);
}

int gizmo_runtime_create(gizmo_instance_t *out, const char *type_id,
    gizmo_gui_context_t *gui, gizmo_core_context_t *core, int pane_id)
{
    if (out == NULL) {
        return 0;
    }

    const gizmo_api_t *api = gizmo_registry_lookup(type_id);
    if (api == NULL) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    out->api = api;
    out->gui = gui;
    out->core = core;
    out->pane_id = pane_id;
    copy_string(out->type_id, sizeof(out->type_id), api->type_id);
    copy_string(out->display_name, sizeof(out->display_name), api->display_name);

    if (api->init != NULL && !api->init(out)) {
        if (api->shutdown != NULL) {
            api->shutdown(out);
        }
        memset(out, 0, sizeof(*out));
        return 0;
    }

    return 1;
}

void gizmo_runtime_shutdown(gizmo_instance_t *gizmo)
{
    if (gizmo == NULL) {
        return;
    }

    if (gizmo->api != NULL && gizmo->api->shutdown != NULL) {
        gizmo->api->shutdown(gizmo);
    }
    memset(gizmo, 0, sizeof(*gizmo));
}

int gizmo_runtime_replace(gizmo_instance_t *gizmo, const char *new_type_id)
{
    if (gizmo == NULL) {
        return 0;
    }

    gizmo_gui_context_t *gui = gizmo->gui;
    gizmo_core_context_t *core = gizmo->core;
    int pane_id = gizmo->pane_id;
    SDL_Rect bounds = gizmo->bounds;

    gizmo_runtime_shutdown(gizmo);
    if (!gizmo_runtime_create(gizmo, new_type_id, gui, core, pane_id)) {
        return 0;
    }
    gizmo->bounds = bounds;
    if (gizmo->api != NULL && gizmo->api->on_resize != NULL) {
        gizmo->api->on_resize(gizmo, bounds.w, bounds.h);
    }
    return 1;
}

int gizmo_runtime_clone_state(const gizmo_instance_t *source, gizmo_instance_t *dest)
{
    if (source == NULL || dest == NULL || source->api == NULL) {
        return 0;
    }
    if (source->api->clone_state == NULL) {
        return 0;
    }
    return source->api->clone_state(source, dest);
}
