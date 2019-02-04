/*
 *   Copyright 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
#include "btconnectionmanager.h"
#include "btdevicemodel.h"
#include "filterproxymodel.h"
#include "settings.h"
#include "commandqueue.h"
#include "idlemode.h"

#ifdef HAS_QT5REMOTEOBJECTS
#include <QAbstractItemModelReplica>
#include "rep_settingsproxy_replica.h"
#include "rep_btconnectionmanagerproxy_replica.h"
#include "rep_commandqueueproxy_replica.h"
#endif

Q_IMPORT_PLUGIN(KirigamiPlugin)

int appMain(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//The desktop QQC2 style needs it to be a QApplication
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QAndroidJniObject::callStaticMethod<void>("org/thetailcompany/digitail/TailService",
                                                "startTailService",
                                                "(Landroid/content/Context;)V",
                                                QtAndroid::androidActivity().object());
#else
    QApplication app(argc, argv);
#endif
    //qputenv("QML_IMPORT_TRACE", "1");

    QQmlApplicationEngine engine;
//     qmlRegisterType<BTConnectionManager>("org.thetailcompany.digitail", 1, 0, "BTConnectionManager");
    qmlRegisterType<FilterProxyModel>("org.thetailcompany.digitail", 1, 0, "FilterProxyModel");
//     qmlRegisterSingletonType<SettingsProxyReplica>("org.thetailcompany.digitail", 1, 0, "AppSettings", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
//         Q_UNUSED(engine)
//         Q_UNUSED(scriptEngine)
// 
//         SettingsProxyReplica* appSettings = settingsReplica.data();
//         return appSettings;
//     });
    KirigamiPlugin::getInstance().registerTypes();

    qInfo() << "Connecting to service...";
    QRemoteObjectNode repNode;
    repNode.connectToNode(QUrl(QStringLiteral("local:digitail")));

    qInfo() << "Connected, attempting to load replicas...";
    QSharedPointer<SettingsProxyReplica> settingsReplica(repNode.acquire<SettingsProxyReplica>());
    bool res = settingsReplica->waitForSource();
    if(!res) { qCritical() << "Kapow! Replica for Settings failed to surface"; }
    engine.rootContext()->setContextProperty(QLatin1String("AppSettings"), settingsReplica.data());

    QSharedPointer<BTConnectionManagerProxyReplica> btConnectionManagerReplica(repNode.acquire<BTConnectionManagerProxyReplica>());
    res = btConnectionManagerReplica->waitForSource();
    if(!res) { qCritical() << "Kapow! Replica for btConnectionManagerReplica failed to surface"; }
    engine.rootContext()->setContextProperty(QLatin1String("BTConnectionManager"), btConnectionManagerReplica.data());

    QScopedPointer<CommandQueueProxyReplica> commandQueueReplica(repNode.acquire<CommandQueueProxyReplica>());
    res = commandQueueReplica->waitForSource();
    if(!res) { qCritical() << "Kapow! Replica for commandQueueReplica failed to surface"; }
    engine.rootContext()->setContextProperty(QLatin1String("CommandQueue"), commandQueueReplica.data());

    QScopedPointer<QAbstractItemModelReplica> btDeviceModelReplica(repNode.acquireModel("DeviceModel"));
    engine.rootContext()->setContextProperty(QLatin1String("DeviceModel"), btDeviceModelReplica.data());

    QScopedPointer<QAbstractItemModelReplica> commandModelReplica(repNode.acquireModel("CommandModel"));
    engine.rootContext()->setContextProperty(QLatin1String("CommandModel"), commandModelReplica.data());

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        qWarning() << "Failed to load the main qml file, exiting";
        return -1;
    }

    //HACK to color the system bar on Android, use qtandroidextras and call the appropriate Java methods 
#ifdef Q_OS_ANDROID
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
    qInfo() << "Service starting...";

    QRemoteObjectHost srcNode(QUrl(QStringLiteral("local:digitail")));

    qDebug() << "Creating application settings";
    AppSettings* appSettings = new AppSettings(&app);

    qDebug() << "Creating connection manager";
    BTConnectionManager* btConnectionManager = new BTConnectionManager(&app);

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
        roles << BTDeviceModel::Name << BTDeviceModel::DeviceID;
        srcNode.enableRemoting(btDeviceModel, "DeviceModel", roles);

        qDebug() << "Getting command model";
        TailCommandModel* tailCommandModel = qobject_cast<TailCommandModel*>(btConnectionManager->commandModel());
        qDebug() << "Replicating command model";
        roles.clear();
        roles << TailCommandModel::Name << TailCommandModel::Command << TailCommandModel::IsRunning << TailCommandModel::Category << TailCommandModel::Duration << TailCommandModel::MinimumCooldown << TailCommandModel::CommandIndex;
        srcNode.enableRemoting(tailCommandModel, "CommandModel", roles);

        qDebug() << "Getting command queue";
        CommandQueue* commandQueue = qobject_cast<CommandQueue*>(btConnectionManager->commandQueue());
        qDebug() << "Replicating command queue";
        srcNode.enableRemoting(commandQueue);
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
