package se.mau;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

/**
 * Unit test for derov.
 */
public class derovTest
{
    @Test
    public void nonEmptyStringAllAlphaChars()
    {
        assertEquals("abcdefghijklmnopqrstyvwxyzåäö", rovar.derov("abobcocdodefofgoghohijkoklolmomnonopopqogrorsostotyvovwowxoxyzozåäö"));
    }

    @Test
    public void nonEmptyStringAllAlphaCharsCapital()
    {
        assertEquals("ABCDEFGHIJKLMNOPQRSTYVWXYZÅÄÖ", rovar.derov("ABOBCOCDODEFOFGOGHOHIJKOKLOLMOMNONOPOPQOGRORSOSTOTYVOVWOWXOXYZOZÅÄÖ"));
    }

    @Test
    public void nonEmptyStringNumerals()
    {
        assertEquals("1234567890", rovar.derov("1234567890"));
    }

    @Test
    public void nonEmptyStringSpecialCharacters()
    {
        assertEquals("!\"€#%&/(),.? ", rovar.derov("!\"€#%&/(),.? "));
    }

    @Test
    public void emptyString()
    {
        assertEquals("", rovar.derov(""));
    }

    @Test
    public void nullPointer()
    {
        assertEquals(null, rovar.derov(null));
    }
}
