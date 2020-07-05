package com.example.smartclock;

public class ConfigModel {
    public String Location;
    public String API_KEY;
    public int UTC;

    public ConfigModel() {
    }


    public ConfigModel(String location, String API_KEY, int UTC) {
        Location = location;
        this.API_KEY = API_KEY;
        this.UTC = UTC;
    }


    @Override
    public String toString() {
        return "ConfigModel{" +
                "Location='" + Location + '\'' +
                ", API_KEY='" + API_KEY + '\'' +
                ", UTC=" + UTC +
                '}';
    }
}
