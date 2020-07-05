package com.example.smartclock;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class MainActivity extends AppCompatActivity {
    Button btSave;
    EditText etLocation,etApikey,etUTC;
    ConfigModel configModel;
    boolean checknumber =false;
    FirebaseDatabase database = FirebaseDatabase.getInstance();
    DatabaseReference myRef = database.getReference("Config");
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        btSave =findViewById(R.id.btSave);
        etApikey =findViewById(R.id.etApikey);
        etLocation= findViewById(R.id.etLocation);
        etUTC=findViewById(R.id.etUTC);

        configModel =new ConfigModel("hanoi","apikey",7);
        loadData(configModel);
        btSave.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String location = etLocation.getText().toString();
                String apikey = etApikey.getText().toString();
                int utc=0;
                if(etUTC.getText().toString().matches("")){
                    Toast.makeText(MainActivity.this,"Vui lòng điền múi giờ!",Toast.LENGTH_SHORT).show();
                    checknumber =false;
                }else {

                    utc = Integer.parseInt(etUTC.getText().toString());
                    if(utc <-12 || utc >12){
                        Toast.makeText(MainActivity.this,"Vui lòng điền múi giờ trong khoảng từ -12 đến +12!",Toast.LENGTH_SHORT).show();
                        checknumber =false;
                    }else
                        checknumber =true;
                }
                if(location.matches("") || apikey.matches("") || !checknumber){
                    Toast.makeText(MainActivity.this,"Vui lòng điền đủ thông tin!",Toast.LENGTH_SHORT).show();
                }else{
                    configModel= new ConfigModel(location,apikey,utc);
                    updateConfig(configModel);
                    Toast.makeText(MainActivity.this,"Thông số được cập nhật.Vui lòng RESET thiết bị!",Toast.LENGTH_LONG).show();
                }

            }
        });
    }

    private void updateConfig(ConfigModel configModel) {
        myRef.setValue(configModel);
    }

    private void loadData(ConfigModel configModel) {
        myRef.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                // This method is called once with the initial value and again
                // whenever data at this location is updated.
                ConfigModel value = dataSnapshot.getValue(ConfigModel.class);
                Log.d("abc", "Value is: " + value.toString());
                etApikey.setText(value.API_KEY);
                etLocation.setText(value.Location);
                etUTC.setText(""+value.UTC);
            }
            @Override
            public void onCancelled(DatabaseError error) {
                // Failed to read value
                Log.w("abc", "Failed to read value.", error.toException());
            }
        });
    }
}
