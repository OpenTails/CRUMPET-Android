/*
 *   Copyright 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *   Copyright 2019 Ildar Gilmanov <gil.ildar@gmail.com>
 *   This file based on sample code from Kirigami
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 3, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this program; if not, see <https://www.gnu.org/licenses/>
 */

#ifdef Q_OS_ANDROID
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#include <QQmlApplicationEngine>
#include <QtQml>
#include <QUrl>
#include <QColor>
#include <QTimer>
#include <QIcon>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h> // for QAndroidService
// WindowManager.LayoutParams
#define FLAG_TRANSLUCENT_STATUS 0x04000000
#define FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS 0x80000000
// View
#define SYSTEM_UI_FLAG_LIGHT_STATUS_BAR 0x00002000

#endif

#include "BTConnectionManager.h"
#include "DeviceModel.h"
#include "CommandModel.h"
#include "FilterProxyModel.h"
#include "AlarmList.h"
#include "AppSettings.h"
#include "CommandQueue.h"
#include "GestureController.h"
#include "GestureDetectorModel.h"
#include "IdleMode.h"
#include "Utilities.h"
#include "PermissionsManager.h"

#include <QAbstractItemModelReplica>
#include "rep_AppSettingsProxy_replica.h"
#include "rep_BTConnectionManagerProxy_replica.h"
#include "rep_CommandQueueProxy_replica.h"
#include <rep_GestureControllerProxy_replica.h>

#include <KLocalizedContext>
#include <KLocalizedString>

int appMain(int argc, char *argv[])
{
//The desktop QQC2 style needs it to be a QApplication
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif
    //qputenv("QML_IMPORT_TRACE", "1");
    PermissionsManager* permissionsManager = new PermissionsManager(&app);
    permissionsManager->requestPermissionNow(QLatin1String{"WRITE_EXTERNAL_STORAGE"});
    if(!permissionsManager->hasPermission(QLatin1String{"WRITE_EXTERNAL_STORAGE"})) {
        qCritical() << Q_FUNC_INFO << "We were not granted the external storage access.";
        app.quit();
    }

#ifdef Q_OS_ANDROID
    qDebug() << Q_FUNC_INFO << "Starting service, if it isn't already...";
    QJniObject::callStaticMethod<void>("org/thetailcompany/digitail/TailService",
                                                "startTailService",
                                                "(Landroid/content/Context;)V",
                                                QNativeInterface::QAndroidApplication::context());
    qDebug() << Q_FUNC_INFO << "Service started, or already launched";
#else
    app.setApplicationVersion(QLatin1String{"Desktop"});
#endif

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("digitail"));
#ifdef Q_OS_ANDROID
    KLocalizedString::addDomainLocaleDir("digitail", QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/org.kde.ki18n/"));
    KLocalizedString::addDomainLocaleDir("digitail", QLatin1String{"assets:/share/locale/"});
    qDebug() << Q_FUNC_INFO << "Available application translations:" << KLocalizedString::availableApplicationTranslations();
#else
    KLocalizedString::addDomainLocaleDir("digitail", QString::fromUtf8("%1/../locale").arg(app.applicationDirPath()));
#endif

    QIcon::setThemeSearchPaths({QStringLiteral(":/icons")});
    QIcon::setThemeName(QStringLiteral("breeze-internal"));

    qInfo() << Q_FUNC_INFO << "Creating engine";
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&app));
    qInfo() << Q_FUNC_INFO << "Registering Kirigami types";
    qmlRegisterType<FilterProxyModel>("org.thetailcompany.digitail", 1, 0, "FilterProxyModel");

    QFile file(QLatin1String(":/gplv3-license-text"));
    if(file.open(QIODevice::ReadOnly)) {
        const QString data = QString::fromUtf8(file.readAll());
        file.close();
        engine.rootContext()->setContextProperty(QLatin1String("GPLv3LicenseText"), data);
    }
    engine.rootContext()->setContextProperty(QLatin1String("AppVersion"), app.applicationVersion());

    qInfo() << Q_FUNC_INFO << "Connecting to service...";
    QRemoteObjectNode* repNode = new QRemoteObjectNode(&app);
#ifdef Q_OS_ANDROID
    repNode->connectToNode(QUrl(QStringLiteral("localabstract:digitail")));
