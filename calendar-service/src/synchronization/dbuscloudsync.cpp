#include "dbuscloudsync.h"
#include <QJsonObject>
#include <QJsonDocument>
#include "units.h"

Dbuscloudsync::Dbuscloudsync(QObject *parent)
    : DServiceBase(serviceBasePath + "/CloudSync", serviceBaseName + ".CloudSync", parent)
{

}

void Dbuscloudsync::MsgCallBack(const QByteArray &msg)
{
    //msg信息处理
    QJsonObject json;
    json = QJsonDocument::fromJson(msg).object();

    //TODO:解析获取到的数据，依据需要做后续操作

    qWarning() << "Get " << " error.";
}

