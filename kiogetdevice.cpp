#include "kiogetdevice.h"

#include <QUrl>
#include <KIO/TransferJob>
#include <KIO/Job>

#include <KDebug>

KIOGetDevice::KIOGetDevice(QObject* parent)
{
    m_pos = 0;
    finished = false;

    KJob* job = KIO::get(QUrl("http://download02.ztgame.com.cn/elsmusic/27.mp3"));
    connect(job, SIGNAL(data(KIO::Job*, QByteArray)),
            this, SLOT(slot_data(KIO::Job*, QByteArray)));
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slot_result(KJob*)));
    job->start();
}

KIOGetDevice::~KIOGetDevice()
{
}

bool KIOGetDevice::atEnd() const
{
    kWarning();
    return true;
    if (!finished)
        return false;
    return m_pos == m_tmp.size();
}

qint64 KIOGetDevice::bytesAvailable() const
{
    kWarning();
    return m_tmp.size() - m_pos;
}

bool KIOGetDevice::canReadLine() const
{
    kWarning();
    return false;
}

void KIOGetDevice::close()
{
    kWarning();
}

bool KIOGetDevice::isSequential() const
{
    kWarning();
    return true;
}

bool KIOGetDevice::open(OpenMode mode)
{
    kWarning();
    return true;
}

qint64 KIOGetDevice::pos() const
{
    kWarning();
    return m_pos;
}

bool KIOGetDevice::reset()
{
    kWarning();
    m_pos = 0;
    return true;
}

bool KIOGetDevice::seek(qint64 pos)
{
    kWarning();
    return false;
}

qint64 KIOGetDevice::size() const
{
    kWarning();
    return m_tmp.size();
}

qint64 KIOGetDevice::readData(char* data, qint64 maxSize)
{
    kWarning() << maxSize;
    qint64 nread = maxSize > bytesAvailable() ? bytesAvailable() : maxSize;
    memcpy(data, m_tmp.constData(), nread);
    return nread;
}

qint64 KIOGetDevice::writeData(const char* data, qint64 maxSize)
{
    kWarning();
    return 0;
}

void KIOGetDevice::slot_data(KIO::Job*, const QByteArray& data)
{
    kWarning();
    m_tmp.append(data);
    emit readyRead();
}

void KIOGetDevice::slot_result(KJob*)
{
    kWarning();
    finished = true;
}
