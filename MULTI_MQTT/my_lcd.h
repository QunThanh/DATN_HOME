#ifndef __MY_LCD_H__
#define __MY_LCD_H__

/*  addresses I2C
 *  A2 - A1 - A0
 *  0  - 0  - 0 = 0x20
 *  0  - 0  - 1 = 0x21
 *  0  - 1  - 0 = 0x22
 *  0  - 1  - 1 = 0x23
 *  1  - 0  - 0 = 0x24
 *  1  - 0  - 1 = 0x25
 *  1  - 1  - 0 = 0x26
 *  1  - 1  - 1 = 0x27   // #default LCD
 */
#ifndef LCD_I2C_ADDR
#define LCD_I2C_ADDR           0x27      // default address
#endif

#ifndef LCD_I2C_NUM_COL
#define LCD_I2C_NUM_COL        20        
#endif

#ifndef LCD_I2C_NUM_ROW
#define LCD_I2C_NUM_ROW        4          
#endif

#include <LiquidCrystal_I2C.h>


uint8_t Address_LCD = LCD_I2C_ADDR;
uint8_t cols_LCD = LCD_I2C_NUM_COL;
uint8_t rows_LCD = LCD_I2C_NUM_ROW;

LiquidCrystal_I2C lcd(Address_LCD, cols_LCD, rows_LCD);

void printLcd(int colum, int row, String message)
{
    if (message.length() > cols_LCD)
    {
      lcd.setCursor(0,0);
      lcd.print("maxLength<" + String(cols_LCD));
      lcd.setCursor(0,1);
      lcd.print("yourMsg:"+ String( message.length() ));
      return;
    }    
    lcd.setCursor(colum, row);
    lcd.print(message);
}

void clearLcd()
{ 
    lcd.clear();
}

void scrollLcd(int col, int row, String message, int delayTime, int lenView)
{
    String messagelong;
    int character = lenView;
    if (message.length() > character)
    {
        for (int i = 0; i < character; i++)
            messagelong = "" + message + "-" + message;
        for (int pos = 0; pos <= (message.length() + 1); pos++)
        {
            lcd.setCursor(col, row);
            lcd.print(messagelong.substring(pos, pos + character));
            delay(delayTime);
        }
        return;
    }
    lcd.setCursor(col, row);
    lcd.print(message);
}

/**
 * @param light 1:on  0:off
 */
void backLightLcd(bool light)
{
    light ? lcd.backlight() : lcd.noBacklight();
}

void setupLcd()
{
    lcd.init();

    backLightLcd(1);
    
    Serial.println("Setup LCD done!!!");
}

#endif