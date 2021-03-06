#include<GPNBadge.hpp>
#include <FS.h>
#include "rboot.h"
#include "rboot-api.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF
#define MAXLEVEL 23
#define WINSCREEN 0

#define BNO055_SAMPLERATE_DELAY_MS (100)
Adafruit_BNO055 bno = Adafruit_BNO055(BNO055_ID, BNO055_ADDRESS_B);

Badge badge;

imu::Quaternion quat;
imu::Vector<3> euler;


struct sensorData{
  float x,y,z,w;
};

struct wall {
  float x,y,w,h, color;
};

struct circle {
  float x,y,r, color, checked;
};

struct intvec {
  int x,y;
};


struct wall walls[20];
struct circle goals[20];
circle kugelpos;

int middle = 60;
int wallcount = 4;
int goalcount = 1;
int level = 1;
int lifes = 3;
int goalsReached = 0;
intvec winscreenpos;

sensorData dat;


int avoidMiddlePos(int a) {
  if(a < middle && a > middle - 20) {
    a = middle - 20;
  } else if(a > middle && a < middle + 20) {
    a = 70;
  }
  return a;
}

void initWalls() {
  memset(walls, 0, sizeof(walls));
  wallcount = constrain(level + 2, 0, 19);
  for (int j = 0; j < wallcount; j++) {
    walls[j].x = avoidMiddlePos(random(10, 128));
    walls[j].y = avoidMiddlePos(random(10, 128));
    walls[j].w = random(4, 10);
    walls[j].h = random(4, 43);
    walls[j].color = RED;
  }
}

void initGoals() {
  memset(goals, 0, sizeof(goals));
  goalcount = level + 1;
  for (int j = 0; j < goalcount; j++) {
    
    goals[j].x = random(0, 128);
    goals[j].y = random(0, 128);
    goals[j].r = random(11, 20);
    goals[j].color = GREEN;
    goals[j].checked = false;
  }
}

void initPlayer() {
  kugelpos.x = 60.0f;
  kugelpos.y = 60.0f;
  kugelpos.r = 4;
  kugelpos.color = WHITE;
}

