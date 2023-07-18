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
#include <QtAndroid>
#include <QAndroidService>

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

#include <klocalizedcontext.h>
#include <klocalizedstring.h>

int appMain(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
//The desktop QQC2 style needs it to be a QApplication
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif
    //qputenv("QML_IMPORT_TRACE", "1");
    PermissionsManager* permissionsManager = new PermissionsManager(&app);
    permissionsManager->requestPermissionNow("WRITE_EXTERNAL_STORAGE");
    if(!permissionsManager->hasPermission("WRITE_EXTERNAL_STORAGE")) {
        qCritical() << Q_FUNC_INFO << "We were not granted the external storage access.";
        app.quit();
    }

#ifdef Q_OS_ANDROID
    qDebug() << Q_FUNC_INFO << "Starting service, if it isn't already...";
    QAndroidJniObject::callStaticMethod<void>("org/thetailcompany/digitail/TailService",
                                                "startTailService",
                                                "(Landroid/content/Context;)V",
                                                QtAndroid::androidActivity().object());
    qDebug() << Q_FUNC_INFO << "Service started, or already launched";
#else
    app.setApplicationVersion("Desktop");
#endif

    KLocalizedString::setApplicationDomain("digitail");
#if defined(__ANDROID__)
    KLocalizedString::addDomainLocaleDir("digitail", QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/org.kde.ki18n/"));
    QStringList languages{"cs_CZ", "da_DK", "es_ES", "fr", "ja_JP", "nl_NL", "ru"};
    for (const QString& language : languages) {
        qDebug() << Q_FUNC_INFO << "App is translated into" << language << KLocalizedString::isApplicationTranslatedInto(language);
    }
#else
    KLocalizedString::addDomainLocaleDir("digitail", QString("%1/../locale").arg(app.applicationDirPath()));
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
        QString data(file.readAll());
        file.close();
        engine.rootContext()->setContextProperty(QLatin1String("GPLv3LicenseText"), data);
    }
    engine.rootContext()->setContextProperty(QLatin1String("AppVersion"), app.applicationVersion());

    qInfo() << Q_FUNC_INFO << "Connecting to service...";
    QRemoteObjectNode* repNode = new QRemoteObjectNode(&app);
    repNode->connectToNode(QUrl(QStringLiteral("local:digitail")));

    qInfo() << Q_FUNC_INFO << "Connected, attempting to load replicas...";
    QSharedPointer<AppSettingsProxyReplica> settingsReplica(repNode->acquire<AppSettingsProxyReplica>());
    bool res = settingsReplica->waitForSource(500);
    if(!res) {
        qInfo() << Q_FUNC_INFO << "No service exists yet, so let's start it...";
        QProcess service;
        service.startDetached(app.applicationFilePath(), QStringList() << QStringLiteral("-service"));
        QCoreApplication::processEvents();
        repNode->connectToNode(QUrl(QStringLiteral("local:digitail")));
        settingsReplica.reset(repNode->acquire<AppSettingsProxyReplica>());
        res = settingsReplica->waitForSource(500);
        if (!res) {
            qCritical() << Q_FUNC_INFO << "Kapow! Replica for Settings failed to surface";
        }
    }
    engine.rootContext()->setContextProperty(QLatin1String("AppSettings"), settingsReplica.data());
    auto retranslate = [&settingsReplica](){
        if (settingsReplica->languageOverride().isEmpty()) {
            KLocalizedString::clearLanguages();
        } else {
            KLocalizedString::setLanguages(QStringList() << settingsReplica->languageOverride() << "en_US");
        }
    };
    QObject::connect(settingsReplica.data(), &AppSettingsProxyReplica::languageOverrideChanged, &app, retranslate);
    retranslate();

    QSharedPointer<BTConnectionManagerProxyReplica> btConnectionManagerReplica(repNode->acquire<BTConnectionManagerProxyReplica>());
    res = btConnectionManagerReplica->waitForSource();
    if(!res) { qCritical() << Q_FUNC_INFO << "Kapow! Replica for btConnectionManagerReplica failed to surface"; }
    engine.rootContext()->setContextProperty(QLatin1String("BTConnectionManager"), btConnectionManagerReplica.data());

    QScopedPointer<CommandQueueProxyReplica> commandQueueReplica(repNode->acquire<CommandQueueProxyReplica>());
    res = commandQueueReplica->waitForSource();
    if(!res) { qCritical() << Q_FUNC_INFO << "Kapow! Replica for commandQueueReplica failed to surface"; }
    engine.rootContext()->setContextProperty(QLatin1String("CommandQueue"), commandQueueReplica.data());

    QScopedPointer<GestureControllerProxyReplica> gestureControllerReplica(repNode->acquire<GestureControllerProxyReplica>());
    res = gestureControllerReplica->waitForSource();
    if(!res) { qCritical() << Q_FUNC_INFO << "Kapow! Replica for gestureControllerReplica failed to surface"; }
    engine.rootContext()->setContextProperty(QLatin1String("GestureController"), gestureControllerReplica.data());

    QScopedPointer<QAbstractItemModelReplica> deviceModelReplica(repNode->acquireModel("DeviceModel"));
    engine.rootContext()->setContextProperty(QLatin1String("DeviceModel"), deviceModelReplica.data());

    QScopedPointer<QAbstractItemModelReplica> commandModelReplica(repNode->acquireModel("CommandModel"));
    engine.rootContext()->setContextProperty(QLatin1String("CommandModel"), commandModelReplica.data());

    QScopedPointer<QAbstractItemModelReplica> gestureDetectorModel(repNode->acquireModel("GestureDetectorModel"));
    engine.rootContext()->setContextProperty(QLatin1String("GestureDetectorModel"), gestureDetectorModel.data());

    Utilities::getInstance()->setConnectionManager(btConnectionManagerReplica.data());
    Utilities::getInstance()->setParent(&app);
    qmlRegisterSingletonType<Utilities>("org.thetailcompany.digitail", 1, 0, "Utilities", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        return Utilities::getInstance();
    });

    QObject::connect(permissionsManager, &PermissionsManager::permissionsChanged, permissionsManager, [=](){
        if(permissionsManager->hasBluetoothPermissions()) {
            // Don't launch the discovery immediately, let's give things a change to start up...
            QTimer::singleShot(100, btConnectionManagerReplica.data(), &BTConnectionManagerProxyReplica::startDiscovery);
        }
    });
    engine.rootContext()->setContextProperty("PermissionsManager", permissionsManager);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Failed to load the main qml file, exiting";
        return -1;
    }

    bool settingsReplicaDestroyed{false};
    QObject::connect(settingsReplica.data(), &QObject::destroyed, [&settingsReplicaDestroyed](){ settingsReplicaDestroyed = true; });
    QObject::connect(&app, &QCoreApplication::aboutToQuit, btConnectionManagerReplica.data(), [&btConnectionManagerReplica,&settingsReplica,&settingsReplicaDestroyed,repNode](){
        if(!btConnectionManagerReplica->isConnected()) {
            // Not connected, so kill the service
#ifdef Q_OS_ANDROID
            Q_UNUSED(settingsReplica);
            Q_UNUSED(settingsReplicaDestroyed);
            QAndroidJniObject::callStaticMethod<void>("org/thetailcompany/digitail/TailService",
                                                "stopTailService",
                                                "(Landroid/content/Context;)V",
                                                QtAndroid::androidActivity().object());
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
    QtAndroid::runOnAndroidThread([=]() {
        QAndroidJniObject window = QtAndroid::androidActivity().callObjectMethod("getWindow", "()Landroid/view/Window;");
        window.callMethod<void>("addFlags", "(I)V", FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
        window.callMethod<void>("clearFlags", "(I)V", FLAG_TRANSLUCENT_STATUS);
        window.callMethod<void>("setStatusBarColor", "(I)V", QColor("#2196f3").rgba());
        window.callMethod<void>("setNavigationBarColor", "(I)V", QColor("#2196f3").rgba());
    });
#endif

    return app.exec();
}

int serviceMain(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    QAndroidService app(argc, argv);

#else
    QCoreApplication app(argc, argv);
#endif
    app.setOrganizationName("The Tail Company");
    app.setOrganizationDomain("thetailcompany.com");
    app.setApplicationName("DIGITAiL");
    qInfo() << Q_FUNC_INFO << "Service starting...";

    KLocalizedString::setApplicationDomain("digitail");
#if defined(__ANDROID__)
    KLocalizedString::addDomainLocaleDir("digitail", QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/org.kde.ki18n/"));
    QStringList languages{"cs_CZ", "da_DK", "es_ES", "fr", "ja_JP", "nl_NL", "ru"};
    for (const QString& language : languages) {
        qDebug() << Q_FUNC_INFO << "App is translated into" << language << KLocalizedString::isApplicationTranslatedInto(language);
    }
#else
    KLocalizedString::addDomainLocaleDir("digitail", QString("%1/../src/locale").arg(app.applicationDirPath()));
#endif

    QRemoteObjectHost srcNode(QUrl(QStringLiteral("local:digitail")));

    qDebug() << Q_FUNC_INFO << "Creating application settings";
    AppSettings* appSettings = new AppSettings(&app);

    qDebug() << Q_FUNC_INFO << "Creating connection manager";
    BTConnectionManager* btConnectionManager = new BTConnectionManager(appSettings, &app);

    qDebug() << Q_FUNC_INFO << "Setting command queue on alarm list";
    appSettings->alarmListImpl()->setCommandQueue(qobject_cast<CommandQueue*>(btConnectionManager->commandQueue()));

    QObject::connect(btConnectionManager, &BTConnectionManager::isConnectedChanged, [](bool isConnected) {
#ifdef Q_OS_ANDROID
        QAndroidJniObject androidService = QtAndroid::androidService();
        if(androidService.isValid()) {
            if(isConnected) {
                QtAndroid::runOnAndroidThread([=]() { androidService.callMethod<void>("acquireWakeLock"); });
            } else {
                QtAndroid::runOnAndroidThread([=]() { androidService.callMethod<void>("releaseWakeLock"); });
            }
        }
#else
    Q_UNUSED(isConnected)
#endif
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
        roles << DeviceModel::Name << DeviceModel::DeviceID << DeviceModel::DeviceVersion << DeviceModel::BatteryLevel << DeviceModel::CurrentCall << DeviceModel::IsConnected << DeviceModel::ActiveCommandTitles << DeviceModel::Checked << DeviceModel::HasListening << DeviceModel::ListeningState << DeviceModel::EnabledCommandsFiles << DeviceModel::MicsSwapped << DeviceModel::SupportsOTA << DeviceModel::HasAvailableOTA << DeviceModel::HasOTAData << DeviceModel::DeviceProgress << DeviceModel::ProgressDescription << DeviceModel::OperationInProgress << DeviceModel::OTAVersion << DeviceModel::HasLights << DeviceModel::HasShutdown << DeviceModel::HasNoPhoneMode << DeviceModel::NoPhoneModeGroups << DeviceModel::ChargingState << DeviceModel::BatteryLevelPercent << DeviceModel::HasTilt << DeviceModel::CanBalanceListening << DeviceModel::TiltEnabled << DeviceModel::KnownFirmwareMessage;
        srcNode.enableRemoting(deviceModel, "DeviceModel", roles);

        qDebug() << Q_FUNC_INFO << "Getting command model";
        CommandModel * tailCommandModel = qobject_cast<CommandModel *>(btConnectionManager->commandModel());
        qDebug() << Q_FUNC_INFO << "Replicating command model";
        roles.clear();
        roles << CommandModel::Name << CommandModel::Command << CommandModel::IsRunning << CommandModel::Category << CommandModel::Duration << CommandModel::MinimumCooldown << CommandModel::CommandIndex << CommandModel::DeviceIDs << CommandModel::IsAvailable;
        srcNode.enableRemoting(tailCommandModel, "CommandModel", roles);

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
        roles << GestureDetectorModel::NameRole << GestureDetectorModel::IdRole << GestureDetectorModel::SensorIdRole << GestureDetectorModel::SensorNameRole << GestureDetectorModel::SensorEnabledRole << GestureDetectorModel::SensorPinnedRole << GestureDetectorModel::CommandRole << GestureDetectorModel:: DefaultCommandRole << GestureDetectorModel::DevicesModel << GestureDetectorModel::FirstInSensorRole << GestureDetectorModel::VisibleRole;
        srcNode.enableRemoting(gestureController->model(), "GestureDetectorModel", roles);
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
