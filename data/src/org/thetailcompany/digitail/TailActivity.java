package org.thetailcompany.digitail;

import java.lang.String;
import android.os.Bundle;
import android.util.Log;
import android.content.Intent;
import android.app.Activity;

import org.qtproject.qt5.android.bindings.QtActivity;

public class TailActivity extends org.qtproject.qt5.android.bindings.QtActivity
{
    private static final String TAG = "TailActivity";

    @Override
    protected void onPause()
    {
        super.onPause();
        Log.v(TAG, "onPause - entered");
        finish();
    }
}
