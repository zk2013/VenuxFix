package com.venustv.venusfix.patch;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.security.PublicKey;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import javax.security.auth.x500.X500Principal;

/**
 * Created by zhengkai on 2017/9/7.
 */

public class SecurityChecker {
    Context mContext;
    public SecurityChecker(Context context) {
        mContext = context;
        init(mContext);
    }
    private static final String TAG = "VenusAndFix";
    // initialize,and check debuggable

    private static final X500Principal DEBUG_DN = new X500Principal(
            "CN=Android Debug,O=Android,C=US");
    /**
     * host publickey
     */
    private PublicKey mPublicKey;
    /**
     * host debuggable
     */
    private boolean mDebuggable;
    private void init(Context context) {



        PackageManager pm = context.getPackageManager();
        String packageName = context.getPackageName();


        try {
            PackageInfo packageInfo = pm.getPackageInfo(packageName,
                    PackageManager.GET_SIGNATURES);
            CertificateFactory certFactory = CertificateFactory
                    .getInstance("X.509");
            ByteArrayInputStream stream = new ByteArrayInputStream(
                    packageInfo.signatures[0].toByteArray());
            X509Certificate cert = (X509Certificate) certFactory
                    .generateCertificate(stream);
            mDebuggable = cert.getSubjectX500Principal().equals(DEBUG_DN);
            mPublicKey = cert.getPublicKey();
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }catch (CertificateException e) {
            Log.e(TAG, "init", e);
        }
    }
}
