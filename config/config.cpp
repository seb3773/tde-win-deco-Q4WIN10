/* Plastik KWin window decoration
  Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>

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

#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqradiobutton.h>
#include <tqslider.h>
#include <tqspinbox.h>
#include <tqwhatsthis.h>
#include <tqlabel.h>
#include <ntqlayout.h>

#include <tdeconfig.h>
#include <tdeglobal.h>
#include <tdelocale.h>

#include <tqfont.h>
#include <tqimage.h>
#include <tqpixmap.h>

#include "config.h"
#include "configdialog.h"
#include "q4win10_version.h"
#include "q4win10_logo.h"

Q4Win10Config::Q4Win10Config(TDEConfig *config, TQWidget *parent)
    : TQObject(parent), m_config(0), m_dialog(0) {
  // create the configuration object
  m_config = new TDEConfig("twinq4win10rc");
  TDEGlobal::locale()->insertCatalogue("twin_clients");

  // create and show the configuration dialog
  TQVBoxLayout *layout = new TQVBoxLayout(parent);
  m_dialog = new ConfigDialog(parent);
  m_dialog->labelVersion->setText(Q4WIN10_VERSION_STRING);

  // Title: Bold & slightly larger
  TQFont titleFont = m_dialog->labelTitle->font();
  titleFont.setBold(true);
  if (titleFont.pointSize() > 0) {
    titleFont.setPointSize(titleFont.pointSize() + 2);
  } else if (titleFont.pixelSize() > 0) {
    titleFont.setPixelSize(titleFont.pixelSize() + 3);
  }
  m_dialog->labelTitle->setFont(titleFont);

  // Smooth-scale embedded logo to 96x56 at runtime
  TQPixmap logoPixmap;
  if (logoPixmap.loadFromData(q4win10_logo_png, q4win10_logo_png_len, "PNG")) {
    TQImage img = logoPixmap.convertToImage().smoothScale(96, 56);
    logoPixmap.convertFromImage(img);
    m_dialog->labelLogo->setPixmap(logoPixmap);
  }

  layout->addWidget(m_dialog);
  m_dialog->show();

  // load the configuration
  load(config);

  // setup the connections
  connect(m_dialog->darkMode, TQT_SIGNAL(toggled(bool)), this,
          TQT_SIGNAL(changed()));
  connect(m_dialog->win11Mode, TQT_SIGNAL(toggled(bool)), this,
          TQT_SIGNAL(changed()));
}

Q4Win10Config::~Q4Win10Config() {
  if (m_dialog)
    delete m_dialog;
  if (m_config)
    delete m_config;
}

void Q4Win10Config::load(TDEConfig *) {
  m_config->setGroup("General");
  bool darkMode = m_config->readBoolEntry("DarkMode", false);
  m_dialog->darkMode->setChecked(darkMode);

  bool win11Mode = m_config->readBoolEntry("Win11Mode", false);
  m_dialog->win11Mode->setChecked(win11Mode);
}

void Q4Win10Config::save(TDEConfig *) {
  m_config->setGroup("General");

  m_config->writeEntry("DarkMode", m_dialog->darkMode->isChecked());
  m_config->writeEntry("Win11Mode", m_dialog->win11Mode->isChecked());
  m_config->sync();
}

void Q4Win10Config::defaults() {
  m_dialog->darkMode->setChecked(false);
  m_dialog->win11Mode->setChecked(false);
}

//////////////////////////////////////////////////////////////////////////////
// Plugin Stuff                                                             //
//////////////////////////////////////////////////////////////////////////////

#ifndef TDE_EXPORT
#define TDE_EXPORT KDE_EXPORT
#endif

extern "C" {
TDE_EXPORT TQObject *allocate_config(TDEConfig *config, TQWidget *parent) {
  return (new Q4Win10Config(config, parent));
}
}

#include "config.moc"
