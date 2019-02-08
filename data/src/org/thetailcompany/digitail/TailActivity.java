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
    private static TailActivity mTailActivityInstance = null;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        /* Do your staff here: */
        TailActivity.mTailActivityInstance = this;
    }

    @Override
    protected void onStop()
    {
        super.onStop();
        Log.v(TAG, "onStop - entered");
        forceCloseApp();
    }
    @Override
    protected void onDestroy()
    {
        super.onDestroy();
        mTailActivityInstance = null;
    }

    public static void forceCloseApp()
    {
        //Log.v(TAG, "forceClosingMyActivity - entered");
        mTailActivityInstance.finish();

    }
}
