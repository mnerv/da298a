package se.mau;

public class Date {
    private int year = 2000;
    private int month = 1;
    private int day = 1;

    protected String setDate(int year, int month, int day) {
        final boolean isValidYear = year > 1999 && year < 2101;
        final boolean isValidMonth = month > 0 && month < 13;
        final boolean isValidDay = day > 0 && day < 32;

        if (isValidYear && isValidMonth && isValidDay) {
            this.year = year;
            this.month = month;
            this.day = day;
        }
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
