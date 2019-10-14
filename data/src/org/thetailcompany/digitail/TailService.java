// java file goes in android/src/org/thetailcompany/digitail/TailService.java
package org.thetailcompany.digitail;

import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningServiceInfo;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.NotificationChannel;
import android.app.PendingIntent;
import android.app.Service;
import org.qtproject.qt5.android.bindings.QtService;

public class TailService extends QtService
{
    private static WakeLock mWakeLock;
    private static final int ONGOING_NOTIFICATION_ID = 1;
    private static final String NOTIFICATION_CHANNEL_ID = "org.thetailcompany.digitail";

    private synchronized String createChannel() {
        NotificationManager mNotificationManager = (NotificationManager) this.getSystemService(Context.NOTIFICATION_SERVICE);

        String channelName = "TailService";
        int importance = NotificationManager.IMPORTANCE_LOW;

        NotificationChannel mChannel = new NotificationChannel(NOTIFICATION_CHANNEL_ID, channelName, importance);

        if (mNotificationManager != null) {
            mNotificationManager.createNotificationChannel(mChannel);
        } else {
            stopSelf();
        }
        return NOTIFICATION_CHANNEL_ID;
    }

    public void acquireWakeLock() {
        PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
        mWakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
            "DIGITAiL::TailWakeLockTag");
        mWakeLock.acquire();

        // Start foregrounding the service and put up a notification saying we're alive
        Intent notificationIntent = new Intent(this, TailActivity.class);
        notificationIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        PendingIntent pendingIntent =
                PendingIntent.getActivity(this, 0, notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        String channel;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
            channel = createChannel();
        else {
            channel = "";
        }
        Notification notification = new Notification.Builder(this, channel)
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
        if(mWakeLock != null) {
            mWakeLock.release();
            mWakeLock = null;
        }
    }

    private static boolean isMyServiceRunning(Class<?> serviceClass, Context ctx) {
        ActivityManager manager = (ActivityManager) ctx.getSystemService(Context.ACTIVITY_SERVICE);
        for (RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
            if (serviceClass.getName().equals(service.service.getClassName())) {
                return true;
            }
        }
        return false;
    }
    public static void startTailService(Context ctx) {
        if(!isMyServiceRunning(TailService.class, ctx)) {
            ctx.startService(new Intent(ctx, TailService.class));
        }
    }
    public static void stopTailService(Context ctx) {
        ctx.stopService(new Intent(ctx, TailService.class));
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        return START_STICKY;
    }

}
