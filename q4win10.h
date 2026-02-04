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

#ifndef Q4WIN10_H
#define Q4WIN10_H

#include <tqcolor.h>
#include <tqfont.h>

#include <kdecoration.h>
#include <kdecorationfactory.h>

namespace KWinQ4Win10 {

inline TQColor hsvRelative(const TQColor &baseColor, int relativeH,
                           int relativeS, int relativeV) {
  int h, s, v;
  baseColor.hsv(&h, &s, &v);
  h += relativeH;
  s += relativeS;
  v += relativeV;
  if (h < 0) {
    h = 0;
  } else if (h > 359) {
    h = 359;
  }
  if (s < 0) {
    s = 0;
  } else if (s > 255) {
    s = 255;
  }
  if (v < 0) {
    v = 0;
  } else if (v > 255) {
    v = 255;
  }
  TQColor c;
  c.setHsv(h, s, v);
  return c;
}

inline TQColor alphaBlendColors(const TQColor &bgColor, const TQColor &fgColor,
                                const int a) {
  TQRgb rgb = bgColor.rgb();
  TQRgb rgb_b = fgColor.rgb();
  int alpha = a;
  if (alpha > 255)
    alpha = 255;
  if (alpha < 0)
    alpha = 0;
  int inv_alpha = 255 - alpha;
  return TQColor(
      tqRgb(tqRed(rgb_b) * inv_alpha / 255 + tqRed(rgb) * alpha / 255,
            tqGreen(rgb_b) * inv_alpha / 255 + tqGreen(rgb) * alpha / 255,
            tqBlue(rgb_b) * inv_alpha / 255 + tqBlue(rgb) * alpha / 255));
}

enum ColorType {
  WindowContour = 0,
  TitleGradient1, // top
  TitleGradient2,
  TitleGradient3, // bottom
  ShadeTitleLight,
  ShadeTitleDark,
  Border,
  TitleFont
};

enum Pixmaps {
  TitleBarTileTop = 0,
  TitleBarTile,
  TitleBarLeft,
  TitleBarRight,
  BorderLeftTile,
  BorderRightTile,
  BorderBottomTile,
  BorderBottomLeft,
  BorderBottomRight,
  NumPixmaps
};

enum ButtonIcon {
  CloseIcon = 0,
  MaxIcon,
  MaxRestoreIcon,
  MinIcon,
  HelpIcon,
  OnAllDesktopsIcon,
  NotOnAllDesktopsIcon,
  KeepAboveIcon,
  NoKeepAboveIcon,
  KeepBelowIcon,
  NoKeepBelowIcon,
  ShadeIcon,
  UnShadeIcon,
  NumButtonIcons
};

class Q4Win10Handler : public TQObject, public KDecorationFactory {
  TQ_OBJECT
public:
  Q4Win10Handler();
  ~Q4Win10Handler();
  virtual bool reset(unsigned long changed);

  virtual KDecoration *createDecoration(KDecorationBridge *);
  virtual bool supports(Ability ability);

  const TQPixmap &pixmap(Pixmaps type, bool active, bool toolWindow);
  const TQBitmap &buttonBitmap(ButtonIcon type, const TQSize &size,
                               bool toolWindow);

  int titleHeight() { return m_titleHeight; }
  int titleHeightTool() { return m_titleHeightTool; }
  const TQFont &titleFont() { return m_titleFont; }
  const TQFont &titleFontTool() { return m_titleFontTool; }
  bool titleShadow() { return false; }
  int borderSize() { return m_borderSize; }
  bool animateButtons() { return false; }
  bool menuClose() { return true; } // Hardcoded to true
  bool darkMode() { return m_darkMode; }
  TQt::AlignmentFlags titleAlign() { return TQt::AlignLeft; }
  bool reverseLayout() { return m_reverse; }
  TQColor getColor(KWinQ4Win10::ColorType type, const bool active = true);

  TQValueList<Q4Win10Handler::BorderSize> borderSizes() const;
  void readConfig();

private:
  void pretile(TQPixmap *&pix, int size, TQt::Orientation dir) const;

  // Removed unused members: m_coloredBorder, m_titleShadow, m_animateButtons,
  // m_menuClose

  bool m_darkMode;
  bool m_reverse;
  int m_borderSize;
  int m_titleHeight;
  int m_titleHeightTool;
  TQFont m_titleFont;
  TQFont m_titleFontTool;
  TQt::AlignmentFlags m_titleAlign;

  // pixmap cache
  TQPixmap *m_pixmaps[2][2][NumPixmaps]; // button pixmaps have normal+pressed
                                         // state...
  TQBitmap *m_bitmaps[2][NumButtonIcons];
};

Q4Win10Handler *Handler();

} // namespace KWinQ4Win10

#endif // Q4WIN10_H
