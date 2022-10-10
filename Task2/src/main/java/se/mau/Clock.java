package se.mau;

/**
 * Hello world!
 *
 */
public class Clock
{
    private Time time = new Time();
    private Date date = new Date();

    private enum State {
        S1,
        S2,
        S3,
        S4
    }
    private State currentState = State.S1;

    public String changeMode() {
        switch (currentState) {
            case S1:
                currentState = State.S2;
                return date.showDate();
            case S2:
                currentState = State.S1;
                return time.showTime();
            case S3:
                return "Er.3";
            case S4:
                return "Er.4";
            default:
                return "Invalid enum";
        }
    }

    public String ready() {
        switch (currentState){
            case S1:
                currentState = State.S3;
                return "ChangeTime";
            case S2:
                currentState = State.S4;
                return "ChangeDate";
            case S3:
                return "Er.1";
            case S4:
                return "Er.2";
            default:
                return "Invalid enum";
        }
    }

    public String set(int a, int b, int c) {
        switch (currentState){
            case S3:
                currentState = State.S1;
                return time.setTime(a,b,c);
            case S4:
                currentState = State.S2;
                return date.setDate(a,b,c);
            case S1:
                return "Er.5";
            case S2:
                return "Er.6";
            default:
                return "Invalid enum";
        }
    }
}
