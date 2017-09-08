package com.venustv.venusfix.patch;

import android.annotation.SuppressLint;

import java.lang.reflect.Method;

/**
 * Created by zhengkai on 2017/9/7.
 */

public class Compat {
    public static boolean isChecked = false;
    public static boolean isSupport = false;

    @SuppressLint("DefaultLocale")
    private static boolean isYunOS() {
        String version = null;
        String vmName = null;
        try {
            Method m = Class.forName("android.os.SystemProperties").getMethod(
                    "get", String.class);
            version = (String) m.invoke(null, "ro.yunos.version");
            vmName = (String) m.invoke(null, "java.vm.name");
        } catch (Exception e) {
            // nothing todo
        }
        if ((vmName != null && vmName.toLowerCase().contains("lemur"))
                || (version != null && version.trim().length() > 0)) {
            return true;
        } else {
            return false;
        }
    }
    // from android 2.3 to android 7.0
    private static boolean isSupportSDKVersion() {
        if (android.os.Build.VERSION.SDK_INT >= 8
                && android.os.Build.VERSION.SDK_INT <= 24) {
            return true;
        }
        return false;
    }

    private static boolean inBlackList() {
        return false;
    }
    public static synchronized boolean isSupport() {
        if (isChecked)
            return isSupport;
        isChecked = true;
        if (!isYunOS() && AndFix.setup() && isSupportSDKVersion()) {
            isSupport = true;
        }
        return isSupport;
    }
}