#else
    repNode->connectToNode(QUrl(QStringLiteral("local:digitail")));
#endif

    qInfo() << Q_FUNC_INFO << "Connected, attempting to load replicas...";
    QSharedPointer<AppSettingsProxyReplica> settingsReplica(repNode->acquire<AppSettingsProxyReplica>());
    bool res = settingsReplica->waitForSource(500);
    if(!res) {
        qInfo() << Q_FUNC_INFO << "No service exists yet, so let's start it...";
#ifdef Q_OS_ANDROID
        repNode->connectToNode(QUrl(QStringLiteral("localabstract:digitail")));
#else
        QProcess service;
        service.startDetached(app.applicationFilePath(), QStringList() << QStringLiteral("-service"));
        QCoreApplication::processEvents();
        repNode->connectToNode(QUrl(QStringLiteral("local:digitail")));
#endif
        settingsReplica.reset(repNode->acquire<AppSettingsProxyReplica>());
        res = settingsReplica->waitForSource(500);
        if (!res) {
            qCritical() << Q_FUNC_INFO << "Kapow! Replica for Settings failed to surface";
        }
    }
    qmlRegisterSingletonType<AppSettings>("org.thetailcompany.digitail", 1, 0, "AppSettings", [&settingsReplica](QQmlEngine */*engine*/, QJSEngine */*scriptEngine*/) -> QObject * {
        QQmlEngine::setObjectOwnership(settingsReplica.data(), QQmlEngine::CppOwnership);
        return settingsReplica.data();
    });
    auto retranslate = [&settingsReplica](){
        if (settingsReplica->languageOverride().isEmpty()) {
            qDebug() << Q_FUNC_INFO << "Clearing ki18n languages";
            KLocalizedString::clearLanguages();
        } else {
            const QStringList languages{settingsReplica->languageOverride(), QLatin1String{"en_US"}};
            qDebug() << Q_FUNC_INFO << "Setting ki18n language list to" << languages;
            KLocalizedString::setLanguages(languages);
        }
    };
    QObject::connect(settingsReplica.data(), &AppSettingsProxyReplica::languageOverrideChanged, &app, retranslate);
    retranslate();

    QSharedPointer<BTConnectionManagerProxyReplica> btConnectionManagerReplica(repNode->acquire<BTConnectionManagerProxyReplica>());
    res = btConnectionManagerReplica->waitForSource();
    if(!res) { qCritical() << Q_FUNC_INFO << "Kapow! Replica for btConnectionManagerReplica failed to surface"; }
    qmlRegisterSingletonType<BTConnectionManager>("org.thetailcompany.digitail", 1, 0, "BTConnectionManager", [&btConnectionManagerReplica](QQmlEngine */*engine*/, QJSEngine */*scriptEngine*/) -> QObject * {
        QQmlEngine::setObjectOwnership(btConnectionManagerReplica.data(), QQmlEngine::CppOwnership);
        return btConnectionManagerReplica.data();
    });

    QSharedPointer<CommandQueueProxyReplica> commandQueueReplica(repNode->acquire<CommandQueueProxyReplica>());
    res = commandQueueReplica->waitForSource();
    if(!res) { qCritical() << Q_FUNC_INFO << "Kapow! Replica for commandQueueReplica failed to surface"; }
    qmlRegisterSingletonType<CommandQueue>("org.thetailcompany.digitail", 1, 0, "CommandQueue", [&commandQueueReplica](QQmlEngine */*engine*/, QJSEngine */*scriptEngine*/) -> QObject * {
        QQmlEngine::setObjectOwnership(commandQueueReplica.data(), QQmlEngine::CppOwnership);
        return commandQueueReplica.data();
    });

    QSharedPointer<GestureControllerProxyReplica> gestureControllerReplica(repNode->acquire<GestureControllerProxyReplica>());
    res = gestureControllerReplica->waitForSource();
    if(!res) { qCritical() << Q_FUNC_INFO << "Kapow! Replica for gestureControllerReplica failed to surface"; }
    qmlRegisterSingletonType<GestureController>("org.thetailcompany.digitail", 1, 0, "GestureController", [&gestureControllerReplica](QQmlEngine */*engine*/, QJSEngine */*scriptEngine*/) -> QObject * {
        QQmlEngine::setObjectOwnership(gestureControllerReplica.data(), QQmlEngine::CppOwnership);
        return gestureControllerReplica.data();
    });

    QSharedPointer<QAbstractItemModelReplica> deviceModelReplica(repNode->acquireModel(QLatin1String{"DeviceModel"}));
    qmlRegisterSingletonType<DeviceModel>("org.thetailcompany.digitail", 1, 0, "DeviceModel", [&deviceModelReplica](QQmlEngine */*engine*/, QJSEngine */*scriptEngine*/) -> QObject * {
        QQmlEngine::setObjectOwnership(deviceModelReplica.data(), QQmlEngine::CppOwnership);
        return deviceModelReplica.data();
    });
    qmlRegisterUncreatableType<DeviceModel>("org.thetailcompany.digitail", 1, 0, "DeviceModelTypes", QLatin1String{"Not createable, use the replicated object named DeviceModel"});

    QSharedPointer<QAbstractItemModelReplica> commandModelReplica(repNode->acquireModel(QLatin1String{"CommandModel"}));
    qmlRegisterSingletonType<CommandModel>("org.thetailcompany.digitail", 1, 0, "CommandModel", [&commandModelReplica](QQmlEngine */*engine*/, QJSEngine */*scriptEngine*/) -> QObject * {
        QQmlEngine::setObjectOwnership(commandModelReplica.data(), QQmlEngine::CppOwnership);
        return commandModelReplica.data();
    });
    qmlRegisterUncreatableType<CommandModel>("org.thetailcompany.digitail", 1, 0, "CommandModelTypes", QLatin1String{"Not createable, use the replicated object named CommandModel"});

    QSharedPointer<QAbstractItemModelReplica> gestureDetectorModel(repNode->acquireModel(QLatin1String{"GestureDetectorModel"}));
    qmlRegisterSingletonType<GestureDetectorModel>("org.thetailcompany.digitail", 1, 0, "GestureDetectorModel", [&gestureDetectorModel](QQmlEngine */*engine*/, QJSEngine */*scriptEngine*/) -> QObject * {
        QQmlEngine::setObjectOwnership(gestureDetectorModel.data(), QQmlEngine::CppOwnership);
        return gestureDetectorModel.data();
    });
    qmlRegisterUncreatableType<GestureDetectorModel>("org.thetailcompany.digitail", 1, 0, "GestureDetectorModelTypes", QLatin1String{"Not createable, use the replicated object named GestureDetectorModel"});

    Utilities::getInstance()->setConnectionManager(btConnectionManagerReplica.data());
    Utilities::getInstance()->setParent(&app);
    qmlRegisterSingletonType<Utilities>("org.thetailcompany.digitail", 1, 0, "Utilities", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        QQmlEngine::setObjectOwnership(Utilities::getInstance(), QQmlEngine::CppOwnership);
        return Utilities::getInstance();
    });

    QObject::connect(permissionsManager, &PermissionsManager::permissionsChanged, btConnectionManagerReplica.data(), [=](){
        if(permissionsManager->hasBluetoothPermissions() && permissionsManager->hasNotificationPermissions()) {
            // Don't launch the discovery immediately, let's give things a change to start up...
            QTimer::singleShot(100, btConnectionManagerReplica.data(), &BTConnectionManagerProxyReplica::startDiscovery);
        }
    });
    qmlRegisterSingletonType<PermissionsManager>("org.thetailcompany.digitail", 1, 0, "PermissionsManager", [=](QQmlEngine */*engine*/, QJSEngine */*scriptEngine*/) -> QObject * {
        QQmlEngine::setObjectOwnership(permissionsManager, QQmlEngine::CppOwnership);
        return permissionsManager;
    });

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Failed to load the main qml file, exiting";
        return -1;
