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
      onPhoneCall(ctx, "IncomingCallReceived");
  }

  @Override
  protected void onIncomingCallAnswered(Context ctx, String number, Date start)
  {
      onPhoneCall(ctx, "IncomingCallAnswered");
  }

  @Override
  protected void onIncomingCallEnded(Context ctx, String number, Date start, Date end)
  {
      onPhoneCall(ctx, "IncomingCallEnded");
  }

  @Override
  protected void onOutgoingCallStarted(Context ctx, String number, Date start)
  {
      onPhoneCall(ctx, "OutgoingCallStarted");
  } 

  @Override 
  protected void onOutgoingCallEnded(Context ctx, String number, Date start, Date end)
  {
      onPhoneCall(ctx, "OutgoingCallEnded");
  }

  @Override
  protected void onMissedCall(Context ctx, String number, Date start)
  {
      onPhoneCall(ctx, "MissedCall");
  }

  private void onPhoneCall(Context ctx, String callType)
  {
      Log.v(TAG, callType);
      TailService.startTailService(ctx);
      TailService.onPhoneCall(callType);
  }
}