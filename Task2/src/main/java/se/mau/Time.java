package se.mau;

public class Time {
    private int hour = 0;
    private int minute = 0;
    private int second = 0;

    protected String setTime(int hour, int minute, int second) {
        this.hour = hour;
        this.minute = minute;
        this.second = second;
        return showTime();
    }

    protected String showTime() {
        String output = "";
        output += (hour<10 ? "0"+hour: hour)+":";
        output += (minute<10 ? "0"+minute: minute)+":";
        output += second<10 ? "0"+second: second;
        return output;
    }
}