#ifdef Q_OS_ANDROID
    } else {
        QNativeInterface::QAndroidApplication::hideSplashScreen();
#endif
    }

    bool settingsReplicaDestroyed{false};
    QObject::connect(settingsReplica.data(), &QObject::destroyed, [&settingsReplicaDestroyed](){ settingsReplicaDestroyed = true; });
    QObject::connect(&app, &QCoreApplication::aboutToQuit, btConnectionManagerReplica.data(), [&btConnectionManagerReplica,&settingsReplica,&settingsReplicaDestroyed,repNode](){
        if(!btConnectionManagerReplica->isConnected()) {
            // Not connected, so kill the service
#ifdef Q_OS_ANDROID
            Q_UNUSED(settingsReplica);
            Q_UNUSED(settingsReplicaDestroyed);
            QJniObject::callStaticMethod<void>("org/thetailcompany/digitail/TailService",
                                                "stopTailService",
                                                "(Landroid/content/Context;)V",
                                                QNativeInterface::QAndroidApplication::context());
#else
            if (!settingsReplicaDestroyed) {
                settingsReplica->shutDownService();
            }
            QCoreApplication::processEvents(); // Actually let the replicant respond to our request...
#endif
        }
        repNode->deleteLater();
    });
#ifdef Q_OS_ANDROID
    //HACK to color the system bar on Android, use qtandroidextras and call the appropriate Java methods
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
        QJniObject activity = QNativeInterface::QAndroidApplication::context();
        QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");
        window.callMethod<void>("addFlags", "(I)V", FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
        window.callMethod<void>("clearFlags", "(I)V", FLAG_TRANSLUCENT_STATUS);
        window.callMethod<void>("setStatusBarColor", "(I)V", QColor("#2196f3").rgba());
        window.callMethod<void>("setNavigationBarColor", "(I)V", QColor("#2196f3").rgba());
    });
