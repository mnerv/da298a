package se.mau;

public class Date {
    private int year = 2000;
    private int month = 1;
    private int day = 1;

    protected String setDate(int year, int month, int day) {
        this.year = year;
        this.month = month;
        this.day = day;
        return showDate();
    }

    protected String showDate() {
        String output = "";
        output += year+"-";
        output += (month<10 ? "0"+month: month)+"-";
        output += day<10 ? "0"+day: day;
        return output;
    }
}
