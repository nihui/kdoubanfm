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

#ifndef MPRIS2_H
#define MPRIS2_H

#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <Phonon/MediaObject>

class TrayIcon;
class QDBusObjectPath;

class Mpris2 : public QObject
{
    Q_OBJECT

    // org.mpris.MediaPlayer2 MPRIS 2.0 Root interface
    Q_PROPERTY( bool CanQuit READ CanQuit )
    Q_PROPERTY( bool CanRaise READ CanRaise )
    Q_PROPERTY( bool HasTrackList READ HasTrackList )
    Q_PROPERTY( QString Identity READ Identity )
    Q_PROPERTY( QString DesktopEntry READ DesktopEntry )
    Q_PROPERTY( QStringList SupportedUriSchemes READ SupportedUriSchemes )
    Q_PROPERTY( QStringList SupportedMimeTypes READ SupportedMimeTypes )

    // org.mpris.MediaPlayer2.Player MPRIS 2.0 Player interface
    Q_PROPERTY( QString PlaybackStatus READ PlaybackStatus )
    Q_PROPERTY( QString LoopStatus READ LoopStatus WRITE SetLoopStatus )
    Q_PROPERTY( double Rate READ Rate WRITE SetRate )
    Q_PROPERTY( bool Shuffle READ Shuffle WRITE SetShuffle )
    Q_PROPERTY( QVariantMap Metadata READ Metadata )
    Q_PROPERTY( double Volume READ Volume WRITE SetVolume )
    Q_PROPERTY( qlonglong Position READ Position )
    Q_PROPERTY( double MinimumRate READ MinimumRate )
    Q_PROPERTY( double MaximumRate READ MaximumRate )
    Q_PROPERTY( bool CanGoNext READ CanGoNext )
    Q_PROPERTY( bool CanGoPrevious READ CanGoPrevious )
    Q_PROPERTY( bool CanPlay READ CanPlay )
    Q_PROPERTY( bool CanPause READ CanPause )
    Q_PROPERTY( bool CanSeek READ CanSeek )
    Q_PROPERTY( bool CanControl READ CanControl )

public:
    explicit Mpris2(TrayIcon* trayicon);
    virtual ~Mpris2();

    // PropertiesChanged
    void EmitPropertyChanged(const QString& name, const QVariant& value);

    // Root Properties
    bool CanQuit() const { return true; }
    bool CanRaise() const { return false; }
    bool HasTrackList() const { return false; }
    QString Identity() const { return "KDoubanFM"; }
    QString DesktopEntry() const { return "kdoubanfm"; }
    QStringList SupportedUriSchemes() const { return QStringList(); }
    QStringList SupportedMimeTypes() const { return QStringList(); }

    // Root Methods
    void Raise() {}
    void Quit();

    // Player Properties
    QString PlaybackStatus() const;
    QString LoopStatus() const { return "None"; }
    void SetLoopStatus(const QString& value) { Q_UNUSED(value) }
    double Rate() const { return 1.0; }
    void SetRate(double value) { Q_UNUSED(value) }
    bool Shuffle() const { return false; }
    void SetShuffle(bool value) { Q_UNUSED(value) }
    QVariantMap Metadata() const;
    double Volume() const;
    void SetVolume(double value);
    qlonglong Position() const;
    double MaximumRate() const { return 1.0; }
    double MinimumRate() const { return 1.0; }
    bool CanGoNext() const { return true; }
    bool CanGoPrevious() const { return false; }
    bool CanPlay() const { return true; }
    bool CanPause() const { return true; }
    bool CanSeek() const;
    bool CanControl() const { return true; }

    // Player Methods
    void Next();
    void Previous() {}
    void Pause();
    void PlayPause();
    void Stop();
    void Play();
    void Seek(qlonglong offset);
    void SetPosition(const QDBusObjectPath& trackId, qlonglong position);
    void OpenUri(const QString& uri) { Q_UNUSED(uri) }

Q_SIGNALS:
    // Player Signals
    void Seeked(qlonglong offset);

private Q_SLOTS:
    void slotCurrentSourceChanged();
    void slotStateChanged(Phonon::State, Phonon::State);
    void slotVolumeChanged(qreal);

private:
    TrayIcon* m_trayicon;
};

#endif // MPRIS2_H
