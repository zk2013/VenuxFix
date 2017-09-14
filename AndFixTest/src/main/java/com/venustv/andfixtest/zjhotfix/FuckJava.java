package com.venustv.andfixtest.zjhotfix;

import android.os.Build;
import android.util.Log;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

/**
 * Created by zhengkai on 2017/9/14.
 */

public class FuckJava {
    public static final String TAG = "FuckJava";

    public static native boolean setup(boolean isArt, int apilevel);

    public static native void replaceMethod(Method dest, Method src);

    public static native void setFieldFlag(Field field);

    public static boolean setup() {
        try {
            final String vmVersion = System.getProperty("java.vm.version");
            boolean isArt = vmVersion != null && vmVersion.startsWith("2");
            int apilevel = Build.VERSION.SDK_INT;
            return setup(isArt, apilevel);
        } catch (Exception e) {
            Log.e(TAG, "setup", e);
            return false;
        }
    }
}
