package net.mulliken.pinenotenotebook;

import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity implements AdapterView.OnItemSelectedListener{

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Set up the spinner
        Spinner displayModeSelector = findViewById(R.id.display_mode_selector);
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this, R.array.display_mode_array, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        displayModeSelector.setAdapter(adapter);
        displayModeSelector.setOnItemSelectedListener(this);

        // Set up the clear screen button
        Button clearScreenButton = findViewById(R.id.button_clear_screen);
        clearScreenButton.setOnClickListener(v -> {
            Toast.makeText(getApplicationContext(), "Cleared screen", Toast.LENGTH_SHORT).show();
            NoteJNI noteJNI = NoteJNI.getInstance();

            noteJNI.clearDisplay();
        });
    }

    // Handle the selection of the dropdown
    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        if (parent.getId() == R.id.display_mode_selector) {
            String displayMode = parent.getItemAtPosition(position).toString();

            NoteJNI noteJNI = NoteJNI.getInstance();
            try {
                noteJNI.setDisplayMode(displayMode);
            } catch (NumberFormatException e) {
                Toast.makeText(this, "Display mode invalid", Toast.LENGTH_SHORT).show();
            }
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {
        // Do nothing
    }
}