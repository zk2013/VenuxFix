package com.venustv.andfixtest;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import com.venustv.andfixtest.zjhotfix.FuckJava;

import java.lang.reflect.Method;

public class MainActivity extends AppCompatActivity {

    public static final String TAG = "FuckJava";

    public Tom mCat;

    public static boolean mHasSetup = false;

    public void doFix() {
        System.loadLibrary("zjandfix");

        if (false == mHasSetup) {
            if ( FuckJava.setup() ) {
                mHasSetup = true;
            } else {
                Log.e(TAG, "setup fail");
            }
        }

        Class tom = Tom.class;
        Method tom_say = null;
        try {
            tom_say = tom.getMethod("say");
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }

        Class tom_fix = Tom_Fix.class;
        Method tom_fix_say = null;
        try {
            tom_fix_say = tom_fix.getMethod("say");
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }
        if (tom_say != null && tom_fix_say != null) {
            FuckJava.replaceMethod(tom_say, tom_fix_say);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mCat = new Tom();

        // tom say
        findViewById(R.id.tom).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Toast.makeText(MainActivity.this, mCat.say(), Toast.LENGTH_SHORT).show();
            }
        });

        // do hot fix
        findViewById(R.id.fix).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                doFix();
            }
        });
    }
}
