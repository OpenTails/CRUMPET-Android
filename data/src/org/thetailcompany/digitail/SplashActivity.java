package org.thetailcompany.digitail;

import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.net.Uri;
import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.widget.VideoView;
import android.widget.RelativeLayout;
import android.view.MotionEvent;
import android.graphics.Color;
import android.preference.PreferenceManager;
import android.content.SharedPreferences;

public class SplashActivity extends Activity
{
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        try
        {
            SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
            boolean isPlaySplashVideo = preferences.getBoolean("isPlaySplashVideo", true);
            if (!isPlaySplashVideo)
            {
                jump();
                return;
            }

            SharedPreferences.Editor editor = preferences.edit();
            editor.putBoolean("isPlaySplashVideo", false);
            editor.commit();

            // Create parent RelativeLayout
            RelativeLayout parentLayout = new RelativeLayout(this);
            RelativeLayout.LayoutParams parentLayoutParam = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
            parentLayout.setLayoutParams(parentLayoutParam);

            // Create child VideoView
            VideoView videoHolder = new VideoView(this);
            RelativeLayout.LayoutParams videoHolderParams = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
            videoHolder.setLayoutParams(videoHolderParams);
            videoHolderParams.addRule(RelativeLayout.CENTER_IN_PARENT);

            // Add child VideoView to parent RelativeLayout
            parentLayout.addView(videoHolder);

            // Set parent RelativeLayout to your screen
            setContentView(parentLayout, parentLayoutParam);
            getWindow().getDecorView().setBackgroundColor(Color.WHITE);

            Uri video = Uri.parse("android.resource://" + getPackageName() + "/" + R.raw.splash);
            videoHolder.setVideoURI(video);

            videoHolder.setOnCompletionListener(new MediaPlayer.OnCompletionListener()
            {
                public void onCompletion(MediaPlayer mp)
                {
                    jump();
                }
            });

            videoHolder.start();
        }
        catch (Exception ex)
        {
            jump();
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        jump();
        return true;
    }

    private void jump()
    {
        if (isFinishing())
        {
            return;
        }

        startActivity(new Intent(this, TailActivity.class));
        finish();
    }
}