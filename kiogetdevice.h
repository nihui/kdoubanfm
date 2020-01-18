#ifndef KIOGETDEVICE
#define KIOGETDEVICE

#include <QIODevice>
#include <QFile>

class KJob;
namespace KIO { class Job; }

class KIOGetDevice : public QIODevice
{
    Q_OBJECT
public:
    explicit KIOGetDevice(QObject* parent = 0);
    virtual ~KIOGetDevice();
    virtual bool atEnd() const;
    virtual qint64 bytesAvailable() const;
    virtual bool canReadLine() const;
    virtual void close();
    virtual bool isSequential() const;
    virtual bool open(OpenMode mode);
    virtual qint64 pos() const;
    virtual bool reset();
    virtual bool seek(qint64 pos);
    virtual qint64 size() const;
protected:
    virtual qint64 readData(char* data, qint64 maxSize);
    virtual qint64 writeData(const char* data, qint64 maxSize);
private Q_SLOTS:
    void slot_data(KIO::Job*, const QByteArray& data);
    void slot_result(KJob*);
private:
    KJob* job;
    QByteArray m_tmp;
    int m_pos;
    bool finished;
};

#endif // KIOGETDEVICE
