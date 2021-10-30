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

#include "../3rdparty/kirigami/src/kirigamiplugin.h"
#include "BTConnectionManager.h"
#include "BTDeviceModel.h"
#include "BTDeviceCommandModel.h"
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
#include "rep_SettingsProxy_replica.h"
#include "rep_BTConnectionManagerProxy_replica.h"
#include "rep_CommandQueueProxy_replica.h"
#include <rep_GestureControllerProxy_replica.h>

#include <klocalizedcontext.h>
#include <klocalizedstring.h>

Q_IMPORT_PLUGIN(KirigamiPlugin)

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
        qCritical() << "We were not granted the external storage access.";
        app.quit();
    }

#ifdef Q_OS_ANDROID
    qDebug() << "Starting service, if it isn't already...";
    QAndroidJniObject::callStaticMethod<void>("org/thetailcompany/digitail/TailService",
                                                "startTailService",
                                                "(Landroid/content/Context;)V",
                                                QtAndroid::androidActivity().object());
    qDebug() << "Service started, or already launched";
#else
    app.setApplicationVersion("Desktop");
#endif

    KLocalizedString::setApplicationDomain("digitail");
#if defined(__ANDROID__)
    KLocalizedString::addDomainLocaleDir("digitail", QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/org.kde.ki18n/"));
    QStringList languages{"da_DK", "es_ES", "fr", "nl_NL"};
    for (const QString& language : languages) {
        qDebug() << "App is translated into" << language << KLocalizedString::isApplicationTranslatedInto(language);
    }
#else
    KLocalizedString::addDomainLocaleDir("digitail", QString("%1/../locale").arg(app.applicationDirPath()));
#endif

    QIcon::setThemeSearchPaths({QStringLiteral(":/icons")});
    QIcon::setThemeName(QStringLiteral("breeze-internal"));

    qInfo() << "Creating engine";
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&app));
    qInfo() << "Registering Kirigami types";
    KirigamiPlugin::getInstance().registerTypes(&engine);
    qmlRegisterType<FilterProxyModel>("org.thetailcompany.digitail", 1, 0, "FilterProxyModel");

    QFile file(QLatin1String(":/gplv3-license-text"));
    if(file.open(QIODevice::ReadOnly)) {
        QString data(file.readAll());
        file.close();
        engine.rootContext()->setContextProperty(QLatin1String("GPLv3LicenseText"), data);
    }
    engine.rootContext()->setContextProperty(QLatin1String("AppVersion"), app.applicationVersion());

    qInfo() << "Connecting to service...";
    QRemoteObjectNode* repNode = new QRemoteObjectNode(&app);
    repNode->connectToNode(QUrl(QStringLiteral("local:digitail")));

    qInfo() << "Connected, attempting to load replicas...";
    QSharedPointer<SettingsProxyReplica> settingsReplica(repNode->acquire<SettingsProxyReplica>());
    bool res = settingsReplica->waitForSource(500);
    if(!res) {
        qInfo() << "No service exists yet, so let's start it...";
        QProcess service;
        service.startDetached(app.applicationFilePath(), QStringList() << QStringLiteral("-service"));
        QCoreApplication::processEvents();
        repNode->connectToNode(QUrl(QStringLiteral("local:digitail")));
        settingsReplica.reset(repNode->acquire<SettingsProxyReplica>());
        res = settingsReplica->waitForSource(500);
        if (!res) {
            qCritical() << "Kapow! Replica for Settings failed to surface";
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
    QObject::connect(settingsReplica.data(), &SettingsProxyReplica::languageOverrideChanged, &app, retranslate);
    retranslate();

    QSharedPointer<BTConnectionManagerProxyReplica> btConnectionManagerReplica(repNode->acquire<BTConnectionManagerProxyReplica>());
    res = btConnectionManagerReplica->waitForSource();
    if(!res) { qCritical() << "Kapow! Replica for btConnectionManagerReplica failed to surface"; }
    engine.rootContext()->setContextProperty(QLatin1String("BTConnectionManager"), btConnectionManagerReplica.data());

    QScopedPointer<CommandQueueProxyReplica> commandQueueReplica(repNode->acquire<CommandQueueProxyReplica>());
    res = commandQueueReplica->waitForSource();
    if(!res) { qCritical() << "Kapow! Replica for commandQueueReplica failed to surface"; }
    engine.rootContext()->setContextProperty(QLatin1String("CommandQueue"), commandQueueReplica.data());

    QScopedPointer<GestureControllerProxyReplica> gestureControllerReplica(repNode->acquire<GestureControllerProxyReplica>());
    res = gestureControllerReplica->waitForSource();
    if(!res) { qCritical() << "Kapow! Replica for gestureControllerReplica failed to surface"; }
    engine.rootContext()->setContextProperty(QLatin1String("GestureController"), gestureControllerReplica.data());

    QScopedPointer<QAbstractItemModelReplica> btDeviceModelReplica(repNode->acquireModel("DeviceModel"));
    engine.rootContext()->setContextProperty(QLatin1String("DeviceModel"), btDeviceModelReplica.data());

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

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty()) {
        qWarning() << "Failed to load the main qml file, exiting";
        return -1;
    }

    QObject::connect(permissionsManager, &PermissionsManager::permissionsChanged, permissionsManager, [=](){
        if(permissionsManager->hasPermission(QString("ACCESS_FINE_LOCATION"))) {
            // Don't launch the discovery immediately, let's give things a change to start up...
            QTimer::singleShot(100, btConnectionManagerReplica.data(), &BTConnectionManagerProxyReplica::startDiscovery);
        }
    });
    engine.rootContext()->setContextProperty("PermissionsManager", permissionsManager);

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
    qInfo() << "Service starting...";

    KLocalizedString::setApplicationDomain("digitail");
#if defined(__ANDROID__)
    KLocalizedString::addDomainLocaleDir("digitail", QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/org.kde.ki18n/"));
    QStringList languages{"da_DK", "es_ES", "fr", "nl_NL"};
    for (const QString& language : languages) {
        qDebug() << "App is translated into" << language << KLocalizedString::isApplicationTranslatedInto(language);
    }
#else
    KLocalizedString::addDomainLocaleDir("digitail", QString("%1/../locale").arg(app.applicationDirPath()));
#endif

    QRemoteObjectHost srcNode(QUrl(QStringLiteral("local:digitail")));

    qDebug() << "Creating application settings";
    AppSettings* appSettings = new AppSettings(&app);

    qDebug() << "Creating connection manager";
    BTConnectionManager* btConnectionManager = new BTConnectionManager(appSettings, &app);
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

    qDebug() << "Creating casual mode handler";
    IdleMode* idleMode = new IdleMode(&app);
    idleMode->setAppSettings(appSettings);
    idleMode->setConnectionManager(btConnectionManager);

    QTimer::singleShot(1, &app, [&srcNode, appSettings, btConnectionManager]() {
        qDebug() << "Replicating application settings";
        srcNode.enableRemoting(appSettings);

        qDebug() << "Replicating connection manager";
        srcNode.enableRemoting(btConnectionManager);

        qDebug() << "Getting device model";
        BTDeviceModel* btDeviceModel = qobject_cast<BTDeviceModel*>(btConnectionManager->deviceModel());
        qDebug() << "Replicating device model";
        QVector<int> roles;
        roles << BTDeviceModel::Name << BTDeviceModel::DeviceID << BTDeviceModel::DeviceVersion << BTDeviceModel::BatteryLevel << BTDeviceModel::CurrentCall << BTDeviceModel::IsConnected << BTDeviceModel::ActiveCommandTitles << BTDeviceModel::Checked << BTDeviceModel::HasListening << BTDeviceModel::ListeningState << BTDeviceModel::EnabledCommandsFiles << BTDeviceModel::MicsSwapped << BTDeviceModel::SupportsOTA << BTDeviceModel::HasAvailableOTA << BTDeviceModel::HasOTAData;
        srcNode.enableRemoting(btDeviceModel, "DeviceModel", roles);

        qDebug() << "Getting command model";
        BTDeviceCommandModel* tailCommandModel = qobject_cast<BTDeviceCommandModel*>(btConnectionManager->commandModel());
        qDebug() << "Replicating command model";
        roles.clear();
        roles << BTDeviceCommandModel::Name << BTDeviceCommandModel::Command << BTDeviceCommandModel::IsRunning << BTDeviceCommandModel::Category << BTDeviceCommandModel::Duration << BTDeviceCommandModel::MinimumCooldown << BTDeviceCommandModel::CommandIndex << BTDeviceCommandModel::DeviceIDs << BTDeviceCommandModel::IsAvailable;
        srcNode.enableRemoting(tailCommandModel, "CommandModel", roles);

        qDebug() << "Getting command queue";
        CommandQueue* commandQueue = qobject_cast<CommandQueue*>(btConnectionManager->commandQueue());
        qDebug() << "Replicating command queue";
        srcNode.enableRemoting(commandQueue);

        qDebug() << "Creating gesture controller";
        GestureController* gestureController = new GestureController(btConnectionManager);
        gestureController->setConnectionManager(btConnectionManager);
        qDebug() << "Replicating gesture controller";
        srcNode.enableRemoting(gestureController);

        qDebug() << "Getting and replicating gesture detector model";
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
