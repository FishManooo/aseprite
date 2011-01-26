// ASE gui library
// Copyright (C) 2001-2011  David Capello
//
// This source file is ditributed under a BSD-like license, please
// read LICENSE.txt for more information.

/***********************************************************************
 
  Alert format:
  ------------

     "Title Text"
     "==Centre line of text"
     "--"
     "<<Left line of text"
     ">>Right line of text"
     "||My &Button||&Your Button||&He Button"

    .-----------------------------------------.
    | My Message                              |
    +=========================================+
    |           Centre line of text           |
    | --------------------------------------- |
    | Left line of text                       |
    |                      Right line of text |
    |                                         |
    | [My Button]  [Your Button] [Him Button] |
    `-----~---------~-------------~-----------'
 */

#include "config.h"

#include <allegro/unicode.h>
#include <stdarg.h>
#include <stdio.h>

#include "base/bind.h"
#include "gui/jinete.h"

static Frame* create_alert(char *buf, JList *labels, JList *buttons);

/* creates a new alert-box

   the buttons will have names like: button-1, button-2, etc.
 */
Frame* jalert_new(const char *format, ...)
{
  JList labels, buttons;
  char buf[4096];
  Frame* window;
  va_list ap;

  /* process arguments */
  va_start(ap, format);
  vsprintf(buf, format, ap);
  va_end(ap);

  /* create the alert window */
  labels = jlist_new();
  buttons = jlist_new();

  window = create_alert(buf, &labels, &buttons);

  jlist_free(labels);
  jlist_free(buttons);

  /* and return it */
  return window;
}

int jalert(const char *format, ...)
{
  JList labels, buttons;
  Frame* window;
  Widget* killer;
  char buf[4096];
  int c, ret = 0;
  JLink link;
  va_list ap;

  /* process arguments */
  va_start(ap, format);
  vsprintf(buf, format, ap);
  va_end(ap);

  /* create the alert window */
  labels = jlist_new();
  buttons = jlist_new();

  window = create_alert(buf, &labels, &buttons);

  /* was created succefully? */
  if (window) {
    /* open it */
    window->open_window_fg();

    /* check the killer */
    killer = window->get_killer();
    if (killer) {
      c = 1;
      JI_LIST_FOR_EACH(buttons, link) {
	if (killer == (JWidget)link->data) {
	  ret = c;
	  break;
	}
	c++;
      }
    }

    /* destroy the window */
    jwidget_free(window);
  }

  jlist_free(labels);
  jlist_free(buttons);

  /* and return it */
  return ret;
}

static Frame* create_alert(char *buf, JList *labels, JList *buttons)
{
  Box* box1, *box2, *box3, *box4, *box5;
  Grid* grid;
  Frame* window = NULL;
  bool title = true;
  bool label = false;
  bool separator = false;
  bool button = false;
  int align = 0;
  char *beg;
  int c, chr;
  JLink link;

  /* process buffer */
  c = 0;
  beg = buf;
  for (; ; c++) {
    if ((!buf[c]) ||
	((buf[c] == buf[c+1]) &&
	 ((buf[c] == '<') ||
	  (buf[c] == '=') ||
	  (buf[c] == '>') ||
	  (buf[c] == '-') ||
	  (buf[c] == '|')))) {
      if (title || label || separator || button) {
	chr = buf[c];
	buf[c] = 0;

	if (title) {
	  window = new Frame(false, beg);
	}
	else if (label) {
	  Label* label = new Label(beg);
	  label->setAlign(align);
	  jlist_append(*labels, label);
	}
	else if (separator) {
	  jlist_append(*labels, ji_separator_new(NULL, JI_HORIZONTAL));
	}
	else if (button) {
	  char button_name[256];
	  Button* button_widget = new Button(beg);
	  jwidget_set_min_size(button_widget, 60*jguiscale(), 0);
	  jlist_append(*buttons, button_widget);

	  usprintf(button_name, "button-%d", jlist_length(*buttons));
	  button_widget->setName(button_name);
	  button_widget->Click.connect(Bind<void>(&Frame::closeWindow, window, button_widget));
	}

	buf[c] = chr;
      }

      /* done */
      if (!buf[c])
	break;
      /* next widget */
      else {
	title = label = separator = button = false;
	beg = buf+c+2;
	align = 0;

	switch (buf[c]) {
	  case '<': label=true; align=JI_LEFT; break;
	  case '=': label=true; align=JI_CENTER; break;
	  case '>': label=true; align=JI_RIGHT; break;
	  case '-': separator=true; break;
	  case '|': button=true; break;
	}
	c++;
      }
    }
  }

  if (window) {
    box1 = new Box(JI_VERTICAL);
    box2 = new Box(JI_VERTICAL);
    grid = new Grid(1, false);
    box3 = new Box(JI_HORIZONTAL | JI_HOMOGENEOUS);

    /* to identify by the user */
    box2->setName("labels");
    box3->setName("buttons");

    /* pseudo separators (only to fill blank space) */
    box4 = new Box(0);
    box5 = new Box(0);

    jwidget_expansive(box4, true);
    jwidget_expansive(box5, true);
    jwidget_noborders(box4);
    jwidget_noborders(box5);

    /* setup parent <-> children relationship */

    jwidget_add_child(window, box1);

    jwidget_add_child(box1, box4);	/* filler */
    jwidget_add_child(box1, box2);	/* labels */
    jwidget_add_child(box1, box5);	/* filler */
    jwidget_add_child(box1, grid);	/* buttons */

    grid->addChildInCell(box3, 1, 1, JI_CENTER | JI_BOTTOM);

    JI_LIST_FOR_EACH(*labels, link)
      jwidget_add_child(box2, (JWidget)link->data);

    JI_LIST_FOR_EACH(*buttons, link)
      jwidget_add_child(box3, (JWidget)link->data);

    /* default button is the last one */
    if (jlist_last(*buttons))
      jwidget_magnetic((JWidget)jlist_last(*buttons)->data, true);
  }

  return window;
}