#endif

    QTimer keepAliver;
    keepAliver.setInterval(10000);
    QObject::connect(&keepAliver, &QTimer::timeout, btConnectionManagerReplica.data(), [&btConnectionManagerReplica](){
        // This is very extremely silly and i'd rather we didn't have to do this,
        // but... since otherwise Android will kill the service, perform some
        // kind of activity against that activity to stop it from being shut down
        btConnectionManagerReplica->isConnected();
    });
    keepAliver.start();

    return app.exec();
}

int serviceMain(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    QAndroidService app(argc, argv);
#else
    QCoreApplication app(argc, argv);
#endif
    app.setOrganizationName(QLatin1String{"The Tail Company"});
    app.setOrganizationDomain(QLatin1String{"thetailcompany.com"});
    app.setApplicationName(QLatin1String{"DIGITAiL"});
    qInfo() << Q_FUNC_INFO << "Service starting...";

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("digitail"));
#ifdef Q_OS_ANDROID
    KLocalizedString::addDomainLocaleDir("digitail", QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/org.kde.ki18n/"));
    KLocalizedString::addDomainLocaleDir("digitail", QLatin1String{"assets:/share/locale/"});
    qDebug() << Q_FUNC_INFO << "Available application translations:" << KLocalizedString::availableApplicationTranslations();
#else
    KLocalizedString::addDomainLocaleDir("digitail", QString::fromUtf8("%1/../src/locale").arg(app.applicationDirPath()));
#endif

#ifdef Q_OS_ANDROID
    QRemoteObjectHost srcNode(QUrl(QStringLiteral("localabstract:digitail")));
#else
    QRemoteObjectHost srcNode(QUrl(QStringLiteral("local:digitail")));
#endif

    qDebug() << Q_FUNC_INFO << "Creating application settings";
    AppSettings* appSettings = new AppSettings(&app);

    qDebug() << Q_FUNC_INFO << "Creating connection manager";
    BTConnectionManager* btConnectionManager = new BTConnectionManager(appSettings, &app);

    qDebug() << Q_FUNC_INFO << "Setting command queue on alarm list";
    appSettings->alarmListImpl()->setCommandQueue(qobject_cast<CommandQueue*>(btConnectionManager->commandQueue()));

    PermissionsManager* permissionsManager = new PermissionsManager(&app);
    auto acquireWakeLock = [permissionsManager](){
        if(permissionsManager->hasNotificationPermissions()) {
#ifdef Q_OS_ANDROID
            QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
                QJniObject androidService = QNativeInterface::QAndroidApplication::context();
                if(androidService.isValid()) {
                    androidService.callMethod<void>("acquireWakeLock");
                }
            });
#endif
        }
    };
    QObject::connect(permissionsManager, &PermissionsManager::permissionsChanged, btConnectionManager, acquireWakeLock);
    QObject::connect(btConnectionManager, &BTConnectionManager::isConnectedChanged, [&acquireWakeLock](bool isConnected) {
        if(isConnected) {
            acquireWakeLock();
        } else {
#ifdef Q_OS_ANDROID
            QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
                QJniObject androidService = QNativeInterface::QAndroidApplication::context();
                if(androidService.isValid()) {
                    androidService.callMethod<void>("releaseWakeLock");
                }
            });
#endif
        }
    });

    qDebug() << Q_FUNC_INFO << "Creating casual mode handler";
    IdleMode* idleMode = new IdleMode(&app);
    idleMode->setAppSettings(appSettings);
    idleMode->setConnectionManager(btConnectionManager);

    QTimer::singleShot(1, &app, [&srcNode, appSettings, btConnectionManager]() {
        qDebug() << Q_FUNC_INFO << "Replicating application settings";
        srcNode.enableRemoting(appSettings);

        qDebug() << Q_FUNC_INFO << "Replicating connection manager";
        srcNode.enableRemoting(btConnectionManager);

        qDebug() << Q_FUNC_INFO << "Getting device model";
        DeviceModel * deviceModel = qobject_cast<DeviceModel *>(btConnectionManager->deviceModel());
        qDebug() << Q_FUNC_INFO << "Replicating device model";
        QVector<int> roles;
        static const QMetaEnum deviceModelRolesEnum = DeviceModel::staticMetaObject.enumerator(DeviceModel::staticMetaObject.indexOfEnumerator("Roles"));
        for (int enumKey = 0; enumKey < deviceModelRolesEnum.keyCount(); ++enumKey) {
            roles << deviceModelRolesEnum.value(enumKey);
        }
        srcNode.enableRemoting(deviceModel, QLatin1String{"DeviceModel"}, roles);

        qDebug() << Q_FUNC_INFO << "Getting command model";
        CommandModel * tailCommandModel = qobject_cast<CommandModel *>(btConnectionManager->commandModel());
        qDebug() << Q_FUNC_INFO << "Replicating command model";
        roles.clear();
        static const QMetaEnum tailCommandModelRolesEnum = CommandModel::staticMetaObject.enumerator(CommandModel::staticMetaObject.indexOfEnumerator("Roles"));
        for (int enumKey = 0; enumKey < tailCommandModelRolesEnum.keyCount(); ++enumKey) {
            roles << tailCommandModelRolesEnum.value(enumKey);
        }
        srcNode.enableRemoting(tailCommandModel, QLatin1String{"CommandModel"}, roles);

        qDebug() << Q_FUNC_INFO << "Getting command queue";
        CommandQueue* commandQueue = qobject_cast<CommandQueue*>(btConnectionManager->commandQueue());
        qDebug() << Q_FUNC_INFO << "Replicating command queue";
        srcNode.enableRemoting(commandQueue);

        qDebug() << Q_FUNC_INFO << "Creating gesture controller";
        GestureController* gestureController = new GestureController(btConnectionManager);
        gestureController->setConnectionManager(btConnectionManager);
        qDebug() << Q_FUNC_INFO << "Replicating gesture controller";
        srcNode.enableRemoting(gestureController);

        qDebug() << Q_FUNC_INFO << "Getting and replicating gesture detector model";
        gestureController->model()->setAppSettings(appSettings);
        roles.clear();
        static const QMetaEnum gestureDetectorModelRolesEnum = GestureDetectorModel::staticMetaObject.enumerator(GestureDetectorModel::staticMetaObject.indexOfEnumerator("Roles"));
        for (int enumKey = 0; enumKey < gestureDetectorModelRolesEnum.keyCount(); ++enumKey) {
            roles << gestureDetectorModelRolesEnum.value(enumKey);
        }
        srcNode.enableRemoting(gestureController->model(), QLatin1String{"GestureDetectorModel"}, roles);
    });

    return app.exec();
}

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    if (argc <= 1) {
        return appMain(argc, argv);
    } else if(argc > 1 && strcmp(argv[1], "-service") == 0) {
        return serviceMain(argc, argv);
    } else {
        // unrecognised argument
        return -1;
    }
}
