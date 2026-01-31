#include "workspace_manager.h"

#include "app_state.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

enum {
    DEFAULT_DEBOUNCE_MS = 150
};

static int ensure_directory(const char *path)
{
#ifdef _WIN32
    if (_mkdir(path) == 0 || errno == EEXIST) {
        return 1;
    }
#else
    if (mkdir(path, 0755) == 0 || errno == EEXIST) {
        return 1;
    }
#endif
    return 0;
}

static int workspace_build_path(workspace_manager_t *workspace, int ensure_dirs)
{
    if (workspace == NULL) {
        return 0;
    }

    char *base_path = SDL_GetBasePath();
    const char *root = base_path != NULL ? base_path : ".";

    if (ensure_dirs) {
        char disk_path[512];
        char workspace_dir[512];
        snprintf(disk_path, sizeof(disk_path), "%sdisk", root);
        snprintf(workspace_dir, sizeof(workspace_dir), "%sdisk/workspace", root);
        if (!ensure_directory(disk_path) || !ensure_directory(workspace_dir)) {
            if (base_path != NULL) {
                SDL_free(base_path);
            }
            return 0;
        }
    }

    snprintf(workspace->workspace_path, sizeof(workspace->workspace_path),
        "%sdisk/workspace/workspace.json", root);

    if (base_path != NULL) {
        SDL_free(base_path);
    }

    return 1;
}

static const char *json_skip_ws(const char *cursor)
{
    while (cursor != NULL && *cursor != '\0' && isspace((unsigned char)*cursor)) {
        cursor++;
    }
    return cursor;
}

static int json_expect(const char **cursor, char expected)
{
    const char *pos = json_skip_ws(*cursor);
    if (pos == NULL || *pos != expected) {
        return 0;
    }
    *cursor = pos + 1;
    return 1;
}

static int json_parse_string(const char **cursor, char *output, size_t output_size)
{
    const char *pos = json_skip_ws(*cursor);
    if (pos == NULL || *pos != '"') {
        return 0;
    }
    pos++;

    size_t written = 0;
    while (*pos != '\0' && *pos != '"') {
        char ch = *pos++;
        if (ch == '\\' && *pos != '\0') {
            ch = *pos++;
        }
        if (written + 1 < output_size) {
            output[written++] = ch;
        }
    }
    if (*pos != '"') {
        return 0;
    }
    pos++;
    if (written < output_size) {
        output[written] = '\0';
    }
    *cursor = pos;
    return 1;
}

static int json_parse_number(const char **cursor, double *value)
{
    const char *pos = json_skip_ws(*cursor);
    if (pos == NULL) {
        return 0;
    }

    char *end = NULL;
    double parsed = strtod(pos, &end);
    if (end == pos) {
        return 0;
    }
    if (value != NULL) {
        *value = parsed;
    }
    *cursor = end;
    return 1;
}

static int json_skip_value(const char **cursor);

static int json_skip_array(const char **cursor)
{
    if (!json_expect(cursor, '[')) {
        return 0;
    }

    const char *pos = json_skip_ws(*cursor);
    if (pos != NULL && *pos == ']') {
        *cursor = pos + 1;
        return 1;
    }

    while (1) {
        if (!json_skip_value(cursor)) {
            return 0;
        }
        pos = json_skip_ws(*cursor);
        if (pos == NULL) {
            return 0;
        }
        if (*pos == ',') {
            *cursor = pos + 1;
            continue;
        }
        if (*pos == ']') {
            *cursor = pos + 1;
            return 1;
        }
        return 0;
    }
}

