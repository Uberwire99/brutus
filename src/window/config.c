#include "config.h"

#include "core/config.h"
#include "core/dir.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/string.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "window/main_menu.h"

#include <string.h>

#define NUM_CHECKBOXES 9
#define NUM_BOTTOM_BUTTONS 3
#define MAX_LANGUAGE_DIRS 20

static void toggle_switch(int id, int param2);
static void button_reset_defaults(int param1, int param2);
static void button_close(int save, int param2);

static generic_button checkbox_buttons[] = {
    { 20, 72, 20, 20, toggle_switch, button_none, CONFIG_UI_SHOW_INTRO_VIDEO },
    { 20, 96, 20, 20, toggle_switch, button_none, CONFIG_UI_SIDEBAR_INFO },
    { 20, 120, 20, 20, toggle_switch, button_none, CONFIG_UI_SMOOTH_SCROLLING },
    { 20, 144, 20, 20, toggle_switch, button_none, CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE },
    { 20, 168, 20, 20, toggle_switch, button_none, CONFIG_UI_ALLOW_CYCLING_TEMPLES },
    { 20, 192, 20, 20, toggle_switch, button_none, CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE },
    { 20, 264, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_IMMIGRATION_BUG },
    { 20, 288, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_100_YEAR_GHOSTS },
    { 20, 312, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_EDITOR_EVENTS }
};

static generic_button bottom_buttons[] = {
    { 20, 430, 150, 30, button_reset_defaults, button_none },
    { 410, 430, 100, 30, button_close, button_none, 0 },
    { 520, 430, 100, 30, button_close, button_none, 1 },
};

static const char *bottom_button_texts[] = {
    "Reset defaults",
    "Cancel",
    "OK"
};

static struct {
    int focus_button;
    int bottom_focus_button;
    int values[CONFIG_MAX_ENTRIES];
    char string_values[CONFIG_STRING_MAX_ENTRIES][CONFIG_STRING_VALUE_MAX];
    char language_options_data[MAX_LANGUAGE_DIRS][CONFIG_STRING_VALUE_MAX];
    char *language_options[MAX_LANGUAGE_DIRS];
    int num_language_options;
} data;

static void init(void)
{
    for (int i = 0; i < NUM_CHECKBOXES; i++) {
        config_key key = checkbox_buttons[i].parameter1;
        data.values[key] = config_get(key);
    }
    data.num_language_options = 0;
    const dir_listing *subdirs = dir_find_all_subdirectories();
    for (int i = 0; i < subdirs->num_files; i++) {
        if (lang_dir_is_valid(subdirs->files[i])) {
            int opt_id = data.num_language_options;
            strncpy(data.language_options_data[opt_id], subdirs->files[i], CONFIG_STRING_VALUE_MAX);
            data.language_options[opt_id] = data.language_options_data[opt_id];
            data.num_language_options++;
        }
    }
}

static void draw_background(void)
{
    graphics_clear_screen();

    image_draw_fullscreen_background(image_group(GROUP_CONFIG));

    graphics_in_dialog();
    outer_panel_draw(0, 0, 40, 30);

    text_draw_centered(string_from_ascii("Julius configuration options"), 16, 16, 608, FONT_LARGE_BLACK, 0);

    text_draw(string_from_ascii("User interface changes"), 20, 53, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Play intro videos"), 50, 77, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Extra information in the control panel"), 50, 101, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Enable smooth scrolling"), 50, 125, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Improve visual feedback when clearing land"), 50, 149, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Allow building each temple in succession"), 50, 173, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Show range when building reservoirs, fountains and wells"), 50, 197, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Gameplay changes"), 20, 245, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Fix immigration bug on very hard"), 50, 269, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Fix 100-year-old ghosts"), 50, 293, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Fix Emperor change and survival time in custom missions"), 50, 317, FONT_NORMAL_BLACK, 0);

    for (int i = 0; i < NUM_CHECKBOXES; i++) {
        generic_button *btn = &checkbox_buttons[i];
        if (data.values[btn->parameter1]) {
            text_draw(string_from_ascii("x"), btn->x + 6, btn->y + 3, FONT_NORMAL_BLACK, 0);
        }
    }

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        text_draw_centered(string_from_ascii(bottom_button_texts[i]), bottom_buttons[i].x, bottom_buttons[i].y + 9, bottom_buttons[i].width, FONT_NORMAL_BLACK, 0);
    }

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    for (int i = 0; i < NUM_CHECKBOXES; i++) {
        generic_button *btn = &checkbox_buttons[i];
        button_border_draw(btn->x, btn->y, btn->width, btn->height, data.focus_button == i + 1);
    }
    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        button_border_draw(bottom_buttons[i].x, bottom_buttons[i].y, bottom_buttons[i].width, bottom_buttons[i].height, data.bottom_focus_button == i + 1);
    }
    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    generic_buttons_handle_mouse(m_dialog, 0, 0, checkbox_buttons, NUM_CHECKBOXES, &data.focus_button);
    generic_buttons_handle_mouse(m_dialog, 0, 0, bottom_buttons, NUM_BOTTOM_BUTTONS, &data.bottom_focus_button);
}

static void toggle_switch(int key, int param2)
{
    data.values[key] = 1 - data.values[key];
    window_invalidate();
}

static void button_reset_defaults(int param1, int param2)
{
    config_set_defaults();
    init();
    window_invalidate();
}

static void button_close(int save, int param2)
{
    if (save) {
        for (int i = 0; i < NUM_CHECKBOXES; i++) {
            config_key key = checkbox_buttons[i].parameter1;
            config_set(key, data.values[key]);
        }
    }
    window_main_menu_show(0);
}

void window_config_show()
{
    window_type window = {
        WINDOW_CONFIG,
        draw_background,
        draw_foreground,
        handle_mouse
    };
    init();
    window_show(&window);
}
