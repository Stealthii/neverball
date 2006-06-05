/*
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#include <string.h>

#include "gui.h"
#include "game.h"
#include "util.h"
#include "audio.h"
#include "config.h"
#include "demo.h"
#include "st_shared.h"

#include "st_save.h"

extern struct state st_save;
extern struct state st_clobber;
static char filename[MAXNAM];

/*---------------------------------------------------------------------------*/

static struct state *ok_state;
static struct state *cancel_state;

int goto_save(struct state *ok, struct state *cancel)
{
    demo_unique(filename);
    ok_state     = ok;
    cancel_state = cancel;
    return goto_state(&st_save);
}

/*---------------------------------------------------------------------------*/

#define SAVE_SAVE   2
#define SAVE_CANCEL 3

static int file_id;

static int save_action(int i)
{
    size_t l = strlen(filename);

    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case SAVE_SAVE:
        if (strcmp(filename, "") == 0)
            return 1;
        if (demo_exists(filename))
            return goto_state(&st_clobber);
        else
        {
            demo_play_save(filename);
            return goto_state(ok_state);
        }

    case SAVE_CANCEL:
        return goto_state(cancel_state);

    case GUI_BS:
        if (l > 0)
        {
            filename[l - 1] = 0;
            gui_set_label(file_id, filename);
        }
        break;

    default:
        if (l < MAXNAM - 1)
        {
            filename[l + 0] = (char) i;
            filename[l + 1] = 0;
            gui_set_label(file_id, filename);
        }
    }
    return 1;
}

static int enter_id;

static int save_enter(void)
{
    int id, jd;

    if ((id = gui_vstack(0)))
    {
        gui_label(id, _("Replay Name"), GUI_MED, GUI_ALL, 0, 0);

        gui_space(id);
        gui_space(id);

        file_id = gui_label(id, filename, GUI_MED, GUI_ALL, gui_yel, gui_yel);

        gui_space(id);

        gui_keyboard(id);
        if ((jd = gui_harray(id)))
        {
            enter_id = gui_start(jd, _("Save"), GUI_SML, SAVE_SAVE, 0);
            gui_state(jd, _("Cancel"), GUI_SML, SAVE_CANCEL, 0);
        }

        gui_layout(id, 0, 0);
    }

    SDL_EnableUNICODE(1);

    return id;
}

static void save_leave(int id)
{
    SDL_EnableUNICODE(0);
    gui_delete(id);
}

static int save_keybd(int c, int d)
{
    if (d)
        if ((c & 0xFF80) == 0)
        {
            gui_focus(enter_id);
            c &= 0x7F;
            if (c == '\b')
                return save_action(GUI_BS);
            else if (c > ' ')
                return save_action(c);
        }
    return 1;
}

static int save_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return save_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return save_action(SAVE_CANCEL);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int clobber_action(int i)
{
    audio_play(AUD_MENU, 1.0f);

    if (i == SAVE_SAVE)
    {
        demo_play_save(filename);
        return goto_state(ok_state);
    }
    return goto_state(&st_save);
}

static int clobber_enter(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, _("Overwrite?"), GUI_MED, GUI_ALL, gui_red, gui_red);

        gui_label(id, filename, GUI_MED, GUI_ALL, gui_yel, gui_yel);

        if ((jd = gui_harray(id)))
        {
            gui_start(jd, _("No"),  GUI_SML, SAVE_CANCEL, 1);
            gui_state(jd, _("Yes"), GUI_SML, SAVE_SAVE,   0);
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }

    return id;
}

static int clobber_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return clobber_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return clobber_action(SAVE_CANCEL);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_save = {
    save_enter,
    save_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_click,
    save_keybd,
    save_buttn,
    1, 0
};

struct state st_clobber = {
    clobber_enter,
    shared_leave,
    shared_paint,
    shared_timer,
    shared_point,
    shared_stick,
    shared_click,
    NULL,
    clobber_buttn,
    1, 0
};
