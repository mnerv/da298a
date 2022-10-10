package se.mau;

import static org.junit.Assert.assertEquals;

import org.junit.Before;
import org.junit.Test;

public class TimeBoundaryTest {
    private Clock clock;
    private String current = "00:00:00";

    @Before
    public void init() {
        clock = new Clock();
        clock.ready();
    }

    @Test
    public void lowerBoundOutsideHour() {
        assertEquals(current, clock.set(-1, 0, 0));
    }

    @Test
    public void lowerBoundInsideHour() {
        assertEquals("00:00:00", clock.set(0, 0, 0));
    }

    @Test
    public void upperBoundInsideHour() {
        assertEquals("23:00:00", clock.set(23, 0, 0));
    }

    @Test
    public void upperBoundOutsideHour() {
        assertEquals(current, clock.set(24, 0, 0));
    }

    // Minute
    @Test
    public void lowerBoundOutsideMinute() {
        assertEquals(current, clock.set(0, -1, 0));
    }

    @Test
    public void lowerBoundInsideMinute() {
        assertEquals("00:00:00", clock.set(0, 0, 0));
    }

    @Test
    public void upperBoundInsideMinute() {
        assertEquals("00:59:00", clock.set(0, 59, 0));
    }

    @Test
    public void upperBoundOutsideMinute() {
        assertEquals(current, clock.set(0, 60, 0));
    }

    // Second
    @Test
    public void lowerBoundOutsideSecond() {
        assertEquals(current, clock.set(0, 0, -1));
    }

    @Test
    public void lowerBoundInsideSecond() {
        assertEquals("00:00:00", clock.set(0, 0, 0));
    }

    @Test
    public void upperBoundInsideSecond() {
        assertEquals("00:00:59", clock.set(0, 0, 59));
    }

    @Test
    public void upperBoundOutsideSecond() {
        assertEquals(current, clock.set(0, 0, 60));
    }
}
