package se.mau;

import static org.junit.Assert.assertEquals;

import org.junit.Before;
import org.junit.Test;

/**
 * Unit test for Clock.
 */
public class ClockTest
{
    private Clock clock;
    @Before
    public void before() {
        clock = new Clock();
    }
    // Valid tests

    @Test
    public void ShowDate() {
       assertEquals("2000-01-01", clock.changeMode());
    }

    @Test
    public void readyDateChange() {
        clock.changeMode();
        assertEquals("ChangeDate", clock.ready());
    }

    @Test
    public void setDate() {
        clock.changeMode();
        clock.ready();
        assertEquals("2001-02-03", clock.set(2001, 2,3));
    }

    @Test
    public void ShowTime() {
        clock.changeMode();
        assertEquals("00:00:00", clock.changeMode());
    }

    @Test
    public void readyTimeChange() {
        assertEquals("ChangeTime", clock.ready());
    }

    @Test
    public void setTime() {
        clock.ready();
        assertEquals("01:02:03", clock.set(1,2,3));
    }

    // Invalid test

    @Test
    public void invalidReadyTime() {
        clock.ready();
        assertEquals("Er.1", clock.ready());
    }

    @Test
    public void invalidReadyDate() {
        clock.changeMode();
        clock.ready();
        assertEquals("Er.2", clock.ready());
    }

    @Test
    public void invalidReadyTimeChangeMode() {
        clock.ready();
        assertEquals("Er.3", clock.changeMode());
    }

    @Test
    public void invalidReadyDateChangeMode() {
        clock.changeMode();
        clock.ready();
        assertEquals("Er.4", clock.changeMode());
    }

    @Test
    public void invalidSetTime() {
        assertEquals("Er.5", clock.set(1,1,1));
    }

    @Test
    public void invalidSetDate() {
        clock.changeMode();
        assertEquals("Er.6", clock.set(1,1,1));
    }
}
