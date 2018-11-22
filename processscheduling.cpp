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

static void *PeriodicTaskFunc(ALLEGRO_THREAD *thr, void *arg);
static void *CurrentTimeFunc(ALLEGRO_THREAD *thr, void *arg);
static void *ServerCapacityFunc(ALLEGRO_THREAD *thr, void *arg);
static void *AperiodicTaskFunc(ALLEGRO_THREAD *thr, void *arg);
static void *SchedularFunc(ALLEGRO_THREAD *thr, void *arg);

class PeriodicTask {
    public:
        int c;
        int t;
        int a;
        int d;
        bool isAquired;
        int pr;
    public:
        PeriodicTask() {}
        PeriodicTask(int com, int ti, int arrival, int deadline, int priority) {
            c =com;
            t= ti;
            a=arrival;
            d=deadline;
            pr=priority;
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
      int            ps; // priority of server
      float          currentCapacity;
      float          currentTime = CONTENT_START_X;
      int            currentExc;

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

    ALLEGRO_THREAD      *thread_1    = NULL; // thread for periodic task 1
    ALLEGRO_THREAD      *thread_2    = NULL; // thread for periodic task 2
    ALLEGRO_THREAD      *thread_3    = NULL; // thread to show current time, red line
    ALLEGRO_THREAD      *thread_4    = NULL; // thread to cal server capacity, graph
    ALLEGRO_THREAD      *thread_5    = NULL; // thread for apperiodic tasks
    ALLEGRO_THREAD      *therad_6    = NULL; // thread to schedule

    DATA data;
    PeriodicTask threadData1 = PeriodicTask(1, 5, 0, 10, 1);
    PeriodicTask threadData2 = PeriodicTask(2, 10, 0, 20, 3);
    data.threads.push_back(threadData1);
    data.threads.push_back(threadData2);
    data.cs = 5;
    data.ts = 10;
    data.ps = 2;
    data.currentCapacity = 5;
    data.currentExc = 0;
    AperiodicTask aperiodicTask1 = AperiodicTask(2, 4);
    AperiodicTask aperiodicTask2 = AperiodicTask(2, 8);
    data.aperiodicTask.push_back(aperiodicTask1);
    data.aperiodicTask.push_back(aperiodicTask2);
    thread_1 = al_create_thread(PeriodicTaskFunc, &data);
    thread_2 = al_create_thread(PeriodicTaskFunc, &data);
    thread_3 = al_create_thread(CurrentTimeFunc, &data);
    thread_4 = al_create_thread(ServerCapacityFunc, &data);
    thread_5 = al_create_thread(AperiodicTaskFunc, &data);
    therad_6 = al_create_thread(SchedularFunc, &data);
    al_start_thread(thread_1);
    al_start_thread(thread_2);
    al_start_thread(thread_3);
    al_start_thread(thread_4);
    al_start_thread(thread_5);
    al_start_thread(therad_6);

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
        e.drawCurrentTimeLine(data.currentTime);

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
    al_destroy_thread(thread_3);
    al_destroy_thread(thread_4);
    al_destroy_thread(thread_5);
    al_destroy_thread(therad_6);

    al_destroy_display(display);
    return 0;
}

static void *ServerCapacityFunc(ALLEGRO_THREAD *thr, void *arg){

    DATA *data  = (DATA*) arg;

    for(float i = 0; i < LOOP_TILL; i += 1) {
        if(i==0){
            // wait before eyes are setup
            al_rest(INITAL_WAIT);
        }

        if(data->currentExc == data->ps) {
            al_lock_mutex(data->mutex);
            data->currentCapacity = data->currentCapacity - WAIT_FACTOR;
            al_unlock_mutex(data->mutex);
        }

        al_lock_mutex(data->mutex);
        data->serverCapacityCor.push_back(
            ServerCapacityCordinate(CONTENT_START_X+i, CONTENT_END_Y + 20 - (data->currentCapacity -1) * serverCapacityLabelDis));
        al_unlock_mutex(data->mutex);

        al_rest(WAIT_FACTOR);
    }

   return NULL;
}

static void *SchedularFunc(ALLEGRO_THREAD *thr, void *arg){

    DATA *data  = (DATA*) arg;

    return NULL;
}

static void *AperiodicTaskFunc(ALLEGRO_THREAD *thr, void *arg){

    DATA *data  = (DATA*) arg;

    // get unaquired thread
    // PeriodicTask threadData;
    // bool foundTask = false;
    // for(std::vector<int>::size_type processCount = 0; 
    //         processCount != data->threads.size(); 
    //         processCount++) {
    //             if(!data->threads[processCount].isAquired){
    //                 al_lock_mutex(data->mutex);
    //                 data->threads[processCount].isAquired = true;
    //                 al_unlock_mutex(data->mutex);
    //                 threadData = data->threads[processCount];
    //                 foundTask = true;
    //                 break;
    //             }
    // }

    // if(!foundTask) {
    //     printf("No task found for this thread, returning\n");
    //     return NULL;
    // }

    // for(float i = 0; i < LOOP_TILL; i += 1) {
    //     if(i==0){
    //         // wait before eyes are setup
    //         al_rest(INITAL_WAIT);
    //     }

    //     // al_lock_mutex(data->mutex);
        
    //     // al_unlock_mutex(data->mutex);

    //     al_rest(WAIT_FACTOR);
    // }

   return NULL;
}


static void *PeriodicTaskFunc(ALLEGRO_THREAD *thr, void *arg){

    DATA *data  = (DATA*) arg;

    // get unaquired thread
    PeriodicTask threadData;
    bool foundTask = false;
    for(std::vector<int>::size_type processCount = 0; 
            processCount != data->threads.size(); 
            processCount++) {
                if(!data->threads[processCount].isAquired){
                    al_lock_mutex(data->mutex);
                    data->threads[processCount].isAquired = true;
                    al_unlock_mutex(data->mutex);
                    threadData = data->threads[processCount];
                    foundTask = true;
                    break;
                }
    }

    if(!foundTask) {
        printf("No task found for this thread, returning\n");
        return NULL;
    }

    for(float i = 0; i < LOOP_TILL; i += 1) {
        if(i==0){
            // wait before eyes are setup
            al_rest(INITAL_WAIT);
        }

        // al_lock_mutex(data->mutex);
        
        // al_unlock_mutex(data->mutex);

        al_rest(WAIT_FACTOR);
    }

   return NULL;
}

static void *CurrentTimeFunc(ALLEGRO_THREAD *thr, void *arg){

    DATA *data  = (DATA*) arg;

    for(float i = 0; i < LOOP_TILL; i += 1) {
        if(i==0){
            // wait before eyes are setup
            al_rest(INITAL_WAIT);
        }

        al_lock_mutex(data->mutex);
        data->currentTime = CONTENT_START_X + i;
        al_unlock_mutex(data->mutex);

        al_rest(WAIT_FACTOR);
    }

   return NULL;
}
