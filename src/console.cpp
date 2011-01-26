/* ASE - Allegro Sprite Editor
 * Copyright (C) 2001-2011  David Capello
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <allegro.h>
#include <stdarg.h>
#include <stdio.h>

#include "base/bind.h"
#include "base/memory.h"
#include "gui/jinete.h"

#include "app.h"
#include "console.h"
#include "modules/gui.h"
#include "widgets/statebar.h"

static Frame* wid_console = NULL;
static Widget* wid_view = NULL;
static Widget* wid_textbox = NULL;
static Widget* wid_cancel = NULL;
static int console_counter = 0;
static bool console_locked;
static bool want_close_flag = false;

Console::Console()
{
  console_counter++;

  if (!ji_screen ||
      !App::instance()->isGui() ||
      wid_console ||
      console_counter > 1)
    return;
  else {
    Frame* window = new Frame(false, "Errors Console");
    Grid* grid = new Grid(1, false);
    Widget* view = jview_new();
    Widget* textbox = jtextbox_new(NULL, JI_WORDWRAP);
    Button* button = new Button("&Cancel");

    if (!grid || !textbox || !button)
      return;

    // The "button" closes the console
    button->Click.connect(Bind<void>(&Frame::closeWindow, window, button));

    jview_attach(view, textbox);

    jwidget_set_min_size(button, 60, 0);

    grid->addChildInCell(view, 1, 1, JI_HORIZONTAL | JI_VERTICAL);
    grid->addChildInCell(button, 1, 1, JI_CENTER);
    jwidget_add_child(window, grid);

    view->setVisible(false);
    jwidget_magnetic(button, true);
    jwidget_expansive(view, true);

    /* force foreground mode */
/*     ji_find_widget(window)->in_foreground = true; */

    wid_console = window;
    wid_view = view;
    wid_textbox = textbox;
    wid_cancel = button;
    console_locked = false;
    want_close_flag = false;
  }
}

Console::~Console()
{
  console_counter--;

  if ((wid_console) && (console_counter == 0)) {
    if (console_locked
	&& !want_close_flag
	&& wid_console->isVisible()) {
      /* open in foreground */
      wid_console->open_window_fg();
    }

    delete wid_console; 	// window
    wid_console = NULL;
    want_close_flag = false;
  }
}

void Console::printf(const char *format, ...)
{
  char buf[1024];		// TODO warning buffer overflow
  va_list ap;

  va_start(ap, format);
  uvsprintf(buf, format, ap);
  va_end(ap);

  if (wid_console) {
    const char* text;
    char* final;

    // Open the window
    if (!wid_console->isVisible()) {
      wid_console->open_window();
      jmanager_refresh_screen();
    }

    /* update the textbox */
    if (!console_locked) {
      JRect rect = jrect_new(0, 0, JI_SCREEN_W*9/10, JI_SCREEN_H*6/10);
      console_locked = true;
      
      wid_view->setVisible(true);

      wid_console->remap_window();
      jwidget_set_rect(wid_console, rect);
      wid_console->center_window();
      wid_console->invalidate();

      jrect_free(rect);
    }

    text = wid_textbox->getText();
    if (!text)
      final = base_strdup(buf);
    else {
      final = (char*)base_malloc(ustrlen(text) + ustrlen(buf) + 1);

      ustrcpy(final, empty_string);
      ustrcat(final, text);
      ustrcat(final, buf);
    }

    wid_textbox->setText(final);
    base_free(final);
  }
  else {
    fputs(buf, stdout);
    fflush(stdout);
  }
}

// static
void Console::showException(std::exception& e)
{
  Console console;
  console.printf("A problem has occurred.\n\nDetails:\n%s", e.what());
}