static int json_skip_object(const char **cursor)
{
    if (!json_expect(cursor, '{')) {
        return 0;
    }

    const char *pos = json_skip_ws(*cursor);
    if (pos != NULL && *pos == '}') {
        *cursor = pos + 1;
        return 1;
    }

    while (1) {
        char key[64];
        if (!json_parse_string(cursor, key, sizeof(key))) {
            return 0;
        }
        if (!json_expect(cursor, ':')) {
            return 0;
        }
        if (!json_skip_value(cursor)) {
            return 0;
        }
        pos = json_skip_ws(*cursor);
        if (pos == NULL) {
            return 0;
        }
        if (*pos == ',') {
            *cursor = pos + 1;
            continue;
        }
        if (*pos == '}') {
            *cursor = pos + 1;
            return 1;
        }
        return 0;
    }
}

static int json_skip_value(const char **cursor)
{
    const char *pos = json_skip_ws(*cursor);
    if (pos == NULL) {
        return 0;
    }

    if (*pos == '"') {
        char buffer[8];
        return json_parse_string(cursor, buffer, sizeof(buffer));
    }
    if (*pos == '{') {
        return json_skip_object(cursor);
    }
    if (*pos == '[') {
        return json_skip_array(cursor);
    }
    if (*pos == 't' && strncmp(pos, "true", 4) == 0) {
        *cursor = pos + 4;
        return 1;
    }
    if (*pos == 'f' && strncmp(pos, "false", 5) == 0) {
        *cursor = pos + 5;
        return 1;
    }
    if (*pos == 'n' && strncmp(pos, "null", 4) == 0) {
        *cursor = pos + 4;
        return 1;
    }

    return json_parse_number(cursor, NULL);
}

static int parse_split_node(const char **cursor, split_node_t *node)
{
    if (node == NULL || !json_expect(cursor, '{')) {
        return 0;
    }

    split_node_t parsed = { 0 };
    parsed.first = -1;
    parsed.second = -1;
    parsed.parent = -1;

    while (1) {
        char key[64];
        if (!json_parse_string(cursor, key, sizeof(key))) {
            return 0;
        }
        if (!json_expect(cursor, ':')) {
            return 0;
        }

        double number = 0.0;
        if (strcmp(key, "is_leaf") == 0) {
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            parsed.is_leaf = (int)number;
        } else if (strcmp(key, "pane_id") == 0) {
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            parsed.pane_id = (int)number;
        } else if (strcmp(key, "orientation") == 0) {
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            parsed.orientation = (split_orientation_t)number;
        } else if (strcmp(key, "ratio") == 0) {
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            parsed.ratio = (float)number;
        } else if (strcmp(key, "first") == 0) {
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            parsed.first = (int)number;
        } else if (strcmp(key, "second") == 0) {
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            parsed.second = (int)number;
        } else if (strcmp(key, "parent") == 0) {
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            parsed.parent = (int)number;
        } else {
            if (!json_skip_value(cursor)) {
                return 0;
            }
        }

        const char *pos = json_skip_ws(*cursor);
        if (pos == NULL) {
            return 0;
        }
        if (*pos == ',') {
            *cursor = pos + 1;
            continue;
        }
        if (*pos == '}') {
            *cursor = pos + 1;
            *node = parsed;
            return 1;
        }
        return 0;
    }
}

static int tab_state_is_valid(const tab_state_t *tab)
{
    if (tab == NULL || tab->node_count <= 0 || tab->node_count > MAX_SPLIT_NODES) {
        return 0;
    }
    if (tab->root_node < 0 || tab->root_node >= tab->node_count) {
        return 0;
    }
    for (int i = 0; i < tab->node_count; ++i) {
        const split_node_t *node = &tab->nodes[i];
        if (node->first < -1 || node->first >= tab->node_count) {
            return 0;
        }
        if (node->second < -1 || node->second >= tab->node_count) {
            return 0;
        }
        if (node->parent < -1 || node->parent >= tab->node_count) {
            return 0;
        }
    }
    return 1;
}

