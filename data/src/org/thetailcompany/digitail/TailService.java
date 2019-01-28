// java file goes in android/src/org/thetailcompany/digitail/TailService.java
package org.thetailcompany.digitail;

import android.content.Context;
import android.content.Intent;
import org.qtproject.qt5.android.bindings.QtService;

public class TailService extends QtService
{
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
