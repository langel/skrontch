#include "support/stb_image.c"
#ifdef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_IMPLEMENTATION
#endif

#include "gui/image_loader.c"
#include "gui/cursor_manager.c"
#include "gui/palette_manager.c"
#include "gui/window_manager.c"

#include "core/input_state.c"
#include "core/workspace_manager.c"
#include "core/app_state.c"

#include "gizmos/id_viewer.c"
#include "gizmos/gizmo_registry.c"
#include "gizmos/gizmo_runtime.c"

#include "main.c"
