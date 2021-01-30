#include <M5EPD.h>

// M5EPD_Canvas canvas(&M5.EPD);

void ButtonTest(const char *str)
{
    // canvas.fillCanvas(0);
    // canvas.drawString(str, 100, 100);
    // canvas.pushCanvas(100,300,UPDATE_MODE_DU4);
    // delay(500);
}

void setup()
{

    M5.begin();
    // M5.EPD.SetRotation(90);
    // M5.EPD.Clear(true);
    // canvas.createCanvas(300, 300);
    // canvas.setTextSize(3);
}

void loop()
{
    if (M5.BtnL.isPressed())
        ButtonTest("Btn L Pressed");
    if (M5.BtnP.isPressed())
        ButtonTest("Btn P Pressed");
    if (M5.BtnR.isPressed())
        ButtonTest("Btn R Pressed");
    M5.update();
    delay(100);
}