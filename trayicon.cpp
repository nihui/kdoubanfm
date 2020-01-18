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

#include "trayicon.h"

#include <QDialog>
#include <QTimer>
#include <QUrl>

#include <KAboutApplicationDialog>
#include <KAction>
#include <KComponentData>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KIcon>
#include <KIO/Job>
#include <KIO/StoredTransferJob>
#include <KLocale>
#include <KMenu>
#include <KShortcut>

#include "mpris2.h"
#include "ui_account.h"

TrayIcon::TrayIcon() : KStatusNotifierItem("kdoubanfm")
{
    likeAction = 0;
    m_channelMenu = 0;
    m_channelAct = 0;
    m_channelId = 1;
    m_playIndex = -1;
    m_playIndexNext = 0;
    m_albumPictureJob = 0;
    m_media = 0;
    m_output = 0;

    setAssociatedWidget(contextMenu());
    setIconByName("kdoubanfm");
//     setTitle(i18n("KDoubanFM"));
    setToolTipIconByName("kdoubanfm");
    setToolTipTitle(i18n("KDoubanFM"));
    setToolTipSubTitle(i18n("Douban FM"));
    setCategory(KStatusNotifierItem::ApplicationStatus);
    setStatus(KStatusNotifierItem::Active);

    // lazy initialization
    QTimer::singleShot(0, this, SLOT(init()));
}

TrayIcon::~TrayIcon()
{
}

void TrayIcon::login()
{
    QUrl apiUrl("http://www.douban.com/j/app/login");

    // login info
    QUrl postUrl;
    postUrl.addEncodedQueryItem("email", m_accountEmail);
    postUrl.addEncodedQueryItem("password", m_accountPassword);
    postUrl.addEncodedQueryItem("app_name", "radio_desktop_win");
    postUrl.addEncodedQueryItem("version", "100");

    KIO::TransferJob* job = KIO::storedHttpPost(postUrl.encodedQuery(), apiUrl, KIO::HideProgressInfo);
    job->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slot_login(KJob*)));
    job->start();
}

void TrayIcon::channels()
{
    QUrl apiUrl("http://www.douban.com/j/app/radio/channels");

    KIO::TransferJob* job = KIO::storedGet(apiUrl, KIO::Reload, KIO::HideProgressInfo);
    job->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slot_channels(KJob*)));
    job->start();
}

void TrayIcon::playlist(const QList<SongInfo>& history)
{
    QUrl apiUrl("http://www.douban.com/j/app/radio/people");

    // auth info
    apiUrl.addEncodedQueryItem("user_id", m_userid);
    apiUrl.addEncodedQueryItem("expire", m_expire);
    apiUrl.addEncodedQueryItem("token", m_token);
    apiUrl.addEncodedQueryItem("app_name", "radio_desktop_win");
    apiUrl.addEncodedQueryItem("version", "100");

    // playlist info
    apiUrl.addEncodedQueryItem("type", history.isEmpty() ? "n" : "p");// new playlist
    apiUrl.addEncodedQueryItem("channel", QByteArray::number(m_channelId));
    if (!history.isEmpty()) {
        const SongInfo& esi = history.last();
        apiUrl.addEncodedQueryItem("sid", QByteArray::number(esi.sid));// last sid
        // create history parameter
        QStringList hes;
        foreach (const SongInfo& si, history) {
            hes << QString::number(si.sid) + ':' + (si.fullplayed ? 'p' : 's');
        }
        apiUrl.addEncodedQueryItem("h", QUrl::toPercentEncoding(hes.join("|")));// history
    }
    else {
        apiUrl.addEncodedQueryItem("h", "");// empty history
    }
    apiUrl.addEncodedQueryItem("r", "weq24d903jj");// random 10 bytes here

    KIO::TransferJob* job = KIO::storedGet(apiUrl, KIO::Reload, KIO::HideProgressInfo);
    job->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slot_playlist(KJob*)));
    job->start();
}

