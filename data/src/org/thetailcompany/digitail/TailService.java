// java file goes in android/src/org/thetailcompany/digitail/TailService.java
package org.thetailcompany.digitail;

import android.content.Context;
import android.content.Intent;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import org.qtproject.qt5.android.bindings.QtService;

public class TailService extends QtService
{
    private static WakeLock mWakeLock;
    private static final int ONGOING_NOTIFICATION_ID = 1;
    public void acquireWakeLock() {
        PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
        mWakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
            "DIGITAiL::TailWakeLockTag");
        mWakeLock.acquire();
        // Start foregrounding the service and put up a notification saying we're alive
        Intent notificationIntent = new Intent(this, TailService.class);
        PendingIntent pendingIntent =
                PendingIntent.getActivity(this, 0, notificationIntent, 0);

        Notification notification =
                new Notification.Builder(this)
                .setContentTitle(getText(R.string.notification_title))
                .setContentText(getText(R.string.notification_message))
                .setSmallIcon(R.mipmap.ic_launcher)
                .setContentIntent(pendingIntent)
                .setTicker(getText(R.string.ticker_text))
                .build();

        startForeground(ONGOING_NOTIFICATION_ID, notification);
    }

    public void releaseWakeLock() {
        // Stop forcing foregrounding, and remove the notification
        stopForeground(true);
        mWakeLock.release();
    }

    private boolean isMyServiceRunning(Class<?> serviceClass) {
        ActivityManager manager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        for (RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
            if (serviceClass.getName().equals(service.service.getClassName())) {
                return true;
            }
        }
        return false;
    }
    public static void startTailService(Context ctx) {
        if(!isMyServiceRunning(TailService.class)) {
            ctx.startService(new Intent(ctx, TailService.class));
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        return START_STICKY;
    }

}
