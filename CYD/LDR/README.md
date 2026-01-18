### LDR

This shows how to use the light dependent resistor on the CYD.

```
      -------         ----------
GND---| LDR |---------|resistor|-----3.3V
      -------    |    ----------     
               GPIO34
```

The LDR is connected in series with another resistor between ground and 3.3V, and GPIO 34 is connected to the junction point of the two resistors. The result is that the voltage at GPIO 34 decreases with inreasing light intensity. By reading the voltage using:

    analogRead(34);

you can get a rough idea of the light intensity.

However on my board this is not very useful as it is far too sensitive and reads zero for any but the very dimmest light. Possibly you might get it to work if you place some form of filter over the resistor, but if you need to measure light intensity you would be better of using a more sophisticated light sensor.