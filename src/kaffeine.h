/*
 * kaffeine.h
 *
 * Copyright (C) 2007 Christoph Pfister <christophpfister@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef KAFFEINE_H
#define KAFFEINE_H

#include <KCmdLineArgs>
#include <KXmlGuiWindow>

class Manager;
class MediaWidget;

class Kaffeine : public KXmlGuiWindow
{
	Q_OBJECT
public:
	Kaffeine();
	~Kaffeine();

	static KCmdLineOptions cmdLineOptions();

	void parseArgs();

private slots:
	void actionFullscreen();
	void actionOpen();
	void actionOpenUrl();
	void actionOpenRecent(const KUrl &url);
	void actionOpenAudioCd();
	void actionOpenVideoCd();
	void actionOpenDvd();
	void actionPlayPause(bool paused);

private:
	Manager *manager;
	MediaWidget *mediaWidget;
};

#endif /* KAFFEINE_H */