static int parse_tab(const char **cursor, tab_state_t *tab)
{
    if (tab == NULL || !json_expect(cursor, '{')) {
        return 0;
    }

    tab_state_init_default(tab);

    while (1) {
        char key[64];
        if (!json_parse_string(cursor, key, sizeof(key))) {
            return 0;
        }
        if (!json_expect(cursor, ':')) {
            return 0;
        }

        if (strcmp(key, "focused_pane") == 0) {
            double number = 0.0;
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            tab->focused_pane_node = (int)number;
        } else if (strcmp(key, "root_node") == 0) {
            double number = 0.0;
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            tab->root_node = (int)number;
        } else if (strcmp(key, "nodes") == 0) {
            if (!json_expect(cursor, '[')) {
                return 0;
            }
            tab->node_count = 0;
            const char *pos = json_skip_ws(*cursor);
            if (pos != NULL && *pos == ']') {
                *cursor = pos + 1;
            } else {
                while (1) {
                    if (tab->node_count >= MAX_SPLIT_NODES) {
                        return 0;
                    }
                    if (!parse_split_node(cursor, &tab->nodes[tab->node_count])) {
                        return 0;
                    }
                    tab->node_count += 1;
                    pos = json_skip_ws(*cursor);
                    if (pos == NULL) {
                        return 0;
                    }
                    if (*pos == ',') {
                        *cursor = pos + 1;
                        continue;
                    }
                    if (*pos == ']') {
                        *cursor = pos + 1;
                        break;
                    }
                    return 0;
                }
            }
            if (tab->node_count == 0) {
                tab_state_init_default(tab);
            }
        } else {
            if (!json_skip_value(cursor)) {
                return 0;
            }
        }

        const char *pos = json_skip_ws(*cursor);
        if (pos == NULL) {
            return 0;
        }
        if (*pos == ',') {
            *cursor = pos + 1;
            continue;
        }
        if (*pos == '}') {
            *cursor = pos + 1;
            if (!tab_state_is_valid(tab)) {
                tab_state_init_default(tab);
            }
            return 1;
        }
        return 0;
    }
}

static int parse_tabs_array(const char **cursor, tab_state_t *tabs, int *tab_count)
{
    if (!json_expect(cursor, '[')) {
        return 0;
    }

    int count = 0;
    const char *pos = json_skip_ws(*cursor);
    if (pos != NULL && *pos == ']') {
        *cursor = pos + 1;
        *tab_count = 0;
        return 1;
    }

    while (1) {
        if (count >= MAX_TABS) {
            return 0;
        }
        if (!parse_tab(cursor, &tabs[count])) {
            return 0;
        }
        count += 1;
        pos = json_skip_ws(*cursor);
        if (pos == NULL) {
            return 0;
        }
        if (*pos == ',') {
            *cursor = pos + 1;
            continue;
        }
        if (*pos == ']') {
            *cursor = pos + 1;
            break;
        }
        return 0;
    }

    *tab_count = count;
    return 1;
}

static int window_position_offscreen(int x, int y, int w, int h)
{
    SDL_Rect display = { 0, 0, 0, 0 };
    if (SDL_GetDisplayBounds(0, &display) != 0) {
        return 0;
    }

    int right = x + w;
    int bottom = y + h;
    int margin = 50;
    if (right < display.x + margin) {
        return 1;
    }
    if (x > display.x + display.w - margin) {
        return 1;
    }
    if (bottom < display.y + margin) {
        return 1;
    }
    if (y > display.y + display.h - margin) {
        return 1;
    }
    return 0;
}

