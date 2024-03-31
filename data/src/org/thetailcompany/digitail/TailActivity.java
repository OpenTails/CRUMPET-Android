package org.thetailcompany.digitail;

import java.lang.String;
import android.os.Bundle;
import android.util.Log;
import android.content.Intent;
import android.app.Activity;

import org.qtproject.qt.android.bindings.QtActivity;

public class TailActivity extends org.qtproject.qt.android.bindings.QtActivity
{
    private static final String TAG = "TailActivity";
    // No longer needed as of Qt6 i guess - and looks like it interferes with bringing the app back up from sleep as well, so probably not great anyway
    // Leaving in place just for now, can be removed later if it does turn out to truly not to be a problem any longer
    // @Override
    // protected void onPause()
    // {
        // super.onPause();
        // Log.v(TAG, "onPause - entered");
    // }
}
