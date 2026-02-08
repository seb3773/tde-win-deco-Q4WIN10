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

#include <tdelocale.h>

#include <tqbitmap.h>
#include <tqdatetime.h>
#include <tqdesktopwidget.h>
#include <tqfontmetrics.h>
#include <tqimage.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpainter.h>
#include <tqpixmap.h>

#include "q4win10button.h"
#include "q4win10client.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

namespace KWinQ4Win10 {

// Helper to read X11 property from Style Plugin
static int getMenuBarHeight(WId winId) {
    if (!winId) return 0;
    Display *dpy = tqt_xdisplay();
    Atom atom = XInternAtom(dpy, "_Q4WIN10_MENUBAR_HEIGHT", True);
    if (atom == None) return 0;

    Atom actualType;
    int actualFormat;
    unsigned long nitems;
    unsigned long bytesAfter;
    unsigned char *prop = 0;
    int height = 0;

    if (XGetWindowProperty(dpy, winId, atom, 0, 1, False, XA_CARDINAL,
                           &actualType, &actualFormat, &nitems, &bytesAfter,
                           &prop) == Success) {
        if (prop) {
            if (actualType == XA_CARDINAL && actualFormat == 32 && nitems > 0) {
                height = (int)*(long *)prop;
            }
            XFree(prop);
        }
    }
    return height;
}

Q4Win10Client::Q4Win10Client(KDecorationBridge *bridge,
                             KDecorationFactory *factory)
    : KCommonDecoration(bridge, factory), s_titleFont(TQFont()) {
  memset(m_captionPixmaps, 0, sizeof(TQPixmap *) * 2);
}

Q4Win10Client::~Q4Win10Client() { clearCaptionPixmaps(); }

TQString Q4Win10Client::visibleName() const { return i18n("Q4Win10"); }

TQString Q4Win10Client::defaultButtonsLeft() const { return "M"; }

TQString Q4Win10Client::defaultButtonsRight() const { return "HIAX"; }

bool Q4Win10Client::decorationBehaviour(DecorationBehaviour behaviour) const {
  switch (behaviour) {
  case DB_MenuClose:
    return Handler()->menuClose();

  case DB_WindowMask:
    return true;

  default:
    return KCommonDecoration::decorationBehaviour(behaviour);
  }
}

int Q4Win10Client::layoutMetric(LayoutMetric lm, bool respectWindowState,
                                const KCommonDecorationButton *btn) const {
  bool maximized = maximizeMode() == MaximizeFull &&
                   !options()->moveResizeMaximizedWindows();

  switch (lm) {
  case LM_BorderLeft:
  case LM_BorderRight:
  case LM_BorderBottom: {
    if (respectWindowState && maximized) {
      return 0;
    } else {
      return Handler()->borderSize();
    }
  }

  case LM_TitleEdgeTop: {
    if (respectWindowState && maximized) {
      return 0;
    } else {
      return 4;
    }
  }

  case LM_TitleEdgeBottom: {
    return 2;
  }

  case LM_TitleEdgeLeft:
  case LM_TitleEdgeRight: {
    if (respectWindowState && maximized) {
      return 0;
    } else {
      return 1; // Minimal margin to shift menu icon left
    }
  }

  case LM_TitleBorderLeft:
  case LM_TitleBorderRight:
    return 5;

  case LM_ButtonWidth: {
    // Windows 10 style: Buttons are wider (rectangular)
    int h = (respectWindowState && isToolWindow())
                ? Handler()->titleHeightTool()
                : Handler()->titleHeight();
    return (h * 9) / 5; // 1.8 aspect ratio (Wider Win10 style)
  }

  case LM_ButtonHeight: {
    int h = (respectWindowState && isToolWindow())
                ? Handler()->titleHeightTool()
                : Handler()->titleHeight();

    if (respectWindowState && maximized) {
      return h;
    } else {
      // Stretch 3px up into the top border (Avoid overlapping 1px inactive
      // border)
      return h + 3;
    }
  }

  case LM_TitleHeight: {
    if (respectWindowState && isToolWindow()) {
      return Handler()->titleHeightTool();
    } else {
      return Handler()->titleHeight();
    }
  }

  case LM_ButtonSpacing:
    return 1;

  case LM_ButtonMarginTop:
    if (respectWindowState && maximized) {
      return 0;
    } else {
      // Negative margin to move button up
      // -3 to avoid overlapping the 1px top inactive border
      return -3;
    }

  case LM_ExplicitButtonSpacer:
    return 3;

  default:
    return KCommonDecoration::layoutMetric(lm, respectWindowState, btn);
  }
}

KCommonDecorationButton *Q4Win10Client::createButton(ButtonType type) {
  switch (type) {
  case MenuButton:
    return new Q4Win10Button(MenuButton, this, "menu");

  case OnAllDesktopsButton:
    return new Q4Win10Button(OnAllDesktopsButton, this, "on_all_desktops");

  case HelpButton:
    return new Q4Win10Button(HelpButton, this, "help");

  case MinButton:
    return new Q4Win10Button(MinButton, this, "minimize");

  case MaxButton:
    return new Q4Win10Button(MaxButton, this, "maximize");

  case CloseButton:
    return new Q4Win10Button(CloseButton, this, "close");

  case AboveButton:
    return new Q4Win10Button(AboveButton, this, "above");

  case BelowButton:
    return new Q4Win10Button(BelowButton, this, "below");

  case ShadeButton:
    return new Q4Win10Button(ShadeButton, this, "shade");

  default:
    return 0;
  }
}

void Q4Win10Client::init() {
  s_titleFont =
      isToolWindow() ? Handler()->titleFontTool() : Handler()->titleFont();

  clearCaptionPixmaps();

  KCommonDecoration::init();
}

TQRegion Q4Win10Client::cornerShape(WindowCorner corner) {
  // Windows 10 style: Square Corners (No masking)
  return TQRegion();
}

void Q4Win10Client::borders(int &left, int &right, int &top, int &bottom) const {
  bool maximized = maximizeMode() == MaximizeFull &&
                   !options()->moveResizeMaximizedWindows();

  if (maximized) {
    left = right = bottom = 0;
  } else {
    left = right = bottom = Handler()->borderSize();
  }

  top = layoutMetric(LM_TitleHeight) + layoutMetric(LM_TitleEdgeTop) +
        layoutMetric(LM_TitleEdgeBottom);

  // Debug: force 4 if handler failed
  if (left < 4 && !maximized) {
      left = right = bottom = 4;
  }
}

void Q4Win10Client::paintEvent(TQPaintEvent *e) {
  TQRegion region = e->region();

  Q4Win10Handler *handler = Handler();

  if (oldCaption != caption())
    clearCaptionPixmaps();

  bool active = isActive();
  bool toolWindow = isToolWindow();

  TQPainter painter(widget());

  // often needed coordinates
  TQRect r = widget()->rect();

  int r_w = r.width();
  //     int r_h = r.height();
  int r_x, r_y, r_x2, r_y2;
  r.coords(&r_x, &r_y, &r_x2, &r_y2);
  const int borderLeft = layoutMetric(LM_BorderLeft);
  const int borderRight = layoutMetric(LM_BorderRight);
  const int borderBottom = layoutMetric(LM_BorderBottom);
  const int titleHeight = layoutMetric(LM_TitleHeight);
  const int titleEdgeTop = layoutMetric(LM_TitleEdgeTop);
  const int titleEdgeBottom = layoutMetric(LM_TitleEdgeBottom);
  const int titleEdgeLeft = layoutMetric(LM_TitleEdgeLeft);
  const int titleEdgeRight = layoutMetric(LM_TitleEdgeRight);

  const int borderBottomTop = r_y2 - borderBottom + 1;
  const int borderLeftRight = r_x + borderLeft - 1;
  const int borderRightLeft = r_x2 - borderRight + 1;
  const int titleEdgeBottomBottom =
      r_y + titleEdgeTop + titleHeight + titleEdgeBottom - 1;

  const int sideHeight = borderBottomTop - titleEdgeBottomBottom - 1;

  TQRect Rtitle =
      TQRect(r_x + titleEdgeLeft + buttonsLeftWidth(), r_y + titleEdgeTop,
             r_x2 - titleEdgeRight - buttonsRightWidth() -
                 (r_x + titleEdgeLeft + buttonsLeftWidth()),
             titleEdgeBottomBottom - (r_y + titleEdgeTop));

  TQRect tempRect;

  // topSpacer
  if (titleEdgeTop > 0) {
    tempRect.setRect(r_x + 2, r_y, r_w - 2 * 2, titleEdgeTop);
    if (tempRect.isValid() && region.contains(tempRect)) {
      painter.drawTiledPixmap(
          tempRect, handler->pixmap(TitleBarTileTop, active, toolWindow));
    }
  }

  // leftTitleSpacer
  int titleMarginLeft = 0;
  int titleMarginRight = 0;
  if (titleEdgeLeft > 0) {
    tempRect.setRect(r_x, r_y, borderLeft,
                     titleEdgeTop + titleHeight + titleEdgeBottom);
    if (tempRect.isValid() && region.contains(tempRect)) {
      painter.drawTiledPixmap(
          tempRect, handler->pixmap(TitleBarLeft, active, toolWindow));
      titleMarginLeft = borderLeft;
    }
  }

  // rightTitleSpacer
  if (titleEdgeRight > 0) {
    tempRect.setRect(borderRightLeft, r_y, borderRight,
                     titleEdgeTop + titleHeight + titleEdgeBottom);
    if (tempRect.isValid() && region.contains(tempRect)) {
      painter.drawTiledPixmap(
          tempRect, handler->pixmap(TitleBarRight, active, toolWindow));
      titleMarginRight = borderRight;
    }
  }

  // titleSpacer
  const TQPixmap &caption = captionPixmap();
  if (Rtitle.width() > 0) {
    m_captionRect = captionRect(); // also update m_captionRect!
    if (m_captionRect.isValid() && region.contains(m_captionRect)) {
      painter.drawTiledPixmap(m_captionRect, caption);
    }

    // left to the title
    tempRect.setRect(r_x + titleMarginLeft, m_captionRect.top(),
                     m_captionRect.left() - (r_x + titleMarginLeft),
                     m_captionRect.height());
    if (tempRect.isValid() && region.contains(tempRect)) {
      painter.drawTiledPixmap(
          tempRect, handler->pixmap(TitleBarTile, active, toolWindow));
    }

    // right to the title
    tempRect.setRect(m_captionRect.right() + 1, m_captionRect.top(),
                     (r_x2 - titleMarginRight) - m_captionRect.right(),
                     m_captionRect.height());
    if (tempRect.isValid() && region.contains(tempRect)) {
      painter.drawTiledPixmap(
          tempRect, handler->pixmap(TitleBarTile, active, toolWindow));
    }
  }

  // leftSpacer
  // leftSpacer
  if (borderLeft > 0 && sideHeight > 0) {
    int mbHeight = getMenuBarHeight(windowId());
    
    // Split Border Logic
    if (mbHeight > 0 && mbHeight < sideHeight) {
        // TOP Section (Menu Bar Level) - Paint with Base Color (White)
        TQRect menuRect;
        // Adjusted height: mbHeight - 2 to match visual menu bar bottom
        menuRect.setCoords(r_x, titleEdgeBottomBottom + 1, borderLeftRight, 
                           titleEdgeBottomBottom + mbHeight - 2);
        if (menuRect.isValid() && region.contains(menuRect)) {
            painter.fillRect(menuRect, widget()->colorGroup().base());
            // Add a 1px line on the left edge if needed for contrast? 
            // The style usually puts a 1px border. Let's replicate BorderLeftTile logic for the outer edge.
            if (!active) {
                 painter.setPen(Handler()->darkMode() ? TQColor(90, 90, 90) : TQColor(170, 170, 170));
                 painter.drawPoint(menuRect.left(), menuRect.top());
                 painter.drawLine(menuRect.left(), menuRect.top(), menuRect.left(), menuRect.bottom());
            }
        }

        // BOTTOM Section (Rest of Window) - Paint with Standard Border Tile
        // Adjusted start Y: mbHeight - 2 + 1 = mbHeight - 1
        tempRect.setCoords(r_x, titleEdgeBottomBottom + mbHeight - 1, borderLeftRight,
                           borderBottomTop - 1);
        if (tempRect.isValid() && region.contains(tempRect)) {
           // We need to offset the tile drawing so it aligns? 
           // drawTiledPixmap origin is default top-left of rect.
           painter.drawTiledPixmap(
              tempRect, handler->pixmap(BorderLeftTile, active, toolWindow));
        }

    } else {
        // Standard Uniform Border
        tempRect.setCoords(r_x, titleEdgeBottomBottom + 1, borderLeftRight,
                           borderBottomTop - 1);
        if (tempRect.isValid() && region.contains(tempRect)) {
            painter.drawTiledPixmap(
                tempRect, handler->pixmap(BorderLeftTile, active, toolWindow));
        }
    }
  }

  // rightSpacer
  // rightSpacer
  if (borderRight > 0 && sideHeight > 0) {
    int mbHeight = getMenuBarHeight(windowId());
    
    // Split Border Logic
    if (mbHeight > 0 && mbHeight < sideHeight) {
        // TOP Section (Menu Bar Level) - Paint with Base Color (White)
        TQRect menuRect;
        // Adjusted height: mbHeight - 2
        menuRect.setCoords(borderRightLeft, titleEdgeBottomBottom + 1, r_x2, 
                           titleEdgeBottomBottom + mbHeight - 2);
        if (menuRect.isValid() && region.contains(menuRect)) {
            painter.fillRect(menuRect, widget()->colorGroup().base());
            // Outer edge logic for inactive window
            if (!active) {
                 painter.setPen(Handler()->darkMode() ? TQColor(90, 90, 90) : TQColor(170, 170, 170));
                 painter.drawLine(menuRect.right(), menuRect.top(), menuRect.right(), menuRect.bottom());
            }
        }

        // BOTTOM Section (Rest of Window)
        // Adjusted start Y: mbHeight - 1
        tempRect.setCoords(borderRightLeft, titleEdgeBottomBottom + mbHeight - 1, r_x2,
                           borderBottomTop - 1);
        if (tempRect.isValid() && region.contains(tempRect)) {
            painter.drawTiledPixmap(
                tempRect, handler->pixmap(BorderRightTile, active, toolWindow));
        }

    } else {
        // Standard Uniform Border
        tempRect.setCoords(borderRightLeft, titleEdgeBottomBottom + 1, r_x2,
                           borderBottomTop - 1);
        if (tempRect.isValid() && region.contains(tempRect)) {
            painter.drawTiledPixmap(
                tempRect, handler->pixmap(BorderRightTile, active, toolWindow));
        }
    }
  }

  // bottomSpacer
  if (borderBottom > 0) {
    int l = r_x;
    int r = r_x2;

    tempRect.setRect(r_x, borderBottomTop, borderLeft, borderBottom);
    if (tempRect.isValid() && region.contains(tempRect)) {
      painter.drawTiledPixmap(
          tempRect, handler->pixmap(BorderBottomLeft, active, toolWindow));
      l = tempRect.right() + 1;
    }

    tempRect.setRect(borderRightLeft, borderBottomTop, borderLeft,
                     borderBottom);
    if (tempRect.isValid() && region.contains(tempRect)) {
      painter.drawTiledPixmap(
          tempRect, handler->pixmap(BorderBottomRight, active, toolWindow));
      r = tempRect.left() - 1;
    }

    tempRect.setCoords(l, borderBottomTop, r, r_y2);
    if (tempRect.isValid() && region.contains(tempRect)) {
      painter.drawTiledPixmap(
          tempRect, handler->pixmap(BorderBottomTile, active, toolWindow));
    }
  }
}

TQRect Q4Win10Client::captionRect() const {
  const TQPixmap &caption = captionPixmap();
  TQRect r = widget()->rect();

  const int titleHeight = layoutMetric(LM_TitleHeight);
  const int titleEdgeBottom = layoutMetric(LM_TitleEdgeBottom);
  const int titleEdgeTop = layoutMetric(LM_TitleEdgeTop);
  const int titleEdgeLeft = layoutMetric(LM_TitleEdgeLeft);
  const int marginLeft = layoutMetric(LM_TitleBorderLeft);
  const int marginRight = layoutMetric(LM_TitleBorderRight);

  const int titleLeft =
      r.left() + titleEdgeLeft + buttonsLeftWidth() + marginLeft;
  const int titleWidth = r.width() - titleEdgeLeft -
                         layoutMetric(LM_TitleEdgeRight) - buttonsLeftWidth() -
                         buttonsRightWidth() - marginLeft - marginRight;

  TQt::AlignmentFlags a = Handler()->titleAlign();

  int tX, tW; // position/width of the title buffer
  if (caption.width() > titleWidth) {
    tW = titleWidth;
  } else {
    tW = caption.width();
  }
  if (a == TQt::AlignLeft || (caption.width() > titleWidth)) {
    // Align left
    tX = titleLeft;
  } else if (a == TQt::AlignHCenter) {
    // Align center
    tX = titleLeft + (titleWidth - caption.width()) / 2;
  } else {
    // Align right
    tX = titleLeft + titleWidth - caption.width();
  }

  return TQRect(tX, r.top() + titleEdgeTop, tW, titleHeight + titleEdgeBottom);
}

void Q4Win10Client::updateCaption() {
  TQRect oldCaptionRect = m_captionRect;

  if (oldCaption != caption())
    clearCaptionPixmaps();

  m_captionRect = Q4Win10Client::captionRect();

  if (oldCaptionRect.isValid() && m_captionRect.isValid())
    widget()->update(oldCaptionRect | m_captionRect);
  else
    widget()->update();
}

void Q4Win10Client::reset(unsigned long changed) {
  // Always reload config on reset to pick up Dark Mode changes
  Handler()->readConfig();

  if (changed & SettingColors) {
    // repaint the whole thing
    clearCaptionPixmaps();
    widget()->update();
    updateButtons();
  } else if (changed & SettingFont) {
    // font has changed -- update title height and font
    s_titleFont =
        isToolWindow() ? Handler()->titleFontTool() : Handler()->titleFont();

    updateLayout();

    // then repaint
    clearCaptionPixmaps();
    widget()->update();
  }

  KCommonDecoration::reset(changed);
}

const TQPixmap &Q4Win10Client::getTitleBarTile(bool active) const {
  return Handler()->pixmap(TitleBarTile, active, isToolWindow());
}

const TQPixmap &Q4Win10Client::captionPixmap() const {
  bool active = isActive();

  if (m_captionPixmaps[active]) {
    return *m_captionPixmaps[active];
  }

  // not found, create new pixmap...

  const uint maxCaptionLength = 300; // truncate captions longer than this!
  TQString c(caption());
  if (c.length() > maxCaptionLength) {
    c.truncate(maxCaptionLength);
    c.append(" [...]");
  }

  TQFontMetrics fm(s_titleFont);
  int captionWidth = fm.width(c);
  int captionHeight = fm.height();

  const int th = layoutMetric(LM_TitleHeight, false) +
                 layoutMetric(LM_TitleEdgeBottom, false);

  TQPainter painter;

  const int thickness = 2;

  TQPixmap *captionPixmap = new TQPixmap(captionWidth + 2 * thickness, th);

  painter.begin(captionPixmap);
  painter.drawTiledPixmap(
      captionPixmap->rect(),
      Handler()->pixmap(TitleBarTile, active, isToolWindow()));

  painter.setFont(s_titleFont);
  TQPoint tp(1, captionHeight - 4); // Adjusted: -3 instead of -1 to center title vertically
  if (Handler()->titleShadow()) {
    TQColor shadowColor;
    if (tqGray(Handler()->getColor(TitleFont, active).rgb()) < 100)
      shadowColor = TQColor(255, 255, 255);
    else
      shadowColor = TQColor(0, 0, 0);

    painter.setPen(alphaBlendColors(options()->color(ColorTitleBar, active),
                                    shadowColor, 205));
    painter.drawText(tp + TQPoint(1, 2), c);
    painter.setPen(alphaBlendColors(options()->color(ColorTitleBar, active),
                                    shadowColor, 225));
    painter.drawText(tp + TQPoint(2, 2), c);
    painter.setPen(alphaBlendColors(options()->color(ColorTitleBar, active),
                                    shadowColor, 165));
    painter.drawText(tp + TQPoint(1, 1), c);
  }
  painter.setPen(Handler()->getColor(TitleFont, active));
  painter.drawText(tp, c);
  painter.end();

  m_captionPixmaps[active] = captionPixmap;
  return *captionPixmap;
}

void Q4Win10Client::clearCaptionPixmaps() {
  for (int i = 0; i < 2; ++i) {
    delete m_captionPixmaps[i];
    m_captionPixmaps[i] = 0;
  }

  oldCaption = caption();
}

} // namespace KWinQ4Win10
