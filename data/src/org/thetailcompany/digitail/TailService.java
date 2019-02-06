// java file goes in android/src/org/thetailcompany/digitail/TailService.java
package org.thetailcompany.digitail;

import android.content.Context;
import android.content.Intent;
import org.qtproject.qt5.android.bindings.QtService;

public class TailService extends QtService
{
    private static WakeLock mWakeLock;
    private static final int ONGOING_NOTIFICATION_ID = 1;
    public static void acquireWakeLock(Context ctx) {
        PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
        mWakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
            "DIGITAiL::TailWakeLockTag");
        mWakeLock.acquire();
        // Start foregrounding the service and put up a notification saying we're alive
        Intent notificationIntent = new Intent(this, TailService.class);
        PendingIntent pendingIntent =
                PendingIntent.getActivity(this, 0, notificationIntent, 0);

        Notification notification =
                new Notification.Builder(this, CHANNEL_DEFAULT_IMPORTANCE)
                .setContentTitle(getText(R.string.notification_title))
                .setContentText(getText(R.string.notification_message))
                .setSmallIcon(R.drawable.icon)
                .setContentIntent(pendingIntent)
                .setTicker(getText(R.string.ticker_text))
                .build();

        startForeground(ONGOING_NOTIFICATION_ID, notification);
    }

    public static void releaseWakeLock(Context ctx) {
        // Stop forcing foregrounding, and remove the notification
        stopForeground(STOP_FOREGROUND_REMOVE);
        mWakeLock.release();
    }

    public static void startTailService(Context ctx) {
        ctx.startService(new Intent(ctx, TailService.class));
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        return START_STICKY;
    }

    @Override
    public void onCreate() {
        // Check if notification should be shown and do so if needed
        super.onCreate();
    }
}
