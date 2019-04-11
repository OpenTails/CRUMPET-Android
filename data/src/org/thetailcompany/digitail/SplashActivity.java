package org.thetailcompany.digitail;

import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.net.Uri;
import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.widget.VideoView;
import android.view.MotionEvent;

public class SplashActivity extends Activity
{
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        try
        {
            VideoView videoHolder = new VideoView(this);
            setContentView(videoHolder);
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