#ifdef Quotient_E2EE_ENABLED
#pragma once

#include <QtCore/QObject>

#include <functional>
#include <memory>

namespace QtOlm {
class Account;
}

namespace Quotient {
class Connection;

class EncryptionManager : public QObject {
    Q_OBJECT

public:
    // TODO: store constats separately?
    // TODO: 0.5 oneTimeKeyThreshold instead of 0.1?
    explicit EncryptionManager(
        const QByteArray& encryptionAccountPickle = QByteArray(),
        float signedKeysProportion = 1, float oneTimeKeyThreshold = float(0.1),
        QObject* parent = nullptr);
    ~EncryptionManager();

    void uploadIdentityKeys(Connection* connection);
    void uploadOneTimeKeys(Connection* connection, bool forceUpdate = false);
    void
    updateOneTimeKeyCounts(Connection* connection,
                           const QHash<QString, int>& deviceOneTimeKeysCount);
    void updateDeviceKeys(Connection* connection,
                          const QHash<QString, QStringList>& deviceKeys);
    QString sessionDecryptMessage(const QJsonObject& personalCipherObject,
                                  const QByteArray& senderKey);
    QByteArray olmAccountPickle();

    QtOlm::Account* account() const;

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace Quotient
#endif // Quotient_E2EE_ENABLED
