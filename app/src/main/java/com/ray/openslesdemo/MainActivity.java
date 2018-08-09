package com.ray.openslesdemo;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }


    public void play(View view) {
        File file = new File(Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "mydream.pcm");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, 0);
            } else {
                playPCM(Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "mydream.pcm");
            }
            playPCM(Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "mydream.pcm");
        }
    }

    public native void playPCM(String url);

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if(grantResults.length == 1) {
            playPCM(Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "mydream.pcm");
        }
    }
}
