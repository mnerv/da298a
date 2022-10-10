package se.mau;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

/**
 * Unit test for enrov.
 */
public class enrovTest
{
    @Test
    public void nonEmptyStringAllAlphaChars()
    {
        assertEquals("abobcocdodefofgoghohijojkoklolmomnonopopqoqrorsostotyvovwowxoxyzozåäö", rovar.enrov("abcdefghijklmnopqrstyvwxyzåäö"));
    }

    @Test
    public void nonEmptyStringAllAlphaCharsCapital()
    {
        assertEquals("ABOBCOCDODEFOFGOGHOHIJOJKOKLOLMOMNONOPOPQOQRORSOSTOTYVOVWOWXOXYZOZÅÄÖ", rovar.enrov("ABCDEFGHIJKLMNOPQRSTYVWXYZÅÄÖ"));
    }

    @Test
    public void nonEmptyStringNumerals()
    {
        assertEquals("1234567890", rovar.enrov("1234567890"));
    }

    @Test
    public void nonEmptyStringSpecialCharacters()
    {
        assertEquals("!\"€#%&/(),.? ", rovar.enrov("!\"€#%&/(),.? "));
    }

    @Test
    public void emptyString()
    {
        assertEquals("", rovar.enrov(""));
    }

    @Test
    public void nullPointer()
    {
        assertEquals(null, rovar.enrov(null));
    }
}
