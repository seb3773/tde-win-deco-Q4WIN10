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

#include <tqbitmap.h>
#include <tqimage.h>
#include <tqpainter.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <tdeconfig.h>

#include "q4win10.h"
#include "q4win10.moc"
#include "q4win10button.h"
#include "q4win10client.h"

namespace KWinQ4Win10 {

Q4Win10Handler::Q4Win10Handler() {
  memset(m_pixmaps, 0,
         sizeof(TQPixmap *) * NumPixmaps * 2 * 2); // set elements to 0
  memset(m_bitmaps, 0, sizeof(TQBitmap *) * NumButtonIcons * 2);

  reset(0);
}

Q4Win10Handler::~Q4Win10Handler() {
  for (int t = 0; t < 2; ++t)
    for (int a = 0; a < 2; ++a)
      for (int i = 0; i < NumPixmaps; ++i)
        delete m_pixmaps[t][a][i];
  for (int t = 0; t < 2; ++t)
    for (int i = 0; i < NumButtonIcons; ++i)
      delete m_bitmaps[t][i];
}

bool Q4Win10Handler::reset(unsigned long changed) {
  // we assume the active font to be the same as the inactive font since the
  // control center doesn't offer different settings anyways.
  m_titleFont = KDecoration::options()->font(true, false);    // not small
  m_titleFontTool = KDecoration::options()->font(true, true); // small

  // Hardcode border size to minimum (1px)
  m_borderSize = 1;

  // check if we are in reverse layout mode
  m_reverse = TQApplication::reverseLayout();

  // read in the configuration
  readConfig();

  // pixmaps probably need to be updated, so delete the cache.
  for (int t = 0; t < 2; ++t) {
    for (int a = 0; a < 2; ++a) {
      for (int i = 0; i < NumPixmaps; i++) {
        if (m_pixmaps[t][a][i]) {
          delete m_pixmaps[t][a][i];
          m_pixmaps[t][a][i] = 0;
        }
      }
    }
  }
  for (int t = 0; t < 2; ++t) {
    for (int i = 0; i < NumButtonIcons; i++) {
      if (m_bitmaps[t][i]) {
        delete m_bitmaps[t][i];
        m_bitmaps[t][i] = 0;
      }
    }
  }

  // Do we need to "hit the wooden hammer" ?
  bool needHardReset = true;
  // TODO: besides the Color and Font settings I can maybe handle more changes
  //       without a hard reset. I will do this later...
  if (changed & SettingColors || changed & SettingFont) {
    needHardReset = false;
  } else if (changed & SettingButtons) {
    // handled by KCommonDecoration
    needHardReset = false;
  }

  if (needHardReset) {
    return true;
  } else {
    resetDecorations(changed);
    return false;
  }
}

KDecoration *Q4Win10Handler::createDecoration(KDecorationBridge *bridge) {
  return new Q4Win10Client(bridge, this);
}

bool Q4Win10Handler::supports(Ability ability) {
  switch (ability) {
  case AbilityAnnounceButtons:
  case AbilityButtonMenu:
  case AbilityButtonOnAllDesktops:
  case AbilityButtonSpacer:
  case AbilityButtonHelp:
  case AbilityButtonMinimize:
  case AbilityButtonMaximize:
  case AbilityButtonClose:
  case AbilityButtonAboveOthers:
  case AbilityButtonBelowOthers:
  case AbilityButtonShade:
    return true;
  default:
    return false;
  };
}

void Q4Win10Handler::readConfig() {
  // create a config object
  TDEConfig config("twinq4win10rc");
  config.reparseConfiguration(); // Ensure we have the latest values from disk
  config.setGroup("General");

  // grab settings
  // hardcoded defaults:
  // TitleShadow = false
  // AnimateButtons = false
  // TitleAlign = Left

  TQFontMetrics fm(m_titleFont); // active font = inactive font
  int titleHeightMin = config.readNumEntry("MinTitleHeight", 16);
  // The title should strech with bigger font sizes!
  m_titleHeight =
      TQMAX(titleHeightMin, fm.height() + 4); // 4 px for the shadow etc.
  // have an even title/button size so the button icons are fully centered...
  if (m_titleHeight % 2 == 0)
    m_titleHeight++;

  fm = TQFontMetrics(m_titleFontTool); // active font = inactive font
  int titleHeightToolMin = config.readNumEntry("MinTitleHeightTool", 13);
  // The title should strech with bigger font sizes!
  m_titleHeightTool = TQMAX(titleHeightToolMin,
                            fm.height()); // don't care about the shadow etc.
  // have an even title/button size so the button icons are fully centered...
  if (m_titleHeightTool % 2 == 0)
    m_titleHeightTool++;

  m_darkMode =
      config.readBoolEntry("DarkMode", false); // Default to false (Light Mode)
}

TQColor Q4Win10Handler::getColor(KWinQ4Win10::ColorType type,
                                 const bool active) {
  switch (type) {
  case WindowContour:
    // Windows 10 style:
    // Active: Seamless (matches TitleBar)
    // Inactive: Visible Border (Gray)
    if (active) {
      return KDecoration::options()->color(ColorTitleBar, true);
    } else {
      return m_darkMode ? TQt::lightGray : TQt::darkGray;
    }
  case TitleGradient1:
    return hsvRelative(KDecoration::options()->color(ColorTitleBar, true), 0,
                       -10, +10);
    break;
  case TitleGradient2:
    return hsvRelative(KDecoration::options()->color(ColorTitleBar, true), 0, 0,
                       -25);
    break;
  case TitleGradient3:
    return KDecoration::options()->color(ColorTitleBar, true);
    break;
  case ShadeTitleLight:
    return alphaBlendColors(
        KDecoration::options()->color(ColorTitleBar, active), TQt::white,
        active ? 205 : 215);
    break;
  case ShadeTitleDark:
    return alphaBlendColors(
        KDecoration::options()->color(ColorTitleBar, active), TQt::black,
        active ? 205 : 215);
    break;
  case Border:
    return KDecoration::options()->color(ColorFrame, active);
  case TitleFont:
    return KDecoration::options()->color(ColorFont, active);
  default:
    return TQt::black;
  }
}

void Q4Win10Handler::pretile(TQPixmap *&pix, int size,
                             TQt::Orientation dir) const {
  TQPixmap *newpix;
  TQPainter p;

  if (dir == TQt::Horizontal)
    newpix = new TQPixmap(size, pix->height());
  else
    newpix = new TQPixmap(pix->width(), size);

  p.begin(newpix);
  p.drawTiledPixmap(newpix->rect(), *pix);
  p.end();

  delete pix;
  pix = newpix;
}

const TQPixmap &Q4Win10Handler::pixmap(Pixmaps type, bool active,
                                       bool toolWindow) {
  if (m_pixmaps[toolWindow][active][type])
    return *m_pixmaps[toolWindow][active][type];

  TQPixmap *pm = 0;

  switch (type) {
  case TitleBarTileTop:
  case TitleBarTile: {
    const int titleBarTileHeight =
        (toolWindow ? m_titleHeightTool : m_titleHeight) + 2;
    // gradient used as well in TitleBarTileTop as TitleBarTile
    const int gradientHeight = 2 + titleBarTileHeight - 1;
    TQPixmap gradient(1, gradientHeight);
    TQPainter painter(&gradient);
    // Windows 10 Style: Flat Color.
    // Replace complex gradient with solid fill.
    // Use TitleGradient2 (slightly darker) or TitleGradient3 (base color).
    // Usually Windows 10 uses the base accent color.
    painter.fillRect(gradient.rect(), getColor(TitleGradient3, active));
    // (We leave the rest of the drawing logic, like top highlights, controlled
    // by the next block)
    painter.end();

    // actual titlebar tiles
    if (type == TitleBarTileTop) {
      pm = new TQPixmap(1, 4);
      painter.begin(pm);
      // contour
      painter.setPen(getColor(WindowContour, active));
      painter.drawPoint(0, 0);
      // top highlight
      painter.setPen(getColor(ShadeTitleLight, active));
      painter.drawPoint(0, 1);
      // gradient
      painter.drawPixmap(0, 2, gradient);
      painter.end();
    } else {
      pm = new TQPixmap(1, titleBarTileHeight);
      painter.begin(pm);
      painter.drawPixmap(0, 0, gradient, 0, 2);
      painter.setPen(getColor(TitleGradient3, active));
      painter.drawPoint(0, titleBarTileHeight - 1);
      painter.end();
    }

    pretile(pm, 64, TQt::Horizontal);

    break;
  }

  case TitleBarLeft: {
    const int w = m_borderSize;
    const int h = 4 + (toolWindow ? m_titleHeightTool : m_titleHeight) + 2;

    pm = new TQPixmap(w, h);
    TQPainter painter(pm);

    painter.drawTiledPixmap(0, 0, w, 4,
                            pixmap(TitleBarTileTop, active, toolWindow));
    painter.drawTiledPixmap(0, 4, w, h - 4,
                            pixmap(TitleBarTile, active, toolWindow));

    painter.setPen(getColor(WindowContour, active));
    painter.drawLine(0, 0, 0, h);
    painter.drawPoint(1, 1);

    const TQColor highlightTitleLeft = getColor(ShadeTitleLight, active);
    painter.setPen(highlightTitleLeft);
    painter.drawLine(1, 2, 1, h);

    // outside the region normally masked by doShape
    // Removed to ensure square corners
    // painter.setPen(TQColor(0, 0, 0));
    // painter.drawLine(0, 0, 1, 0);
    // painter.drawPoint(0, 1);

    break;
  }

  case TitleBarRight: {
    const int w = m_borderSize;
    const int h = 4 + (toolWindow ? m_titleHeightTool : m_titleHeight) + 2;

    pm = new TQPixmap(w, h);
    TQPainter painter(pm);

    painter.drawTiledPixmap(0, 0, w, 4,
                            pixmap(TitleBarTileTop, active, toolWindow));
    painter.drawTiledPixmap(0, 4, w, h - 4,
                            pixmap(TitleBarTile, active, toolWindow));

    painter.setPen(getColor(WindowContour, active));
    painter.drawLine(w - 1, 0, w - 1, h);
    painter.drawPoint(w - 2, 1);

    const TQColor highlightTitleRight = getColor(ShadeTitleDark, active);
    painter.setPen(highlightTitleRight);
    painter.drawLine(w - 2, 2, w - 2, h);

    // outside the region normally masked by doShape
    // Removed to ensure square corners
    // painter.setPen(TQColor(0, 0, 0));
    // painter.drawLine(w - 2, 0, w - 1, 0);
    // painter.drawPoint(w - 1, 1);

    break;
  }

  case BorderLeftTile: {
    const int w = m_borderSize;

    pm = new TQPixmap(w, 1);
    TQPainter painter(pm);

    // Dynamic Inactive Border Logic:
    // Active: Paint TitleBar Color (appears borderless/seamless)
    // Inactive: Paint 1px Grey Border
    if (active) {
      painter.setPen(
          getColor(TitleGradient3, true)); // Matches persistent title bar
    } else {
      // Inactive Border Color
      if (m_darkMode)
        painter.setPen(TQt::lightGray);
      else
        painter.setPen(TQt::darkGray);
    }

    painter.drawPoint(0, 0);
    // Draw optional second pixel if border > 1 (shouldn't be, since
    // m_borderSize=1)
    if (w > 1) {
      painter.drawLine(1, 0, w - 1, 0);
    }

    painter.end();

    pretile(pm, 64, TQt::Vertical);

    break;
  }

  case BorderRightTile: {
    const int w = m_borderSize;

    pm = new TQPixmap(w, 1);
    TQPainter painter(pm);

    // Dynamic Inactive Border Logic
    if (active) {
      painter.setPen(getColor(TitleGradient3, true));
    } else {
      if (m_darkMode)
        painter.setPen(TQt::lightGray);
      else
        painter.setPen(TQt::darkGray);
    }

    painter.drawPoint(w - 1, 0);
    // Fill rest if > 1
    if (w > 1) {
      painter.drawLine(0, 0, w - 2, 0);
    }

    painter.end();

    pretile(pm, 64, TQt::Vertical);

    break;
  }

  case BorderBottomLeft: {
    const int w = m_borderSize;
    const int h = m_borderSize;

    pm = new TQPixmap(w, h);
    TQPainter painter(pm);

    // Dynamic Inactive Border Logic
    if (active) {
      painter.setPen(getColor(TitleGradient3, true));
    } else {
      if (m_darkMode)
        painter.setPen(TQt::lightGray);
      else
        painter.setPen(TQt::darkGray);
    }
    // Draw L Shape (Left and Bottom)
    painter.drawLine(0, 0, 0, h - 1);         // Left
    painter.drawLine(0, h - 1, w - 1, h - 1); // Bottom

    painter.end();

    break;
  }

  case BorderBottomRight: {
    const int w = m_borderSize;
    const int h = m_borderSize;

    pm = new TQPixmap(w, h);
    TQPainter painter(pm);

    // Dynamic Inactive Border Logic
    if (active) {
      painter.setPen(getColor(TitleGradient3, true));
    } else {
      if (m_darkMode)
        painter.setPen(TQt::lightGray);
      else
        painter.setPen(TQt::darkGray);
    }
    // Draw L Shape (Right and Bottom)
    painter.drawLine(w - 1, 0, w - 1, h - 1); // Right
    painter.drawLine(0, h - 1, w - 1, h - 1); // Bottom

    painter.end();

    break;
  }

  case BorderBottomTile:
  default: {
    const int h = m_borderSize;

    pm = new TQPixmap(1, m_borderSize);
    TQPainter painter(pm);

    painter.setPen(getColor(Border, active));
    painter.drawLine(0, 0, 0, h - 3);
    painter.setPen(alphaBlendColors(getColor(Border, active),
                                    getColor(ShadeTitleDark, active), 130));
    painter.drawPoint(0, h - 2);
    painter.setPen(getColor(WindowContour, active));
    painter.drawPoint(0, h - 1);
    painter.end();

    pretile(pm, 64, TQt::Horizontal);

    break;
  }
  }

  m_pixmaps[toolWindow][active][type] = pm;
  return *pm;
}

const TQBitmap &Q4Win10Handler::buttonBitmap(ButtonIcon type,
                                             const TQSize &size,
                                             bool toolWindow) {
  int typeIndex = type;

  // btn icon size...
  int reduceW = 0, reduceH = 0;
  if (size.width() > 14) {
    reduceW = static_cast<int>(2 * (size.width() / 3.5));
  } else
    reduceW = 6;
  if (size.height() > 14)
    reduceH = static_cast<int>(2 * (size.height() / 3.5));
  else
    reduceH = 6;

  int w = size.width() - reduceW;
  int h = size.height() - reduceH;

  if (m_bitmaps[toolWindow][typeIndex] &&
      m_bitmaps[toolWindow][typeIndex]->size() == TQSize(w, h))
    return *m_bitmaps[toolWindow][typeIndex];

  // no matching pixmap found, create a new one...

  delete m_bitmaps[toolWindow][typeIndex];
  m_bitmaps[toolWindow][typeIndex] = 0;

  TQBitmap bmp = IconEngine::icon(type /*icon*/, TQMIN(w, h));
  TQBitmap *bitmap = new TQBitmap(bmp);
  m_bitmaps[toolWindow][typeIndex] = bitmap;
  return *bitmap;
}

TQValueList<Q4Win10Handler::BorderSize> Q4Win10Handler::borderSizes() const {
  // Only allow Tiny border (Hardcoded) which effectively disables the dropdown
  // choice
  return TQValueList<BorderSize>() << BorderTiny;
}

// make the handler accessible to other classes...
static Q4Win10Handler *handler = 0;
Q4Win10Handler *Handler() { return handler; }

} // namespace KWinQ4Win10

//////////////////////////////////////////////////////////////////////////////
// Plugin Stuff                                                             //
//////////////////////////////////////////////////////////////////////////////

extern "C" {
TDE_EXPORT KDecorationFactory *create_factory() {
  KWinQ4Win10::handler = new KWinQ4Win10::Q4Win10Handler();
  return KWinQ4Win10::handler;
}
}
