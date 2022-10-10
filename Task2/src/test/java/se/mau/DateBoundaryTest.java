package se.mau;

import static org.junit.Assert.assertEquals;

import org.junit.Before;
import org.junit.Test;

public class DateBoundaryTest {
    private Clock clock;
    private String current = "2000-01-01";

    @Before
    public void init() {
        clock = new Clock();
        clock.changeMode();
        clock.ready();
    }

    @Test
    public void lowerBoundOutsideYear() {
        assertEquals(current, clock.set(1999, 1, 1));
    }

    @Test
    public void lowerBoundInsideYear() {
        assertEquals("2000-01-01", clock.set(2000, 1, 1));
    }

    @Test
    public void upperBoundInsideYear() {
        assertEquals("2100-01-01", clock.set(2100, 1, 1));
    }

    @Test
    public void upperBoundOutsideYear() {
        assertEquals(current, clock.set(2101, 1, 1));
    }

    // Month
    @Test
    public void lowerBoundOutsideMonth() {
        assertEquals(current, clock.set(2000, 0, 1));
    }

    @Test
    public void lowerBoundInsideMonth() {
        assertEquals("2000-01-01", clock.set(2000, 1, 1));
    }

    @Test
    public void upperBoundInsideMonth() {
        assertEquals("2000-12-01", clock.set(2000, 12, 1));
    }

    @Test
    public void upperBoundOutsideMonth() {
        assertEquals(current, clock.set(2000, 13, 1));
    }

    // Day
    @Test
    public void lowerBoundOutsideDay() {
        assertEquals(current, clock.set(2000, 1, 0));
    }

    @Test
    public void lowerBoundInsideDay() {
        assertEquals("2000-01-01", clock.set(2000, 1, 1));
    }

    @Test
    public void upperBoundInsideDay() {
        assertEquals("2000-01-31", clock.set(2000, 1, 31));
    }

    @Test
    public void upperBoundOutsideDay() {
        assertEquals(current, clock.set(2000, 1, 32));
    }
}