int checkHitWalls() {
  for (int i = 0; i < wallcount; i++) {
    
    if (kugelpos.x < walls[i].x + walls[i].w &&
      kugelpos.x + kugelpos.r > walls[i].x &&
      kugelpos.y < walls[i].y + walls[i].h &&
      kugelpos.r + kugelpos.y > walls[i].y) {
        return i;
      }
    }
    return -1;
  }
  
  int checkHitGoals() {
    for (int i = 0; i < goalcount; i++) {
      if(goals[i].checked == true) continue;
      
      int dx = kugelpos.x - goals[i].x;
      int dy = kugelpos.y - goals[i].y;
      int dist = sqrt(dx * dx + dy * dy);
      if (dist < kugelpos.r + goals[i].r) {
        return i;
      }
    }
    return -1;
  }
  
  void moveRandomBlock() {
    int r = random(0, wallcount);
    walls[r].x += ((random(0, 100) - 50) / 100.0);
    walls[r].y += ((random(0, 100) - 50) / 100.0);
  }
    
    
    
    void renderWalls() {
      for (int i = 0; i < wallcount; i++) {
        tft.drawRect(walls[i].x, walls[i].y, walls[i].w, walls[i].h, walls[i].color);
        tft.fillRect(walls[i].x, walls[i].y, walls[i].w, walls[i].h, walls[i].color);
      }
    }
    
    void renderGoals() {
      for (int i = 0; i < goalcount; i++) {
        if(goals[i].checked == true) continue;
        tft.drawCircle(goals[i].x, goals[i].y, goals[i].r, goals[i].color);
        tft.fillCircle(goals[i].x, goals[i].y, goals[i].r, goals[i].color);
      }
    }
    
    void renderDebugText() {
      tft.setCursor(30, 30);
      tft.setTextColor(WHITE);
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
    
    void renderInfo() {
      
      for (size_t i = 0; i < lifes; i++) {
        tft.setCursor(110 + (i*5), 3);
        tft.println("*");
      }
      tft.setCursor(5, 5);
      tft.setTextColor(WHITE);
      tft.setTextSize(1);
      tft.print(level, DEC);
      tft.print(" / ");
      tft.print(MAXLEVEL, DEC);
    }
    
    void renderPlayer() {
      tft.drawCircle(kugelpos.x, kugelpos.y, kugelpos.r, kugelpos.color);
      tft.fillCircle(kugelpos.x, kugelpos.y, kugelpos.r, kugelpos.color);
      
    }
    
    void renderWinScreen() {
      winscreenpos.y = (winscreenpos.y + 1) % 128;
      tft.setCursor(winscreenpos.x, winscreenpos.y);
      tft.setTextColor(MAGENTA);
      tft.setTextSize(2);
      tft.println("*confetti throw*");
      tft.println("*You have won!*");
    }
    
    void render() {
      tft.fillScreen(BLACK);

      if(level == WINSCREEN) {
        // If winscreen
        renderWinScreen();
      } else {
        renderGoals();
        renderWalls();
        renderPlayer();
        renderInfo();
      }
      // renderDebugText();
      tft.writeFramebuffer();
    }
    
    void startLevel(int l) {
      level = l;
      if(level == 1) {
        lifes = 3;
      }
      goalsReached = 0;

      initPlayer();
      initWalls();
      initGoals();
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
      
      startLevel(1);
    }
    
    void loop() {
      
      if(level == WINSCREEN) {
        delay(0);
        if (badge.getJoystickState() == JoystickState::BTN_ENTER) {
          startLevel(1);
        }
        render();
        return;
      }
      
      if(level == MAXLEVEL) {
        moveRandomBlock();
      }
      
      euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
      
      dat.x = euler.x()*0.125;
      dat.y = euler.y()*0.125;
      dat.z = euler.z()*0.125;
      
      // Add rotation value to player positions
      // the more the badge is rotated the bigger the position change
      // Not doing velocity+acceleration calculations here for gameplay reasons
      kugelpos.x -= dat.y;
      kugelpos.y -= dat.z;
      kugelpos.x = constrain(kugelpos.x, 0, 128);
      kugelpos.y = constrain(kugelpos.y, 0, 128);
      

      int goalhit = checkHitGoals();
      if(goalhit != -1 && goals[goalhit].checked == false) {
        // points += 20 / goals[goalhit].radius;
        goals[goalhit].checked = true;
        goals[goalhit].color = BLACK;
        goalsReached += 1;
        
        // check if level finished last frame
        if(goalsReached == goalcount) {
        
          if(level == MAXLEVEL) {
            level = 0;
            winscreenpos.x = 0;
            winscreenpos.y = 0;
          } else {
            startLevel(level + 1);
            render();
            delay(1000);
            return;
          }
          
        }
        
      }
      

      
      int wallhit = checkHitWalls();
      
      if(wallhit != -1) {
        lifes--;
        if(lifes >= 1) {
          // reset position on life loss
          badge.setVibrator(true);
          delay(100);
          badge.setVibrator(false);
          kugelpos.x = 60.0f;
          kugelpos.y = 60.0f;
        } else if (lifes == 0) {
          badge.setVibrator(true);
          delay(1000);
          badge.setVibrator(false);
          startLevel(1);
          delay(1000);
        }

      }
      

      
      delay(0);
      render();
      
      
      
      
      // if (badge.getJoystickState() == JoystickState::BTN_ENTER){
      //   while(badge.getJoystickState() == JoystickState::BTN_ENTER) delay(0);
      //     sensorNumber = sens.renderMenu(titles, (sizeof(titles) / sizeof(*titles)) - 1);
      // }
    }