void TrayIcon::like(int sid)
{
    QUrl apiUrl("http://www.douban.com/j/app/radio/people");

    // auth info
    apiUrl.addEncodedQueryItem("user_id", m_userid);
    apiUrl.addEncodedQueryItem("expire", m_expire);
    apiUrl.addEncodedQueryItem("token", m_token);
    apiUrl.addEncodedQueryItem("app_name", "radio_desktop_win");
    apiUrl.addEncodedQueryItem("version", "100");

    // playlist info
    apiUrl.addEncodedQueryItem("type", "r");// rate
    apiUrl.addEncodedQueryItem("channel", QByteArray::number(m_channelId));
    apiUrl.addEncodedQueryItem("sid", QByteArray::number(sid));// song id
    apiUrl.addEncodedQueryItem("r", "zz343903oo");// random 10 bytes here

    KIO::TransferJob* job = KIO::storedGet(apiUrl, KIO::Reload, KIO::HideProgressInfo);
    job->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slot_playlist(KJob*)));
    job->start();
}

void TrayIcon::unlike(int sid)
{
    QUrl apiUrl("http://www.douban.com/j/app/radio/people");

    // auth info
    apiUrl.addEncodedQueryItem("user_id", m_userid);
    apiUrl.addEncodedQueryItem("expire", m_expire);
    apiUrl.addEncodedQueryItem("token", m_token);
    apiUrl.addEncodedQueryItem("app_name", "radio_desktop_win");
    apiUrl.addEncodedQueryItem("version", "100");

    // playlist info
    apiUrl.addEncodedQueryItem("type", "u");// unrate
    apiUrl.addEncodedQueryItem("channel", QByteArray::number(m_channelId));
    apiUrl.addEncodedQueryItem("sid", QByteArray::number(sid));// song id
    apiUrl.addEncodedQueryItem("r", "wrggx34f55");// random 10 bytes here

    KIO::TransferJob* job = KIO::storedGet(apiUrl, KIO::Reload, KIO::HideProgressInfo);
    job->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slot_playlist(KJob*)));
    job->start();
}

void TrayIcon::ban(int sid)
{
    QUrl apiUrl("http://www.douban.com/j/app/radio/people");

    // auth info
    apiUrl.addEncodedQueryItem("user_id", m_userid);
    apiUrl.addEncodedQueryItem("expire", m_expire);
    apiUrl.addEncodedQueryItem("token", m_token);
    apiUrl.addEncodedQueryItem("app_name", "radio_desktop_win");
    apiUrl.addEncodedQueryItem("version", "100");

    // playlist info
    apiUrl.addEncodedQueryItem("type", "b");// ban
    apiUrl.addEncodedQueryItem("channel", QByteArray::number(m_channelId));
    apiUrl.addEncodedQueryItem("sid", QByteArray::number(sid));// song id
    apiUrl.addEncodedQueryItem("r", "3094fjidse");// random 10 bytes here

    KIO::TransferJob* job = KIO::storedGet(apiUrl, KIO::Reload, KIO::HideProgressInfo);
    job->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slot_playlist(KJob*)));
    job->start();
}

void TrayIcon::end(int sid)
{
    QUrl apiUrl("http://www.douban.com/j/app/radio/people");

    // auth info
    apiUrl.addEncodedQueryItem("user_id", m_userid);
    apiUrl.addEncodedQueryItem("expire", m_expire);
    apiUrl.addEncodedQueryItem("token", m_token);
    apiUrl.addEncodedQueryItem("app_name", "radio_desktop_win");
    apiUrl.addEncodedQueryItem("version", "100");

    // playlist info
    apiUrl.addEncodedQueryItem("type", "e");// end
    apiUrl.addEncodedQueryItem("channel", QByteArray::number(m_channelId));
    apiUrl.addEncodedQueryItem("sid", QByteArray::number(sid));// song id
    apiUrl.addEncodedQueryItem("r", "ie93ddlaaw");// random 10 bytes here

    KIO::TransferJob* job = KIO::storedGet(apiUrl, KIO::Reload, KIO::HideProgressInfo);
    job->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slot_end(KJob*)));
    job->start();
}

#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>
#include "kiogetdevice.h"

