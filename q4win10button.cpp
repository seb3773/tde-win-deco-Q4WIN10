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

// #include <twin/options.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <tqbitmap.h>
#include <tqcursor.h>
#include <tqpainter.h>
#include <tqpixmap.h>
#include <tqtimer.h>

#include "q4win10button.h"
#include "q4win10button.moc"
#include "q4win10client.h"

namespace KWinQ4Win10 {

static const uint TIMERINTERVAL = 50; // msec
static const uint ANIMATIONSTEPS = 4;

Q4Win10Button::Q4Win10Button(ButtonType type, Q4Win10Client *parent,
                             const char *name)
    : KCommonDecorationButton(type, parent, name), m_client(parent),
      m_iconType(NumButtonIcons), hover(false) {
  setBackgroundMode(NoBackground);

  // no need to reset here as the button will be resetted on first resize.

  // no need to reset here as the button will be resetted on first resize.
}

Q4Win10Button::~Q4Win10Button() {}

void Q4Win10Button::reset(unsigned long changed) {
  if (changed & DecorationReset || changed & ManualReset ||
      changed & SizeChange || changed & StateChange) {
    switch (type()) {
    case CloseButton:
      m_iconType = CloseIcon;
      break;
    case HelpButton:
      m_iconType = HelpIcon;
      break;
    case MinButton:
      m_iconType = MinIcon;
      break;
    case MaxButton:
      if (isOn()) {
        m_iconType = MaxRestoreIcon;
      } else {
        m_iconType = MaxIcon;
      }
      break;
    case OnAllDesktopsButton:
      if (isOn()) {
        m_iconType = NotOnAllDesktopsIcon;
      } else {
        m_iconType = OnAllDesktopsIcon;
      }
      break;
    case ShadeButton:
      if (isOn()) {
        m_iconType = UnShadeIcon;
      } else {
        m_iconType = ShadeIcon;
      }
      break;
    case AboveButton:
      if (isOn()) {
        m_iconType = NoKeepAboveIcon;
      } else {
        m_iconType = KeepAboveIcon;
      }
      break;
    case BelowButton:
      if (isOn()) {
        m_iconType = NoKeepBelowIcon;
      } else {
        m_iconType = KeepBelowIcon;
      }
      break;
    default:
      m_iconType = NumButtonIcons; // empty...
      break;
    }

    // Fix: Sanitize hover state on state changes (e.g. Maximize/Restore).
    if (isVisible()) {
      bool actualHover =
          this->rect().contains(this->mapFromGlobal(TQCursor::pos()));
      if (hover != actualHover) {
        hover = actualHover;
        this->repaint(false);
      }
    }

    this->update();
  }
  // Invalidate cache on reset/changes
  m_scaledMenuIcon = TQPixmap();
}

void Q4Win10Button::enterEvent(TQEvent *e) {
  TQButton::enterEvent(e);
  hover = true;
  repaint(false);
}

void Q4Win10Button::leaveEvent(TQEvent *e) {
  TQButton::leaveEvent(e);
  hover = false;
  repaint(false);
}

void Q4Win10Button::drawButton(TQPainter *painter) {
  TQRect r(0, 0, width(), height());

  bool active = m_client->isActive();
  KPixmap tempKPixmap;

  // Windows 10 Style: No calculation of contour/surface colors needed.
  // We use direct simple colors later.

  TQPixmap buffer;
  buffer.resize(width(), height());
  TQPainter bP(&buffer);

  // fake the titlebar background
  bP.drawTiledPixmap(0, 0, width(), width(), m_client->getTitleBarTile(active));

  // Determine if we should draw the background highlight
  // Reverting to standard "hover" logic as requested.
  // Original Plastik relies on enterEvent/leaveEvent setting 'hover'.
  // Fix: Menu Button (App Icon) should NOT have hover effect.
  bool showBackground = hover && (type() != MenuButton);

  if (showBackground) {
    TQColor bgColor;
    if (type() == CloseButton) {
      bgColor = TQColor(232, 17, 35); // Windows 10 Red
    } else {
      // Restore Original "Light Mode" Logic + Dark Mode Enhancement
      TQColor baseColor = Handler()->getColor(TitleGradient2, active);

      // Fix: Use White for both Light and Dark modes to create a visible
      // "highlight". Light Mode: Subtle highlight (lighter grey). Dark Mode:
      // Visible highlight (lightened header).
      TQColor overlayColor = TQt::white;

      // Opacity Settings (Alpha of Base Color)
      // Light Mode (Classic): ~210/255 Base -> Subtle White overlay.
      // Dark Mode: ~190/255 Base -> Stronger White overlay.
      int alpha = Handler()->darkMode() ? 190 : 210;

      bgColor = alphaBlendColors(baseColor, overlayColor, alpha);
    }
    bP.fillRect(r, bgColor);
  }

  if (type() == MenuButton) {
    // Calculate square size to preserve aspect ratio
    int s = TQMIN(width(), height()) - 6; // -6 for reduced size (more padding)
    if (s < 1)
      s = 1;

    // Check cache
    if (m_scaledMenuIcon.isNull() || m_scaledMenuIcon.width() != s ||
        m_scaledMenuIcon.height() != s) {
      TQPixmap menuIcon(
          m_client->icon().pixmap(TQIconSet::Large, TQIconSet::Normal));
      m_scaledMenuIcon.convertFromImage(
          TQImage(menuIcon.convertToImage()).smoothScale(s, s));
    }

    bP.drawPixmap((width() - m_scaledMenuIcon.width()) / 2,
                  (height() - m_scaledMenuIcon.height()) / 2, m_scaledMenuIcon);
  } else {
    int dX, dY;
    const TQBitmap &icon = Handler()->buttonBitmap(
        m_iconType, size(), decoration()->isToolWindow());
    dX = r.x() + (r.width() - icon.width()) / 2;
    dY = r.y() + (r.height() - icon.height()) / 2;
    if (isDown()) {
      dY++;
    }

    // Set icon color
    TQColor iconColor;

    if (Handler()->darkMode()) {
      // Dark Mode
      if (active) {
        iconColor = TQt::white;
      } else {
        iconColor = TQt::lightGray; // Dimmed for inactive
      }
    } else {
      // Light Mode
      if (active) {
        iconColor = TQt::black;
      } else {
        iconColor = TQt::darkGray; // Dimmed for inactive
      }
    }

    // Special Case: Close Button on Hover/Down
    if (type() == CloseButton && (showBackground || isDown())) {
      iconColor = TQt::white;
    }

    bP.setPen(iconColor);
    bP.drawPixmap(dX, dY, icon);
  }

  bP.end();
  painter->drawPixmap(0, 0, buffer);
}

TQBitmap IconEngine::icon(ButtonIcon icon, int size) {
  if (size % 2 == 0)
    --size;

  TQBitmap bitmap(size, size);
  bitmap.fill(TQt::color0);
  TQPainter p(&bitmap);

  p.setPen(TQt::color1);

  TQRect r = bitmap.rect();

  // line widths
  int lwTitleBar = 1;
  if (r.width() > 16) {
    lwTitleBar = 2; // Reduced from 4
  } else if (r.width() > 4) {
    lwTitleBar = 1;
  }
  int lwArrow = 1;
  if (r.width() > 16) {
    lwArrow = 2; // Reduced from 4
  } else if (r.width() > 7) {
    lwArrow = 1;
  }

  switch (icon) {
  case CloseIcon: {
    int lineWidth = 1;
    if (r.width() > 16) {
      lineWidth = 2;
    } else if (r.width() > 4) {
      lineWidth = 1; // Reverted to 1px as requested
    }

    drawObject(p, DiagonalLine, r.x(), r.y(), r.width(), lineWidth);
    drawObject(p, CrossDiagonalLine, r.x(), r.bottom(), r.width(), lineWidth);

    break;
  }

  case MaxIcon: {
    int lineWidth2 = 1; // frame
    if (r.width() > 16) {
      lineWidth2 = 2;
    } else if (r.width() > 4) {
      lineWidth2 = 1;
    }

    drawObject(p, HorizontalLine, r.x(), r.top(), r.width(), lwTitleBar);
    drawObject(p, HorizontalLine, r.x(), r.bottom() - (lineWidth2 - 1),
               r.width(), lineWidth2);
    drawObject(p, VerticalLine, r.x(), r.top(), r.height(), lineWidth2);
    drawObject(p, VerticalLine, r.right() - (lineWidth2 - 1), r.top(),
               r.height(), lineWidth2);

    break;
  }

  case MaxRestoreIcon: {
    int lineWidth2 = 1; // frame
    if (r.width() > 16) {
      lineWidth2 = 2;
    } else if (r.width() > 4) {
      lineWidth2 = 1;
    }

    int margin1, margin2;
    margin1 = margin2 = lineWidth2 * 2;
    if (r.width() < 8)
      margin1 = 1;

    // background window
    drawObject(p, HorizontalLine, r.x() + margin1, r.top(), r.width() - margin1,
               lineWidth2);
    drawObject(p, HorizontalLine, r.right() - margin2,
               r.bottom() - (lineWidth2 - 1) - margin1, margin2, lineWidth2);
    drawObject(p, VerticalLine, r.x() + margin1, r.top(), margin2, lineWidth2);
    drawObject(p, VerticalLine, r.right() - (lineWidth2 - 1), r.top(),
               r.height() - margin1, lineWidth2);

    // foreground window
    drawObject(p, HorizontalLine, r.x(), r.top() + margin2, r.width() - margin2,
               lwTitleBar);
    drawObject(p, HorizontalLine, r.x(), r.bottom() - (lineWidth2 - 1),
               r.width() - margin2, lineWidth2);
    drawObject(p, VerticalLine, r.x(), r.top() + margin2, r.height(),
               lineWidth2);
    drawObject(p, VerticalLine, r.right() - (lineWidth2 - 1) - margin2,
               r.top() + margin2, r.height(), lineWidth2);

    break;
  }

  case MinIcon: {
    // Windows 10 style: Centered "-"
    int centerY = r.y() + r.height() / 2;
    drawObject(p, HorizontalLine, r.x(), centerY, r.width(), lwTitleBar);

    break;
  }

  case HelpIcon: {
    int center = r.x() + r.width() / 2 - 1;
    int side = r.width() / 4;

    // paint a question mark... code is quite messy, to be cleaned up later...!
    // :o

    if (r.width() > 16) {
      int lineWidth = 3;

      // top bar
      drawObject(p, HorizontalLine, center - side + 3, r.y(), 2 * side - 3 - 1,
                 lineWidth);
      // top bar rounding
      drawObject(p, CrossDiagonalLine, center - side - 1, r.y() + 5, 6,
                 lineWidth);
      drawObject(p, DiagonalLine, center + side - 3, r.y(), 5, lineWidth);
      // right bar
      drawObject(p, VerticalLine, center + side + 2 - lineWidth, r.y() + 3,
                 r.height() - (2 * lineWidth + side + 2 + 1), lineWidth);
      // bottom bar
      drawObject(p, CrossDiagonalLine, center, r.bottom() - 2 * lineWidth,
                 side + 2, lineWidth);
      drawObject(p, HorizontalLine, center, r.bottom() - 3 * lineWidth + 2,
                 lineWidth, lineWidth);
      // the dot
      drawObject(p, HorizontalLine, center, r.bottom() - (lineWidth - 1),
                 lineWidth, lineWidth);
    } else if (r.width() > 8) {
      int lineWidth = 1; // Windows 10 style: 1px thin lines

      // top bar
      drawObject(p, HorizontalLine, center - (side - 1), r.y(), 2 * side - 1,
                 lineWidth);
      // top bar rounding
      if (r.width() > 9) {
        drawObject(p, CrossDiagonalLine, center - side - 1, r.y() + 3, 3,
                   lineWidth);
      } else {
        drawObject(p, CrossDiagonalLine, center - side - 1, r.y() + 2, 3,
                   lineWidth);
      }
      drawObject(p, DiagonalLine, center + side - 1, r.y(), 3, lineWidth);
      // right bar
      drawObject(p, VerticalLine, center + side + 2 - lineWidth, r.y() + 2,
                 r.height() - (2 * lineWidth + side + 1), lineWidth);
      // bottom bar
      drawObject(p, CrossDiagonalLine, center, r.bottom() - 2 * lineWidth + 1,
                 side + 2, lineWidth);
      // the dot
      drawObject(p, HorizontalLine, center, r.bottom() - (lineWidth - 1),
                 lineWidth, lineWidth);
    } else {
      int lineWidth = 1;

      // top bar
      drawObject(p, HorizontalLine, center - (side - 1), r.y(), 2 * side,
                 lineWidth);
      // top bar rounding
      drawObject(p, CrossDiagonalLine, center - side - 1, r.y() + 1, 2,
                 lineWidth);
      // right bar
      drawObject(p, VerticalLine, center + side + 1, r.y(),
                 r.height() - (side + 2 + 1), lineWidth);
      // bottom bar
      drawObject(p, CrossDiagonalLine, center, r.bottom() - 2, side + 2,
                 lineWidth);
      // the dot
      drawObject(p, HorizontalLine, center, r.bottom(), 1, 1);
    }

    break;
  }

  case NotOnAllDesktopsIcon: {
    int lwMark = r.width() - lwTitleBar * 2 - 2;
    if (lwMark < 1)
      lwMark = 3;

    drawObject(p, HorizontalLine, r.x() + (r.width() - lwMark) / 2,
               r.y() + (r.height() - lwMark) / 2, lwMark, lwMark);

    // Fall through to OnAllDesktopsIcon intended!
  }
  case OnAllDesktopsIcon: {
    // horizontal bars
    drawObject(p, HorizontalLine, r.x() + lwTitleBar, r.y(),
               r.width() - 2 * lwTitleBar, lwTitleBar);
    drawObject(p, HorizontalLine, r.x() + lwTitleBar,
               r.bottom() - (lwTitleBar - 1), r.width() - 2 * lwTitleBar,
               lwTitleBar);
    // vertical bars
    drawObject(p, VerticalLine, r.x(), r.y() + lwTitleBar,
               r.height() - 2 * lwTitleBar, lwTitleBar);
    drawObject(p, VerticalLine, r.right() - (lwTitleBar - 1),
               r.y() + lwTitleBar, r.height() - 2 * lwTitleBar, lwTitleBar);

    break;
  }

  case NoKeepAboveIcon: {
    int center = r.x() + r.width() / 2;

    // arrow
    drawObject(p, CrossDiagonalLine, r.x(), center + 2 * lwArrow,
               center - r.x(), lwArrow);
    drawObject(p, DiagonalLine, r.x() + center, r.y() + 1 + 2 * lwArrow,
               center - r.x(), lwArrow);
    if (lwArrow > 1)
      drawObject(p, HorizontalLine, center - (lwArrow - 2), r.y() + 2 * lwArrow,
                 (lwArrow - 2) * 2, lwArrow);

    // Fall through to KeepAboveIcon intended!
  }
  case KeepAboveIcon: {
    int center = r.x() + r.width() / 2;

    // arrow
    drawObject(p, CrossDiagonalLine, r.x(), center, center - r.x(), lwArrow);
    drawObject(p, DiagonalLine, r.x() + center, r.y() + 1, center - r.x(),
               lwArrow);
    if (lwArrow > 1)
      drawObject(p, HorizontalLine, center - (lwArrow - 2), r.y(),
                 (lwArrow - 2) * 2, lwArrow);

    break;
  }

  case NoKeepBelowIcon: {
    int center = r.x() + r.width() / 2;

    // arrow
    drawObject(p, DiagonalLine, r.x(), center - 2 * lwArrow, center - r.x(),
               lwArrow);
    drawObject(p, CrossDiagonalLine, r.x() + center,
               r.bottom() - 1 - 2 * lwArrow, center - r.x(), lwArrow);
    if (lwArrow > 1)
      drawObject(p, HorizontalLine, center - (lwArrow - 2),
                 r.bottom() - (lwArrow - 1) - 2 * lwArrow, (lwArrow - 2) * 2,
                 lwArrow);

    // Fall through to KeepBelowIcon intended!
  }
  case KeepBelowIcon: {
    int center = r.x() + r.width() / 2;

    // arrow
    drawObject(p, DiagonalLine, r.x(), center, center - r.x(), lwArrow);
    drawObject(p, CrossDiagonalLine, r.x() + center, r.bottom() - 1,
               center - r.x(), lwArrow);
    if (lwArrow > 1)
      drawObject(p, HorizontalLine, center - (lwArrow - 2),
                 r.bottom() - (lwArrow - 1), (lwArrow - 2) * 2, lwArrow);

    break;
  }

  case ShadeIcon: {
    drawObject(p, HorizontalLine, r.x(), r.y(), r.width(), lwTitleBar);

    break;
  }

  case UnShadeIcon: {
    int lw1 = 1;
    int lw2 = 1;
    if (r.width() > 16) {
      lw1 = 4;
      lw2 = 2;
    } else if (r.width() > 7) {
      lw1 = 2;
      lw2 = 1;
    }

    int h = TQMAX((r.width() / 2), (lw1 + 2 * lw2));

    // horizontal bars
    drawObject(p, HorizontalLine, r.x(), r.y(), r.width(), lw1);
    drawObject(p, HorizontalLine, r.x(), r.x() + h - (lw2 - 1), r.width(), lw2);
    // vertical bars
    drawObject(p, VerticalLine, r.x(), r.y(), h, lw2);
    drawObject(p, VerticalLine, r.right() - (lw2 - 1), r.y(), h, lw2);

    break;
  }

  default:
    break;
  }

  p.end();

  bitmap.setMask(bitmap);

  return bitmap;
}

void IconEngine::drawObject(TQPainter &p, Object object, int x, int y,
                            int length, int lineWidth) {
  switch (object) {
  case DiagonalLine:
    if (lineWidth <= 1) {
      for (int i = 0; i < length; ++i) {
        p.drawPoint(x + i, y + i);
      }
    } else if (lineWidth <= 2) {
      for (int i = 0; i < length; ++i) {
        p.drawPoint(x + i, y + i);
      }
      for (int i = 0; i < (length - 1); ++i) {
        p.drawPoint(x + 1 + i, y + i);
        p.drawPoint(x + i, y + 1 + i);
      }
    } else {
      for (int i = 1; i < (length - 1); ++i) {
        p.drawPoint(x + i, y + i);
      }
      for (int i = 0; i < (length - 1); ++i) {
        p.drawPoint(x + 1 + i, y + i);
        p.drawPoint(x + i, y + 1 + i);
      }
      for (int i = 0; i < (length - 2); ++i) {
        p.drawPoint(x + 2 + i, y + i);
        p.drawPoint(x + i, y + 2 + i);
      }
    }
    break;
  case CrossDiagonalLine:
    if (lineWidth <= 1) {
      for (int i = 0; i < length; ++i) {
        p.drawPoint(x + i, y - i);
      }
    } else if (lineWidth <= 2) {
      for (int i = 0; i < length; ++i) {
        p.drawPoint(x + i, y - i);
      }
      for (int i = 0; i < (length - 1); ++i) {
        p.drawPoint(x + 1 + i, y - i);
        p.drawPoint(x + i, y - 1 - i);
      }
    } else {
      for (int i = 1; i < (length - 1); ++i) {
        p.drawPoint(x + i, y - i);
      }
      for (int i = 0; i < (length - 1); ++i) {
        p.drawPoint(x + 1 + i, y - i);
        p.drawPoint(x + i, y - 1 - i);
      }
      for (int i = 0; i < (length - 2); ++i) {
        p.drawPoint(x + 2 + i, y - i);
        p.drawPoint(x + i, y - 2 - i);
      }
    }
    break;
  case HorizontalLine:
    for (int i = 0; i < lineWidth; ++i) {
      p.drawLine(x, y + i, x + length - 1, y + i);
    }
    break;
  case VerticalLine:
    for (int i = 0; i < lineWidth; ++i) {
      p.drawLine(x + i, y, x + i, y + length - 1);
    }
    break;
  default:
    break;
  }
}

} // namespace KWinQ4Win10
