package se.mau;

public class Time {
    private int hour = 0;
    private int minute = 0;
    private int second = 0;

    protected String setTime(int hour, int minute, int second) {
        final boolean isValidHour = hour > -1 && hour < 24;
        final boolean isValidMinute = minute > -1 && minute < 60;
        final boolean isValidSecond = second > -1 && second < 60;

        if (isValidHour && isValidMinute && isValidSecond) {
            this.hour = hour;
            this.minute = minute;
            this.second = second;
        }
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
