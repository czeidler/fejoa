#include "remotestorage.h"

#include "remoteauthentication.h"


RemoteDataStorage::RemoteDataStorage(Profile *profile) :
    profile(profile),
    authentication(NULL)
{
}

RemoteDataStorage::~RemoteDataStorage()
{
    delete authentication;
}

QString RemoteDataStorage::getUid()
{
    return uid;
}

const QString &RemoteDataStorage::getConnectionType()
{
    return connectionType;
}

const QString &RemoteDataStorage::getUrl()
{
    return url;
}

const QString &RemoteDataStorage::getAuthType()
{
    return authType;
}

void RemoteDataStorage::setPHPEncryptedRemoteConnection(const QString &url)
{
    connectionType = "PHPEncryptedRemoteStorage";
    this->url = url;
    connectionInfo = ConnectionManager::getEncryptedPHPConnectionFor(QUrl(url));
    uid = hash();
}

void RemoteDataStorage::setHTTPRemoteConnection(const QString &url)
{
    connectionType = "HTTPRemoteStorage";
    this->url = url;
    connectionInfo = ConnectionManager::getHTTPConnectionFor(QUrl(url));
    uid = hash();
}

void RemoteDataStorage::setSignatureAuth(const QString &userName, const QString &keyStoreId,
                                         const QString &keyId, const QString &serverUser)
{
    this->authType = "SignatureAuth";
    this->authenticationInfo = RemoteAuthenticationInfo(userName, keyStoreId, keyId, serverUser);
}

RemoteConnectionInfo RemoteDataStorage::getRemoteConnectionInfo()
{
    return connectionInfo;
}

RemoteAuthenticationInfo RemoteDataStorage::getRemoteAuthenticationInfo()
{
    return authenticationInfo;
}

QString RemoteDataStorage::hash()
{
    CryptoInterface *crypto = CryptoInterfaceSingleton::getCryptoInterface();
    QByteArray value = QString(connectionType + url).toLatin1();
    QByteArray hashResult = crypto->sha1Hash(value);
    return crypto->toHex(hashResult);
}

WP::err RemoteDataStorage::write(StorageDirectory &dir)
{
    WP::err error = dir.writeSafe("connection_type", getConnectionType());
    if (error != WP::kOk)
        return error;
    error = dir.writeSafe("connection_url", getUrl());
    if (error != WP::kOk)
        return error;
    error = dir.writeSafe("auth_username", getAuthUserName());
    if (error != WP::kOk)
        return error;
    error = dir.writeSafe("auth_type", getAuthType());
    if (error != WP::kOk)
        return error;
    error = dir.writeSafe("auth_keystore", getAuthKeyStoreId());
    if (error != WP::kOk)
        return error;
    error = dir.writeSafe("auth_keyid", getAuthKeyId());
    if (error != WP::kOk)
        return error;
    error = dir.writeSafe("server_user", getServerUser());
    if (error != WP::kOk)
        return error;

    return WP::kOk;
}


WP::err RemoteDataStorage::load(StorageDirectory &dir)
{
    QString connectionType;
    QString url;
    WP::err error = WP::kError;
    error = dir.readSafe("connection_type", connectionType);
    if (error != WP::kOk)
        return error;

    error = dir.readSafe("connection_url", url);
    if (error == WP::kOk) {
        if (connectionType == "PHPEncryptedRemoteStorage")
            setPHPEncryptedRemoteConnection(url);
        else if (connectionType == "HTTPRemoteStorage")
            setHTTPRemoteConnection(url);
    }

    QString authType;
    QString authUserName;
    QString authKeyStoreId;
    QString authKey;
    QString serverUser;
    // auth
    error = dir.readSafe("auth_type", authType);
    if (error != WP::kOk)
        return error;
    error = dir.readSafe("auth_username", authUserName);
    if (error != WP::kOk)
        return error;
    error = dir.readSafe("auth_keystore", authKeyStoreId);
    if (error != WP::kOk)
        return error;
    error = dir.readSafe("auth_keyid", authKey);
    if (error != WP::kOk)
        return error;
    error = dir.readSafe("server_user", serverUser);
    if (error != WP::kOk)
        return error;

    if (authType == "SignatureAuth")
        setSignatureAuth(authUserName, authKeyStoreId, authKey, serverUser);

    return WP::kOk;
}