static int parse_window_object(const char **cursor, tab_state_t *tabs, int *tab_count,
    int *active_tab, int *x, int *y, int *w, int *h, int *use_position)
{
    if (!json_expect(cursor, '{')) {
        return 0;
    }

    int found_position = 0;
    int width = 1200;
    int height = 800;
    int pos_x = 0;
    int pos_y = 0;
    int active = 0;
    int count = 0;

    while (1) {
        char key[64];
        if (!json_parse_string(cursor, key, sizeof(key))) {
            return 0;
        }
        if (!json_expect(cursor, ':')) {
            return 0;
        }

        double number = 0.0;
        if (strcmp(key, "x") == 0) {
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            pos_x = (int)number;
            found_position = 1;
        } else if (strcmp(key, "y") == 0) {
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            pos_y = (int)number;
            found_position = 1;
        } else if (strcmp(key, "w") == 0) {
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            width = (int)number;
        } else if (strcmp(key, "h") == 0) {
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            height = (int)number;
        } else if (strcmp(key, "active_tab") == 0) {
            if (!json_parse_number(cursor, &number)) {
                return 0;
            }
            active = (int)number;
        } else if (strcmp(key, "tabs") == 0) {
            if (!parse_tabs_array(cursor, tabs, &count)) {
                return 0;
            }
        } else {
            if (!json_skip_value(cursor)) {
                return 0;
            }
        }

        const char *pos = json_skip_ws(*cursor);
        if (pos == NULL) {
            return 0;
        }
        if (*pos == ',') {
            *cursor = pos + 1;
            continue;
        }
        if (*pos == '}') {
            *cursor = pos + 1;
            break;
        }
        return 0;
    }

    if (count <= 0) {
        tab_state_init_default(&tabs[0]);
        count = 1;
    }

    if (active < 0 || active >= count) {
        active = 0;
    }

    if (found_position && window_position_offscreen(pos_x, pos_y, width, height)) {
        found_position = 0;
    }

    *tab_count = count;
    *active_tab = active;
    *x = pos_x;
    *y = pos_y;
    *w = width;
    *h = height;
    *use_position = found_position;
    return 1;
}

static int parse_windows(const char **cursor, app_state_t *app, const char *title,
    int default_width, int default_height)
{
    if (!json_expect(cursor, '[')) {
        return 0;
    }

    const char *pos = json_skip_ws(*cursor);
    if (pos != NULL && *pos == ']') {
        *cursor = pos + 1;
        return 0;
    }

    while (1) {
        tab_state_t tabs[MAX_TABS];
        int tab_count = 0;
        int active_tab = 0;
        int x = 0;
        int y = 0;
        int w = default_width;
        int h = default_height;
        int use_position = 0;

        if (!parse_window_object(cursor, tabs, &tab_count, &active_tab,
            &x, &y, &w, &h, &use_position)) {
            return 0;
        }

        app_state_add_window(app, title, w, h, x, y, use_position, tabs, tab_count, active_tab);

        pos = json_skip_ws(*cursor);
        if (pos == NULL) {
            return 0;
        }
        if (*pos == ',') {
            *cursor = pos + 1;
            continue;
        }
        if (*pos == ']') {
            *cursor = pos + 1;
            return app->window_count > 0;
        }
        return 0;
    }
}

static int workspace_load_file(const char *path, char **buffer_out, size_t *size_out)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }
    long length = ftell(file);
    if (length < 0) {
        fclose(file);
        return 0;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }

    char *buffer = (char *)malloc((size_t)length + 1);
    if (buffer == NULL) {
        fclose(file);
        return 0;
    }

    size_t read_size = fread(buffer, 1, (size_t)length, file);
    fclose(file);
    buffer[read_size] = '\0';

    *buffer_out = buffer;
    if (size_out != NULL) {
        *size_out = read_size;
    }
    return 1;
}

