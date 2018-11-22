// g++ processscheduling.cpp -lallegro_primitives -lallegro

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
      float          cs;
      float          ts;

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
    void drawLine(int x1,int y1, int x2, int y2, int t) {
        al_draw_line(x1, y1, x2, y2, al_map_rgb(0, 0, 0), t);
    }

    void drawTimeLine(float y1, float thickness) {
        drawLine(50, y1, TOTAL_WIDTH - 50, y1, thickness);
    }

    void drawAperiodicTaskTimeLine(float y1){
        al_draw_line(50, y1, TOTAL_WIDTH - 50, y1, al_map_rgb(0, 0, 255), 3);
    }

    void drawServerCapacityTimeLine(float y1){
        al_draw_line(50, y1, TOTAL_WIDTH - 50, y1, al_map_rgb(0, 255, 0), 2);
    }

    void drawServerStatusLine(float y1){
        al_draw_line(50, y1, TOTAL_WIDTH - 50, y1, al_map_rgb(132, 111, 60), 2);
    }

    void drawProcessLine(float x1) {
        drawLine(x1, TOTAL_HEIGHT - 20 , x1, 100, 5);
    }

    void drawCurrentTimeLine(float x1){
        al_draw_line(x1, TOTAL_HEIGHT - 20, x1, 100, al_map_rgb(255, 0, 0), 3);
    }

    void drawTimeLabelLine(float x1, float y){
        drawLine(x1, y-10 , x1, y+10, 2);
    }

    void drawServerLabelLine(float y1){
        drawLine(50, y1 , 70, y1, 2);
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
    AperiodicTask aperiodicTask1 = AperiodicTask(2, 4);
    AperiodicTask aperiodicTask2 = AperiodicTask(2, 8);
    data.aperiodicTask.push_back(aperiodicTask1);
    data.aperiodicTask.push_back(aperiodicTask2);
    thread_1 = al_create_thread(Func_Thread, &data);
    thread_2 = al_create_thread(Func_Thread, &data);
    al_start_thread(thread_1);
    al_start_thread(thread_2);

    engine e;
    float x1 = 60,y1 = 220, x2 = TOTAL_WIDTH-50, y2 = TOTAL_HEIGHT - 50;
    float separationDis = 100;
    float totalTimeLine = x2-x1;

    // // wait before eyes are setup
    // al_clear_to_color(al_map_rgb(255, 255, 255));
    // al_rest(3);
     
    for(float i = 0; i < LOOP_TILL; i += 1) {
        al_clear_to_color(al_map_rgb(255, 255, 255));

        // draw time line
        e.drawTimeLine(y2 + 20, 5);
        e.drawProcessLine(x1);

        // draw time label
        // 1 block 1 second
        for(int j=0; j<MAX_TIME; j++){
            e.drawTimeLabelLine(x1 + j*(totalTimeLine/MAX_TIME), y2 + 20);
        }

        // draw each process line
        for(std::vector<int>::size_type processCount = 0; 
            processCount != data.threads.size(); 
            processCount++) {
                e.drawTimeLine(y2 + 20 - (TOTAL_HEIGHT/2.5) - (processCount + 1 )*separationDis, 3);
        }

        // draw aperopic task line
        e.drawAperiodicTaskTimeLine(y2 + 20 - (TOTAL_HEIGHT/2.5));

        // draw server status (active/idle) line
        e.drawServerStatusLine(y2 + 20 - (TOTAL_HEIGHT/4.5));

        // draw aperopic task line
        e.drawServerCapacityTimeLine(y2 + 20 - 3);

        // draw server capacity label
        float tempY = y2 + 20;
        for(int j=0; j<5; j++){
            e.drawServerLabelLine(tempY - (j)*30);
        }

        // move current time line
        e.drawCurrentTimeLine(x1 + 60 + i);
        
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



//     while(!al_get_thread_should_stop(thr)){

//       al_lock_mutex(data->mutex);
      
//       al_unlock_mutex(data->mutex);

//       al_rest(0.01);

//    }

   return NULL;
   }
