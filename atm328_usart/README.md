Atm128 CircBuffer

```
24.03.2020: code cleanup saves about 122 Bytes of RAM !
            Removed unused local buffer for strings and all methods related to it.
            Next step is to move all methods definitions form *.h file to *.cpp file
            it may save some extra RAM if there are many instances of CircBuffer in use.
```
