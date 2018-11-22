// g++ processscheduling.cpp -lallegro_primitives -lallegro
#include <stdio.h>
#include "allegro5/allegro.h"
#include <allegro5/allegro_primitives.h>
#include <cmath>
#include <vector>

#define PI M_PI

#define TOTAL_WIDTH 1240
#define TOTAL_HEIGHT 750
#define TOTAL_TASKS 2

#define TOTAL_TIME 30
#define MAX_TIME 25
#define WAIT_FACTOR 0.025
#define LOOP_TILL TOTAL_TIME/WAIT_FACTOR
#define INITAL_WAIT 3

#define CONTENT_START_X  60
#define CONTENT_START_Y  220
#define CONTENT_END_X  TOTAL_WIDTH-50
#define CONTENT_END_Y  TOTAL_HEIGHT - 50
#define processLineSeparationDis 100
#define totalTimeLineLength CONTENT_END_X - CONTENT_START_X
#define serverCapacityLabelDis TOTAL_HEIGHT/25

using namespace std;

static void *Func_Thread(ALLEGRO_THREAD *thr, void *arg);

class PeriodicTask {
    public:
        int c;
        int t;
        int a;
        int d;
        bool isAquired;
    public:
        PeriodicTask(int com, int ti, int arrival, int deadline) {
            c =com;
            t= ti;
            a=arrival;
            d=deadline;
        }
};

class AperiodicTask {
    public:
        int c;
        int a;
    public:
        AperiodicTask(int com, int arrival) {
            c =com;
            a=arrival;
        }
};

class ServerCapacityCordinate {
    public:
        float x;
        float y;
    public:
        ServerCapacityCordinate(float xCor, float yCor) {
            x =xCor;
            y=yCor;
        }
};

class DATA{

   public:

      ALLEGRO_MUTEX *mutex;
      ALLEGRO_COND  *cond;
      float          posiX;
      float          posiY;
      bool           modi_X;
      bool           ready;
      vector <PeriodicTask> threads;
      vector <AperiodicTask> aperiodicTask;
      vector <ServerCapacityCordinate> serverCapacityCor;
      float          cs;
      float          ts;
      float          currentCapacity;

   DATA() : mutex(al_create_mutex()),
            cond(al_create_cond()),
            posiX (0),
            posiY (0),
            modi_X(false),
            ready (false) {}

   ~DATA(){

      al_destroy_mutex(mutex);
      al_destroy_cond(cond);

   }

};

class engine {
public :
    void drawLine(int x11,int y11, int x22, int y22, int thickness) {
        al_draw_line(x11, y11, x22, y22, al_map_rgb(0, 0, 0), thickness);
    }

    void drawSererGraphPoint(ServerCapacityCordinate cor){
        al_draw_line(cor.x-2, cor.y-2, cor.x+2, cor.y+2, al_map_rgb(0, 0, 0), 2);
    }

    void drawTimeLine(float y11, float thickness) {
        drawLine(50, y11, TOTAL_WIDTH - 50, y11, thickness);
    }

    void drawAperiodicTaskTimeLine(float y){
        al_draw_line(50, y, TOTAL_WIDTH - 50, y, al_map_rgb(0, 0, 255), 3);
    }

    void drawServerCapacityTimeLine(float y){
        al_draw_line(50, y, TOTAL_WIDTH - 50, y, al_map_rgb(0, 255, 0), 2);
    }

    void drawServerStatusLine(float y){
        al_draw_line(50, y, TOTAL_WIDTH - 50, y, al_map_rgb(132, 111, 60), 2);
    }

    void drawProcessLine(float x) {
        drawLine(x, TOTAL_HEIGHT - 20 , x, 100, 5);
    }

    void drawCurrentTimeLine(float x){
        al_draw_line(x, TOTAL_HEIGHT - 20, x, 100, al_map_rgb(255, 0, 0), 3);
    }

    void drawTimeLabelLine(float x, float y){
        // printf("%.6f %.6f\n", x, y);
        drawLine(x, y-10 , x, y+10, 2);
    }

    void drawServerLabelLine(float y){
        drawLine(50, y , 70, y, 2);
    }
};