void TrayIcon::init()
{

    Phonon::MediaObject *mediaObject = new Phonon::MediaObject(this);
    Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    Phonon::createPath(mediaObject, audioOutput);
    KIOGetDevice* reply = new KIOGetDevice(this);
    mediaObject->setCurrentSource(reply);
    mediaObject->play();
    return;

    // setup menu actions
    pauseAction = new KAction(KIcon("media-playback-start"), i18n("&Play"), this);
    pauseAction->setObjectName("kdoubanfm-pause");
    pauseAction->setGlobalShortcut(KShortcut(Qt::Key_MediaPlay));
    connect(pauseAction, SIGNAL(triggered()), this, SLOT(slotPauseAction()));
    contextMenu()->addAction(pauseAction);

    skipAction = new KAction(KIcon("media-skip-forward"), i18n("&Skip"), this);
    skipAction->setObjectName("kdoubanfm-skip");
    skipAction->setGlobalShortcut(KShortcut(Qt::Key_MediaNext));
    connect(skipAction, SIGNAL(triggered()), this, SLOT(slotSkipAction()));
    contextMenu()->addAction(skipAction);

    likeAction = new KAction(KIcon("emblem-favorite"), i18n("&Like"), this);
    likeAction->setEnabled(false);
    likeAction->setCheckable(true);
    connect(likeAction, SIGNAL(triggered()), this, SLOT(slotLikeAction()));
    contextMenu()->addAction(likeAction);

    banAction = new KAction(KIcon("edit-bomb"), i18n("&Ban"), this);
    banAction->setEnabled(false);
    connect(banAction, SIGNAL(triggered()), this, SLOT(slotBanAction()));
    contextMenu()->addAction(banAction);

    m_channelMenu = contextMenu()->addMenu(KIcon("view-statistics"), i18n("Channel"));

    KAction* accountAction = new KAction(KIcon("im-user"), i18n("A&ccount..."), this);
    connect(accountAction, SIGNAL(triggered()), this, SLOT(slotAccountAction()));
    contextMenu()->addAction(accountAction);

    KAction* aboutAction = new KAction(KIcon("kdoubanfm"), i18n("&About..."), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(slotAboutAction()));
    contextMenu()->addAction(aboutAction);

    m_channelAct = new QActionGroup(this);
    m_channelAct->setExclusive(true);
    connect(m_channelAct, SIGNAL(triggered(QAction*)), this, SLOT(slotChannelAction(QAction*)));

    // predefined red heart channel
    m_redHeartMHz = m_channelMenu->addAction(i18n("Red Heart MHz"));
    m_redHeartMHz->setCheckable(true);
    m_redHeartMHz->setData(-3);
    m_redHeartMHz->setEnabled(false);
    m_channelAct->addAction(m_redHeartMHz);

    // setup media player
    m_media = new Phonon::MediaObject(this);
    m_output = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    Phonon::createPath(m_media, m_output);

    connect(m_media, SIGNAL(aboutToFinish()), this, SLOT(slotMediaAboutToFinish()));
    connect(m_media, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
            this, SLOT(slotMediaCurrentSourceChanged()));
    connect(m_media, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(slotMeidaStateChanged(Phonon::State)));

    // mpris object
    new Mpris2(this);

    // load account info
    KConfigGroup cg(KGlobal::config(), "Account");
    m_accountEmail = cg.readEntry("Email", QByteArray());
    m_accountPassword = cg.readEntry("Password", QByteArray());

    if (!m_accountEmail.isEmpty() && !m_accountPassword.isEmpty()) {
        // let us login now
        login();
    }

    // get channel sub menu entries
    channels();
}

void TrayIcon::slot_login(KJob* job)
{
    if (job->error()) {
        kWarning() << "Job Error: " << job->errorString();
        return;
    }

    KIO::StoredTransferJob* j = static_cast<KIO::StoredTransferJob*>(job);

//     kWarning() << QString::fromUtf8(j->data());

    bool ok;
    QVariantMap map = m_parser.parse(j->data(), &ok).toMap();
    if (!ok) {
        kWarning() << "parse error";
        return;
    }

    int ret = map["r"].toInt();
    if (ret != 0) {
        /// login error here
        kWarning() << "login error";
        return;
    }

    m_userid = map["user_id"].toByteArray();
    m_token = map["token"].toByteArray();
    m_expire = map["expire"].toByteArray();
    m_username = map["user_name"].toByteArray();
    m_email = map["email"].toByteArray();

    // request initial playlist
    playlist();

    likeAction->setEnabled(true);
    banAction->setEnabled(true);

    m_redHeartMHz->setEnabled(true);
}

