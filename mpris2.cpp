/*
 *  This file is part of KDoubanFM, Douban FM Client
 *  Copyright (C) 2012 Ni Hui <shuizhuyuanluo@126.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mpris2.h"

#include <KApplication>
#include <KDebug>

#include "trayicon.h"
#include "mprisadaptor.h"
#include "mprisplayeradaptor.h"

Mpris2::Mpris2(TrayIcon* trayicon) : QObject(trayicon)
{
    m_trayicon = trayicon;

    if (!QDBusConnection::sessionBus().registerService("org.mpris.MediaPlayer2.kdoubanfm")) {
        kWarning() << "registerService failed";
        return;
    }

    new MprisAdaptor(this);
    new MprisPlayerAdaptor(this);

    QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", this);

    connect(m_trayicon->m_media, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
            this, SLOT(slotCurrentSourceChanged()));
    connect(m_trayicon->m_media, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(slotStateChanged(Phonon::State,Phonon::State)));
    connect(m_trayicon->m_output, SIGNAL(volumeChanged(qreal)),
            this, SLOT(slotVolumeChanged(qreal)));
}

Mpris2::~Mpris2()
{
}

void Mpris2::EmitPropertyChanged(const QString& name, const QVariant& value)
{
    QDBusMessage msg = QDBusMessage::createSignal("/org/mpris/MediaPlayer2",
                                                  "org.freedesktop.DBus.Properties",
                                                  "PropertiesChanged");

    QVariantMap map;
    map.insert(name, value);

    QVariantList args;
    args << "org.mpris.MediaPlayer2.Player" << map << QStringList();

    msg.setArguments(args);
    QDBusConnection::sessionBus().send(msg);
}

void Mpris2::Quit()
{
    kapp->quit();
}

QString Mpris2::PlaybackStatus() const
{
    switch (m_trayicon->m_media->state()) {
        case Phonon::PlayingState:
            return "Playing";
        case Phonon::PausedState:
            return "Paused";
        case Phonon::StoppedState:
        default:
            return "Stopped";
    }
}

QVariantMap Mpris2::Metadata() const
{
    QVariantMap map;

    if (m_trayicon->m_playIndex == -1)
        return map;

    const SongInfo& si = m_trayicon->m_playlist.at(m_trayicon->m_playIndex);
    map["mpris:trackid"] = "/org/mpris/MediaPlayer2/Track/" + QString::number(si.sid);
    map["mpris:length"] = qlonglong(si.length) * 1000 * 1000;
    map["mpris:artUrl"] = QString(si.picture).replace("/mpic/", "/lpic/");
    map["xesam:album"] = si.albumtitle;
    map["xesam:artist"] = QStringList() << si.artist;
    map["xesam:title"] = si.title;
    map["xesam:url"] = si.url;
    return map;
}

double Mpris2::Volume() const
{
    return m_trayicon->m_output->volume();
}

void Mpris2::SetVolume(double value)
{
    m_trayicon->m_output->setVolume(value);
}

qlonglong Mpris2::Position() const
{
    return m_trayicon->m_media->currentTime() * 1000;
}

bool Mpris2::CanSeek() const
{
    return m_trayicon->m_media->isSeekable();
}

void Mpris2::Next()
{
    m_trayicon->slotSkipAction();
}

void Mpris2::Pause()
{
    m_trayicon->m_media->pause();
}

void Mpris2::PlayPause()
{
    m_trayicon->slotPauseAction();
}

void Mpris2::Stop()
{
    m_trayicon->slotStopAction();
}

void Mpris2::Play()
{
    if (m_trayicon->m_media->state() == Phonon::StoppedState) {
        m_trayicon->playlist();
    }
    else {
        m_trayicon->m_media->play();
    }
}

void Mpris2::Seek(qlonglong offset)
{
    qlonglong position = Position() + offset;

    if (position < 0)
        position = 0;

    if (m_trayicon->m_playIndex == -1)
        return;

    const SongInfo& si = m_trayicon->m_playlist.at(m_trayicon->m_playIndex);
    if (position > si.length * 1000 * 1000)
        Next();

    m_trayicon->m_media->seek(position / 1000);

    /// FIXME: make this async
    emit Seeked(position);
}

void Mpris2::SetPosition(const QDBusObjectPath& trackId, qlonglong position)
{
    const SongInfo& si = m_trayicon->m_playlist.at(m_trayicon->m_playIndex);
    QString currentTrackId = "/org/mpris/MediaPlayer2/Track/" + QString::number(si.sid);

    if (trackId.path() != currentTrackId)
        return;

    if (m_trayicon->m_playIndex == -1)
        return;

    if (position < 0 || position > si.length * 1000 * 1000)
        return;

    m_trayicon->m_media->seek(position / 1000);

    /// FIXME: make this async
    emit Seeked(position);
}

void Mpris2::slotCurrentSourceChanged()
{
    EmitPropertyChanged("Metadata", Metadata());
    EmitPropertyChanged("CanSeek", CanSeek());
}

void Mpris2::slotStateChanged(Phonon::State /*newState*/, Phonon::State /*oldState*/)
{
    EmitPropertyChanged("PlaybackStatus", PlaybackStatus());
}

void Mpris2::slotVolumeChanged(qreal /*newVolume*/)
{
    EmitPropertyChanged("Volume", Volume());
}