static int workspace_save_file(const char *path, const app_state_t *app)
{
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        return 0;
    }

    fprintf(file, "{\n");
    fprintf(file, "  \"windows\": [\n");
    for (int i = 0; i < app->window_count; ++i) {
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;
        window_manager_get_window_rect((window_state_t *)&app->windows[i], &x, &y, &w, &h);

        fprintf(file, "    {\n");
        fprintf(file, "      \"x\": %d,\n", x);
        fprintf(file, "      \"y\": %d,\n", y);
        fprintf(file, "      \"w\": %d,\n", w);
        fprintf(file, "      \"h\": %d,\n", h);
        fprintf(file, "      \"active_tab\": %d,\n", app->windows[i].active_tab);
        fprintf(file, "      \"tabs\": [\n");

        for (int t = 0; t < app->windows[i].tab_count; ++t) {
            const tab_state_t *tab = &app->windows[i].tabs[t];
            fprintf(file, "        {\n");
            fprintf(file, "          \"focused_pane\": %d,\n", tab->focused_pane_node);
            fprintf(file, "          \"root_node\": %d,\n", tab->root_node);
            fprintf(file, "          \"node_count\": %d,\n", tab->node_count);
            fprintf(file, "          \"nodes\": [\n");

            for (int n = 0; n < tab->node_count; ++n) {
                const split_node_t *node = &tab->nodes[n];
                fprintf(file,
                    "            {\"is_leaf\": %d, \"pane_id\": %d, \"orientation\": %d, "
                    "\"ratio\": %.4f, \"first\": %d, \"second\": %d, \"parent\": %d}%s\n",
                    node->is_leaf,
                    node->pane_id,
                    (int)node->orientation,
                    node->ratio,
                    node->first,
                    node->second,
                    node->parent,
                    (n + 1 < tab->node_count) ? "," : ""
                );
            }

            fprintf(file, "          ]\n");
            fprintf(file, "        }%s\n", (t + 1 < app->windows[i].tab_count) ? "," : "");
        }

        fprintf(file, "      ]\n");
        fprintf(file, "    }%s\n", (i + 1 < app->window_count) ? "," : "");
    }
    fprintf(file, "  ]\n");
    fprintf(file, "}\n");

    fclose(file);
    return 1;
}

void workspace_manager_init(workspace_manager_t *workspace)
{
    if (workspace == NULL) {
        return;
    }

    workspace->workspace_path[0] = '\0';
    workspace->is_dirty = 0;
    workspace->last_change_ticks = 0;
    workspace->debounce_ms = DEFAULT_DEBOUNCE_MS;
    workspace_build_path(workspace, 0);
}

int workspace_manager_load(workspace_manager_t *workspace, app_state_t *app, const char *title,
    int default_width, int default_height)
{
    if (workspace == NULL || app == NULL) {
        return 0;
    }

    if (!workspace_build_path(workspace, 0)) {
        return 0;
    }

    char *buffer = NULL;
    if (!workspace_load_file(workspace->workspace_path, &buffer, NULL)) {
        return 0;
    }

    const char *cursor = buffer;
    int loaded = 0;

    if (json_expect(&cursor, '{')) {
        while (1) {
            char key[64];
            if (!json_parse_string(&cursor, key, sizeof(key))) {
                break;
            }
            if (!json_expect(&cursor, ':')) {
                break;
            }
            if (strcmp(key, "windows") == 0) {
                loaded = parse_windows(&cursor, app, title, default_width, default_height);
            } else {
                if (!json_skip_value(&cursor)) {
                    break;
                }
            }
            const char *pos = json_skip_ws(cursor);
            if (pos == NULL) {
                break;
            }
            if (*pos == ',') {
                cursor = pos + 1;
                continue;
            }
            if (*pos == '}') {
                cursor = pos + 1;
                break;
            }
            break;
        }
    }

    free(buffer);
    return loaded;
}

void workspace_manager_mark_dirty(workspace_manager_t *workspace)
{
    if (workspace == NULL) {
        return;
    }

    workspace->is_dirty = 1;
    workspace->last_change_ticks = SDL_GetTicks64();
}

void workspace_manager_update(workspace_manager_t *workspace, const app_state_t *app)
{
    if (workspace == NULL || app == NULL || !workspace->is_dirty) {
        return;
    }

    Uint64 now = SDL_GetTicks64();
    if (now - workspace->last_change_ticks < workspace->debounce_ms) {
        return;
    }

    if (!workspace_build_path(workspace, 1)) {
        return;
    }

    if (workspace_save_file(workspace->workspace_path, app)) {
        workspace->is_dirty = 0;
    }
}
