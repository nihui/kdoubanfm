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

#ifndef TRAYICON_H
#define TRAYICON_H

#include <KStatusNotifierItem>
#include <QList>
#include <qjson/parser.h>

#include <Phonon/MediaObject>
#include <Phonon/AudioOutput>

class QAction;
class QActionGroup;
class QMenu;
class KAction;
class KJob;
class Mpris2;

class SongInfo
{
public:
    QString title;
    QString artist;
    QString albumtitle;
    QString picture;
    QString url;
    int length;
    int sid;
    int aid;
    int like;

    // flag for full-played or skipped
    mutable bool fullplayed;
};

class TrayIcon : public KStatusNotifierItem
{
    Q_OBJECT
public:
    explicit TrayIcon();
    virtual ~TrayIcon();

    void login();
    void channels();
    void playlist(const QList<SongInfo>& history = QList<SongInfo>());
    void like(int sid);
    void unlike(int sid);
    void ban(int sid);
    void end(int sid);

private Q_SLOTS:
    void init();

    void slot_login(KJob* job);
    void slot_channels(KJob* job);
    void slot_playlist(KJob* job);
    void slot_end(KJob* job);

    void slotPauseAction();
    void slotStopAction();
    void slotSkipAction();
    void slotLikeAction();
    void slotBanAction();
    void slotAccountAction();
    void slotAboutAction();

    void slotChannelAction(QAction* action);

    void slotMediaAboutToFinish();
    void slotMediaCurrentSourceChanged();
    void slotMeidaStateChanged(Phonon::State newstate);

    void slotAlbumPicture(KJob* job);

private:
    friend class Mpris2;

    QJson::Parser m_parser;

    // account info
    QByteArray m_accountEmail;
    QByteArray m_accountPassword;

    // auth info
    QByteArray m_userid;
    QByteArray m_token;
    QByteArray m_expire;
    QByteArray m_username;
    QByteArray m_email;

    // actions
    KAction* pauseAction;
    KAction* skipAction;
    KAction* likeAction;
    KAction* banAction;

    // channel group
    QMenu* m_channelMenu;
    QActionGroup* m_channelAct;
    int m_channelId;
    QAction* m_redHeartMHz;

    // playlist
    QList<SongInfo> m_playlist;
    QList<SongInfo> m_playlistNext;
    int m_playIndex;
    int m_playIndexNext;

    // download album picture
    KJob* m_albumPictureJob;

    // media player
    Phonon::MediaObject* m_media;
    Phonon::AudioOutput* m_output;
};

#endif // TRAYICON_H
