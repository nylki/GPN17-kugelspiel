#include<GPNBadge.hpp>
#include <FS.h>
#include "rboot.h"
#include "rboot-api.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define BNO055_SAMPLERATE_DELAY_MS (100)
// TFT_ILI9163C tft = TFT_ILI9163C(GPIO_LCD_CS, GPIO_LCD_DC);
// Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, GPIO_WS2813, NEO_GRB + NEO_KHZ800);
Adafruit_BNO055 bno = Adafruit_BNO055(BNO055_ID, BNO055_ADDRESS_B);

Badge badge;

// #define STORAGEDEPTH 112


 
imu::Quaternion quat;
imu::Vector<3> euler;


struct sensorData{
  float x,y,z,w;
};

struct wall {
  float x,y,w,h, color;
};

struct circle {
  float x,y,r, color;
};



struct wall walls[10];
struct circle goals[10];
circle kugelpos;

int wallcount = 4;
int goalcount = 0;
int level = 1;

sensorData dat;

void initWalls() {
  wallcount = 10;
  // wallcount = level + ((int) random(1, 2));
  for (int j = 0; j < wallcount; j++) {
    walls[j].x = random(10, 128);
    walls[j].y = random(10, 128);
    walls[j].w = random(10, 40);
    walls[j].h = random(10, 40);
    walls[j].color = 0x000000;

  }
}

void initGoals() {
  goalcount = level + ((int) random(0, 2));
  for (int j = 0; j < goalcount; j++) {
    goals[j].x = random(0, 128);
    goals[j].y = random(0, 128);
    goals[j].r = random(1, 5);

  }
}

bool checkHit() {
  for (int i = 0; i < wallcount; i++) {
    
    if (kugelpos.x < walls[i].x + walls[i].w &&
      kugelpos.x + kugelpos.r*2 > walls[i].x &&
      kugelpos.y < walls[i].y + walls[i].h &&
      kugelpos.r*2 + kugelpos.y > walls[i].y) {
        walls[i].color = 0x00F;
        return true;
      }
    }
    return false;
  }



void renderWalls() {
  for (int i = 0; i < wallcount; i++) {
    tft.drawRect(walls[i].x, walls[i].y, walls[i].w, walls[i].h, walls[i].color);
    tft.fillRect(walls[i].x, walls[i].y, walls[i].w, walls[i].h, walls[i].color);

  }
}

void renderDebugText() {
  tft.setCursor(30, 30);
  tft.setTextColor(0xABCD);
  tft.setTextSize(1);
  tft.println(dat.x, DEC);
  tft.setCursor(30, 40);
  tft.println(dat.y, DEC);
  tft.setCursor(30, 50);
  tft.println(dat.z, DEC);
  tft.setCursor(30, 60);
  tft.println(kugelpos.x, DEC);
  tft.setCursor(35, 70);
  tft.println(kugelpos.y, DEC);
}

void renderPlayer() {
  tft.drawCircle(kugelpos.x, kugelpos.y, 6, 0xFFFF);
  tft.fillCircle(kugelpos.x, kugelpos.y, 6, 0xFFFF);

}

void render() {
  tft.fillScreen(0x2222);
  renderWalls();
  renderPlayer();
  // renderDebugText();
  tft.writeFramebuffer();
}

void startLevel() {
  kugelpos.x = 60.0f;
  kugelpos.y = 60.0f;
  initWalls();
}

void setup() {
  badge.init();
  badge.setBacklight(true);
  bno.begin();
  delay(300);
  badge.setGPIO(MQ3_EN,1);
  // sensorNumber = sens.renderMenu(titles, (sizeof(titles) / sizeof(*titles)) - 1);

  rboot_config rboot_config = rboot_get_config();
  SPIFFS.begin();
  File f = SPIFFS.open("/rom"+String(rboot_config.current_rom),"w");
  f.println("Kugelspiel\n");
  tft.begin();
  startLevel();
}

void loop() {
  
  euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
 
    dat.x = euler.x()*0.125;
    dat.y = euler.y()*0.125;
    dat.z = euler.z()*0.125;
    kugelpos.x -= dat.y;
    kugelpos.y -= dat.z;
    kugelpos.x = constrain(kugelpos.x, 0, 128);
    kugelpos.y = constrain(kugelpos.y, 0, 128);
    
    if(checkHit() == true) {
      badge.setVibrator(true);
      delay(1000);
      badge.setVibrator(false);
      startLevel();
    }

  delay(0);
  render();
  



  // if (badge.getJoystickState() == JoystickState::BTN_ENTER){
  //   while(badge.getJoystickState() == JoystickState::BTN_ENTER) delay(0);
  //     sensorNumber = sens.renderMenu(titles, (sizeof(titles) / sizeof(*titles)) - 1);
  // }
}