void TrayIcon::slot_channels(KJob* job)
{
    if (job->error()) {
        kWarning() << "Job Error: " << job->errorString();
        return;
    }

    KIO::StoredTransferJob* j = static_cast<KIO::StoredTransferJob*>(job);

//     kWarning() << QString::fromUtf8(j->data());

    bool ok;
    QVariantMap map = m_parser.parse(j->data(), &ok).toMap();
    if (!ok) {
        kWarning() << "parse error";
        return;
    }

    QVariantList channellist = map["channels"].toList();
    foreach (const QVariant& channel, channellist) {
        QVariantMap channelmap = channel.toMap();

        QString name = channelmap["name"].toString();
//         int seq_id = channelmap["seq_id"].toInt();
        int channel_id = channelmap["channel_id"].toInt();
//         kWarning() << name << seq_id << channel_id;
        QAction* channelAction = m_channelMenu->addAction(name);
        channelAction->setCheckable(true);
        channelAction->setData(channel_id);
        if (m_channelId == channel_id) {
            channelAction->setChecked(true);
        }
        m_channelAct->addAction(channelAction);
    }
}

void TrayIcon::slot_playlist(KJob* job)
{
    if (job->error()) {
        kWarning() << "Job Error: " << job->errorString();
        return;
    }

    KIO::StoredTransferJob* j = static_cast<KIO::StoredTransferJob*>(job);

//     kWarning() << QString::fromUtf8(j->data());

    bool ok;
    QVariantMap map = m_parser.parse(j->data(), &ok).toMap();
    if (!ok) {
        kWarning() << "parse error";
        return;
    }

    int ret = map["r"].toInt();
    if (ret != 0) {
        /// playlist error here
        kWarning() << "playlist error";
        return;
    }

    // save to an empty playlist
    QList<SongInfo>& playlist = m_playlist.isEmpty() ? m_playlist : m_playlistNext;
    playlist.clear();

    QVariantList songlist = map["song"].toList();
    foreach (const QVariant& song, songlist) {
        QVariantMap songmap = song.toMap();

        SongInfo si;

        QString subtype = songmap["subtype"].toString();
        if (!subtype.isEmpty()) {
            // skip ad
            continue;
        }

        si.picture = songmap["picture"].toString();
        si.albumtitle = songmap["albumtitle"].toString();
//         QString company = songmap["company"].toString();
//         double rating_avg = songmap["rating_avg"].toDouble();
//         QString public_time = songmap["public_time"].toString();
//         QString ssid = songmap["ssid"].toString();
//         QString album = songmap["album"].toString();
        si.like = songmap["like"].toInt();
        si.artist = songmap["artist"].toString();
        si.url = songmap["url"].toString();
        si.title = songmap["title"].toString();
        si.length = songmap["length"].toInt();
        si.sid = songmap["sid"].toInt();
        si.aid = songmap["aid"].toInt();

        si.fullplayed = false;

        kWarning() << si.title << si.artist << si.albumtitle << si.like;

        playlist << si;
    }

    // enqueue next song
    m_playIndexNext = 0;
    const SongInfo& si = playlist.first();
    m_media->clearQueue();
    m_media->enqueue(QList<QUrl>() << si.url);
    if (m_media->state() != Phonon::PlayingState
        && m_media->state() != Phonon::PausedState) {
        m_media->play();
    }
}

void TrayIcon::slot_end(KJob* job)
{
    if (job->error()) {
        kWarning() << "Job Error: " << job->errorString();
        return;
    }

//     KIO::StoredTransferJob* j = static_cast<KIO::StoredTransferJob*>(job);
//     kWarning() << QString::fromUtf8(j->data());
}

void TrayIcon::slotPauseAction()
{
    if (m_media->state() == Phonon::PlayingState) {
        m_media->pause();
    }
    else {
        m_media->play();
    }
}

void TrayIcon::slotStopAction()
{
    // reset media player and playlist
    m_media->stop();
    m_media->clear();
    m_playlist.clear();
    m_playlistNext.clear();
    m_playIndex = -1;
    m_playIndexNext = 0;
}

void TrayIcon::slotSkipAction()
{
    m_media->pause();
    if (!m_media->queue().isEmpty()) {
        // play next media
        m_media->setCurrentSource(m_media->queue().first());
        m_media->play();
    }
    else {
        // get playlist in progress, clear the current one
        m_media->stop();
        m_media->clear();
    }
}