int main() {
    ALLEGRO_DISPLAY *display = NULL;
   // if(!al_init()) return -1;
	
    al_init();
    al_init_primitives_addon();

    display = al_create_display(TOTAL_WIDTH, TOTAL_HEIGHT);
    al_clear_to_color(al_map_rgb(255, 255, 255));
    if(!display) return -1;

    al_clear_to_color(al_map_rgb(255, 255, 255));

    ALLEGRO_THREAD      *thread_1    = NULL;
    ALLEGRO_THREAD      *thread_2    = NULL;

    DATA data;
    PeriodicTask threadData1 = PeriodicTask(1, 5, 0, 10);
    PeriodicTask threadData2 = PeriodicTask(2, 10, 0, 20);
    data.threads.push_back(threadData1);
    data.threads.push_back(threadData2);
    data.cs = 5;
    data.ts = 10;
    data.currentCapacity = 5;
    AperiodicTask aperiodicTask1 = AperiodicTask(2, 4);
    AperiodicTask aperiodicTask2 = AperiodicTask(2, 8);
    data.aperiodicTask.push_back(aperiodicTask1);
    data.aperiodicTask.push_back(aperiodicTask2);
    thread_1 = al_create_thread(Func_Thread, &data);
    thread_2 = al_create_thread(Func_Thread, &data);
    al_start_thread(thread_1);
    al_start_thread(thread_2);

    engine e;
     
    for(float i = 0; i < LOOP_TILL; i += 1) {
        al_clear_to_color(al_map_rgb(255, 255, 255));

        // draw time line
        e.drawTimeLine(CONTENT_END_Y + 20, 5);
        e.drawProcessLine(CONTENT_START_X);

        // draw time label
        // 1 block 1 second
        for(int j=0; j<MAX_TIME; j++){
            //float xCor = CONTENT_START_X + j*(totalTimeLineLength/MAX_TIME);
            float xCor = CONTENT_START_X + j*(float)((int)1130/(int)25);
            float yCor = CONTENT_END_Y +20;
            // printf("%.6f %.6f %.6f\n", xCor, yCor, (float)((int)1130/(int)25));
            e.drawTimeLabelLine(xCor, yCor);
        }

        // draw each process line
        for(std::vector<int>::size_type processCount = 0; 
            processCount != data.threads.size(); 
            processCount++) {
                e.drawTimeLine(CONTENT_END_Y + 20 - (TOTAL_HEIGHT/2.5) - (processCount + 1 )*processLineSeparationDis, 3);
        }

        // draw aperopic task line
        e.drawAperiodicTaskTimeLine(CONTENT_END_Y + 20 - (TOTAL_HEIGHT/2.5));

        // draw server status (active/idle) line
        e.drawServerStatusLine(CONTENT_END_Y + 20 - (TOTAL_HEIGHT/4.5));

        // draw aperopic task line
        e.drawServerCapacityTimeLine(CONTENT_END_Y + 20 - 3);

        // draw server capacity graph
        for(std::vector<int>::size_type point = 0; 
            point != data.serverCapacityCor.size(); 
            point++) {
                e.drawSererGraphPoint(data.serverCapacityCor[point]);
        }

        // draw server capacity label
        for(int j=0; j<5; j++){
            e.drawServerLabelLine(CONTENT_END_Y+20 - (j)*serverCapacityLabelDis);
        }

        // move current time line
        e.drawCurrentTimeLine(CONTENT_START_X + i);

        if(i==0){
            // wait before eyes are setup
            al_flip_display();
            al_rest(INITAL_WAIT);
        }
        
        al_flip_display();
        al_rest(WAIT_FACTOR);
    }

    al_destroy_thread(thread_1);
    al_destroy_thread(thread_2);

    al_destroy_display(display);
    return 0;
}


static void *Func_Thread(ALLEGRO_THREAD *thr, void *arg){

    DATA *data  = (DATA*) arg;
    float num   = 0.1;

    al_lock_mutex(data->mutex);

    bool modi_X = data->modi_X;
    data->ready = true;
    al_broadcast_cond(data->cond);

    al_unlock_mutex(data->mutex);

    for(float i = 0; i < LOOP_TILL; i += 1) {
        if(i==0){
            // wait before eyes are setup
            al_rest(INITAL_WAIT);
        }

        al_lock_mutex(data->mutex);
        data->serverCapacityCor.push_back(
            ServerCapacityCordinate(CONTENT_START_X+i, CONTENT_END_Y+20 - data->currentCapacity * serverCapacityLabelDis));
        al_unlock_mutex(data->mutex);

        al_rest(WAIT_FACTOR);
    }



//     while(!al_get_thread_should_stop(thr)){

//       al_lock_mutex(data->mutex);
      
//       al_unlock_mutex(data->mutex);

//       al_rest(0.01);

//    }

   return NULL;
   }
