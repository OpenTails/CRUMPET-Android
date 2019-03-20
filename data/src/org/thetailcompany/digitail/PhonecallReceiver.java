package org.thetailcompany.digitail;

import java.util.Date;

import android.util.Log;
import android.content.Context;

public class PhonecallReceiver extends AbstractPhonecallReceiver
{
  private static final String TAG = "DigitailPhonecallReceiver";

  @Override
  protected void onIncomingCallReceived(Context ctx, String number, Date start)
  {
      //
      Log.v(TAG, "onIncomingCallReceived");
  }

  @Override
  protected void onIncomingCallAnswered(Context ctx, String number, Date start)
  {
      //
      Log.v(TAG, "onIncomingCallAnswered");
  }

  @Override
  protected void onIncomingCallEnded(Context ctx, String number, Date start, Date end)
  {
      //
      Log.v(TAG, "onIncomingCallEnded");
  }

  @Override
  protected void onOutgoingCallStarted(Context ctx, String number, Date start)
  {
      //
      Log.v(TAG, "onOutgoingCallStarted");
  } 

  @Override 
  protected void onOutgoingCallEnded(Context ctx, String number, Date start, Date end)
  {
      //
      Log.v(TAG, "onOutgoingCallEnded");
  }

  @Override
  protected void onMissedCall(Context ctx, String number, Date start)
  {
      //
      Log.v(TAG, "onMissedCall");
  }
}