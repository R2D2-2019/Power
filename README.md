# Power

#### Fix "error: invalid cast to abstract class type 'due::pin_adc'"

This error is caused by HWLIB, the temporary fix for this is to add a function to the file 'hwlib-arduino-due.hpp'.

In this file, add the code below on line 895

```
 void refresh(){}
