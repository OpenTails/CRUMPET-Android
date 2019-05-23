// java file goes in android/src/org/thetailcompany/digitail/TailService.java
package org.thetailcompany.digitail;

import android.content.Context;
import android.content.Intent;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningServiceInfo;
import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.util.Log;
import org.qtproject.qt5.android.bindings.QtService;

public class TailService extends QtService
{
    // static
    // {
    //   System.loadLibrary("digitail");
    // }
    private static final String TAG = "TailService";

    private static WakeLock mWakeLock;
    private static final int ONGOING_NOTIFICATION_ID = 1;

    public static native void phoneCallHandler(String callType);

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
        if(mWakeLock != null) {
            mWakeLock.release();
            mWakeLock = null;
        }
    }

    private static boolean isMyServiceRunning(Class<?> serviceClass, Context ctx) {
        ActivityManager manager = (ActivityManager) ctx.getSystemService(Context.ACTIVITY_SERVICE);
        for (RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
            Log.v(TAG, "isMyServiceRunning: MY NAME: " + serviceClass.getName() + " SERVICE NAME: " + service.service.getClassName());
            if (serviceClass.getName().equals(service.service.getClassName())) {
                Log.v(TAG, "isMyServiceRunning: TRUE");
                return true;
            }
        }
        Log.v(TAG, "isMyServiceRunning: FALSE");
        return false;
    }

    public static void startTailService(Context ctx) {
        if(!isMyServiceRunning(TailService.class, ctx)) {
            ctx.startService(new Intent(ctx, TailService.class));
            Log.v(TAG, "startTailService: STARTED");
        }
    }

    public static void stopTailService(Context ctx) {
        ctx.stopService(new Intent(ctx, TailService.class));
        Log.v(TAG, "startTailService: STOPED");
    }

    public static void onPhoneCall(String callType) {
        Log.v(TAG, "onPhoneCall: BEFORE");
        if (callType == "MissedCall") {
            phoneCallHandler(callType);
        }
        Log.v(TAG, "onPhoneCall: AFTER");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        return START_STICKY;
    }

}
