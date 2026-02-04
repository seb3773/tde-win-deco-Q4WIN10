/* Plastik KWin window decoration
  Copyright (C) 2003-2005 Sandro Giessl <sandro@giessl.com>

  based on the window decoration "Web":
  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
 */

#ifndef Q4WIN10BUTTON_H
#define Q4WIN10BUTTON_H

#include "q4win10.h"
#include <tqbutton.h>
#include <tqimage.h>

#include <kcommondecoration.h>

class TQTimer;

namespace KWinQ4Win10 {

class Q4Win10Client;

class Q4Win10Button : public KCommonDecorationButton {
  TQ_OBJECT
public:
  Q4Win10Button(ButtonType type, Q4Win10Client *parent, const char *name);
  ~Q4Win10Button();

  void reset(unsigned long changed);
  Q4Win10Client *client() { return m_client; }

private:
  void enterEvent(TQEvent *e);
  void leaveEvent(TQEvent *e);
  void drawButton(TQPainter *painter);

private:
  Q4Win10Client *m_client;
  ButtonIcon m_iconType;
  bool hover;
  TQPixmap m_scaledMenuIcon;
};

/**
 * This class creates bitmaps which can be used as icons on buttons. The icons
 * are "hardcoded".
 * Over the previous "Gimp->xpm->TQImage->recolor->SmoothScale->TQPixmap"
 * solution it has the important advantage that icons are more scalable and at
 * the same time sharp and not blurred.
 */
class IconEngine {
public:
  static TQBitmap icon(ButtonIcon icon, int size);

private:
  enum Object { HorizontalLine, VerticalLine, DiagonalLine, CrossDiagonalLine };

  static void drawObject(TQPainter &p, Object object, int x, int y, int length,
                         int lineWidth);
};

} // namespace KWinQ4Win10

#endif // Q4WIN10BUTTON_H