void TrayIcon::slotLikeAction()
{
    const SongInfo& si = m_playlist.at(m_playIndex);
    if (likeAction->isChecked()) {
        like(si.sid);
    }
    else {
        unlike(si.sid);
    }
}

void TrayIcon::slotBanAction()
{
    const SongInfo& si = m_playlist.at(m_playIndex);
    ban(si.sid);
}

void TrayIcon::slotAccountAction()
{
    QDialog dlg;
    Ui::AccountDialog ui;
    ui.setupUi(&dlg);

    ui.emailEdit->setText(QString::fromAscii(m_accountEmail));
    ui.passwordEdit->setText(QString::fromAscii(m_accountPassword));

    if (dlg.exec() == QDialog::Accepted) {
        // save account info
        m_accountEmail = ui.emailEdit->text().toAscii();
        m_accountPassword = ui.passwordEdit->text().toAscii();

        KConfigGroup cg(KGlobal::config(), "Account");
        cg.writeEntry("Email", m_accountEmail);
        cg.writeEntry("Password", m_accountPassword);
        cg.sync();

        if (!m_accountEmail.isEmpty() && !m_accountPassword.isEmpty()) {
            // let us login now
            login();
        }
    }
}

void TrayIcon::slotAboutAction()
{
    KAboutApplicationDialog dlg(KGlobal::mainComponent().aboutData());
    dlg.exec();
}

void TrayIcon::slotChannelAction(QAction* action)
{
    slotStopAction();

    m_channelId = action->data().toInt();

    playlist();
}

void TrayIcon::slotMediaAboutToFinish()
{
    const SongInfo& si = m_playlist.at(m_playIndex);
    si.fullplayed = true;
}

void TrayIcon::slotMediaCurrentSourceChanged()
{
    kWarning() << m_playIndex << m_playIndexNext;

    // report full played song
    if (m_playIndex != -1) {
        const SongInfo& si = m_playlist.at(m_playIndex);
        if (si.fullplayed) {
            end(si.sid);
        }
    }

    m_playIndex = m_playIndexNext;
    ++m_playIndexNext;

    if (!m_playlistNext.isEmpty()) {
        // switch next playlist
        m_playlist.clear();
        m_playlist = m_playlistNext;
        m_playIndex = 0;
        m_playlistNext.clear();
    }

    // change current song info
    const SongInfo& si = m_playlist.at(m_playIndex);
//     setTitle(si.title);
    setToolTipIconByName("kdoubanfm");
    setToolTipTitle(si.title);
    setToolTipSubTitle(si.artist + " - " + si.albumtitle);

    // change like state
    likeAction->setChecked(si.like == 1);

    // download album picture
    if (m_albumPictureJob) {
        // kill previous one
        m_albumPictureJob->kill();
    }
    KIO::TransferJob* job = KIO::storedGet(si.picture, KIO::Reload, KIO::HideProgressInfo);
    job->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotAlbumPicture(KJob*)));
    m_albumPictureJob = job;
    job->start();

    if (m_playIndexNext < m_playlist.count()) {
        // enqueue next song in playlist
        const SongInfo& nsi = m_playlist.at(m_playIndexNext);
        m_media->enqueue(QList<QUrl>() << nsi.url);
    }
    else {
        // no more song in playlist, get new one with history
        m_playIndexNext = 0;
        playlist(m_playlist);
    }
}

void TrayIcon::slotMeidaStateChanged(Phonon::State newstate)
{
    switch (newstate) {
        case Phonon::StoppedState:
        case Phonon::PausedState:
            pauseAction->setIcon(KIcon("media-playback-start"));
            pauseAction->setText(i18n("&Play"));
            break;
        case Phonon::PlayingState:
            pauseAction->setIcon(KIcon("media-playback-pause"));
            pauseAction->setText(i18n("&Pause"));
            break;
        case Phonon::ErrorState:
        case Phonon::BufferingState:
        case Phonon::LoadingState:
            break;
    }
}

void TrayIcon::slotAlbumPicture(KJob* job)
{
    if (job->error()) {
        kWarning() << "Job Error: " << job->errorString();
        return;
    }

    KIO::StoredTransferJob* j = static_cast<KIO::StoredTransferJob*>(job);
    m_albumPictureJob = 0;

    QPixmap pixmap;
    pixmap.loadFromData(j->data());
    setToolTipIconByPixmap(pixmap);
}
