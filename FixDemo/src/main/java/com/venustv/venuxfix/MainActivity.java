package com.venustv.venuxfix;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import com.venustv.dexposedj.DexposedBridge;
import com.venustv.dexposedj.XC_MethodHook;
import com.venustv.dexposedj.XC_MethodReplacement;
import com.venustv.venuxfix.test.Cat;
import com.venustv.venuxfix.test.Dog;

import java.io.IOException;

public class MainActivity extends AppCompatActivity {
public Cat mCat;
    public Dog mDog;

    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE" };


    public void doHotFix() {
        /*
        // initialize
        mPatchManager = new PatchManager(this);
        mPatchManager.init("1.0");
        Log.d(TAG, "inited.");

        // load patch
        mPatchManager.loadPatch();
        Log.d(TAG, "apatch loaded.");
        // add patch at runtime
        try {
            // .apatch file path
            String patchFileString = Environment.getExternalStorageDirectory()
                    .getAbsolutePath() + VPATCH_PATH;
            mPatchManager.addPatch(patchFileString);
            Log.d(TAG, "apatch:" + patchFileString + " added.");
        } catch (IOException e) {
            Log.e(TAG, "", e);
        }//*/
    }

    public void doHotFixUsingDexposedFramework() {
        // tom cat
        doMethodHook();

        // dog
        doMethodReplace();
    }

    public void doMethodReplace() {
        DexposedBridge.findAndHookMethod(Dog.class, "say", new XC_MethodReplacement() {
            @Override
            protected Object replaceHookedMethod(MethodHookParam param) throws Throwable {
                return "101忠狗汪汪汪";
            }
        });
    }

    public void doMethodHook() {
        DexposedBridge.findAndHookMethod(Cat.class,"say", new XC_MethodHook() {
            @Override
            protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                Log.i(TAG, "beforeHookedMethod");
            }

            @Override
            protected void afterHookedMethod(MethodHookParam param) throws Throwable  {
                Log.i(TAG, "afterHookedMethod");
                param.setResult("喵喵");
            }
        });
    }

    public static void verifyStoragePermissions(Activity activity) {

        try {
            //检测是否有写的权限
            int permission = ActivityCompat.checkSelfPermission(activity,
                    "android.permission.WRITE_EXTERNAL_STORAGE");
            if (permission != PackageManager.PERMISSION_GRANTED) {
                // 没有写的权限，去申请写的权限，会弹出对话框
                ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE,REQUEST_EXTERNAL_STORAGE);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static final String TAG = "VenusFix";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        verifyStoragePermissions(this);
        mCat = new Cat();
        mDog = new Dog();

        findViewById(R.id.cat).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Toast.makeText(MainActivity.this, mCat.say(), Toast.LENGTH_SHORT).show();
            }
        });

        findViewById(R.id.dog).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Toast.makeText(MainActivity.this, mDog.say(), Toast.LENGTH_SHORT).show();
            }
        });

        findViewById(R.id.fix).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MainActivity.this.doHotFixUsingDexposedFramework();
            }
        });
    }
}
