// g++ processscheduling.cpp -lallegro_primitives -lallegro
#include <stdio.h>
#include "allegro5/allegro.h"
#include <allegro5/allegro_primitives.h>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

#define PI M_PI

#define TOTAL_WIDTH 1240
#define TOTAL_HEIGHT 750
#define TOTAL_TASKS 2

#define TOTAL_TIME 30
#define MAX_TIME 25
#define WAIT_FACTOR 0.025
#define LOOP_TILL TOTAL_TIME/WAIT_FACTOR
#define INITAL_WAIT 1

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

class Replenish {
    public:
        float c;
        float t;
    public:
        Replenish(int com, int tim){
            c=com;
            t=tim;
        }
};

class Task {
    public:
        float c;
        float a;
        float d;
        float completed = 0;
        vector <int> excecuting;
    public:
        Task(int com, int arr, int dead){
            c=com;
            a=arr;
            d=dead;
        }
};

class PeriodicTask {
    public:
        float c;
        float t;
        float a;
        float d;
        bool isAquired;
        int pr;
        vector <Task> tasks;
        string name;
        vector <float> verLines;
        bool wantCPU = false;
    public:
        PeriodicTask() {}
        PeriodicTask(string na, int com, int ti, int arrival, int deadline, int priority) {
            name = na;
            c =com;
            t= ti;
            a=arrival;
            d=deadline;
            pr=priority;
        }
};

class AperiodicTask {
    public:
        float c;
        float a;
        float completed = 0;
        vector <float> excecuting;
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
      bool           serverWantCPU = false;
      vector <float>   sVerLines;

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

    void drawVerLines(float x, float y){
        al_draw_line(x-5, y-35, x, y-40, al_map_rgb(0, 0, 0), 2);
        al_draw_line(x+5, y-35, x, y-40, al_map_rgb(0, 0, 0), 2);
        al_draw_line(x, y, x, y-40, al_map_rgb(0, 0, 0), 2);
    }

    void drawSererGraphPoint(ServerCapacityCordinate cor){
        al_draw_line(cor.x-1, cor.y-1, cor.x+1, cor.y+1, al_map_rgb(0, 0, 0), 1);
    }

    void drawExcetuting(float x, float y, float height) {
        al_draw_line(x-1, y, x+1, y-height, al_map_rgb(0, 0, 0), 1);
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
    PeriodicTask threadData2 = PeriodicTask("Periodic Task 1", 1, 5, 0, 5, 1);
    PeriodicTask threadData1 = PeriodicTask("Periodic Task 2", 4, 15, 0, 15, 3);
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
    float timeLabelDis = (CONTENT_END_X - CONTENT_START_X)/MAX_TIME;
    for(float i = 0; i < LOOP_TILL; i += 1) {
        al_clear_to_color(al_map_rgb(255, 255, 255));

        // draw time line
        e.drawTimeLine(CONTENT_END_Y + 20, 5);
        e.drawProcessLine(CONTENT_START_X);

        // draw time label
        // 1 block 1 second
        for(int j=0; j<MAX_TIME; j++){
            float xCor = CONTENT_START_X + j*timeLabelDis;
            //float xCor = CONTENT_START_X + j*(float)((int)1130/(int)25);
            float yCor = CONTENT_END_Y +20;
            // printf("%.6f %.6f %.6f\n", xCor, yCor, (float)((int)1130/(int)25));
            e.drawTimeLabelLine(xCor, yCor);
        }
        
        int serverStatusLineY = CONTENT_END_Y + 20 - (TOTAL_HEIGHT/4);

        al_lock_mutex(data.mutex);
        // draw each process line
        for(std::vector<int>::size_type processCount = 0; 
            processCount != data.threads.size(); 
            processCount++) {
                float yCor = CONTENT_END_Y + 20 - (TOTAL_HEIGHT/2.5) - (processCount + 1 )*processLineSeparationDis;
                e.drawTimeLine(yCor, 3);
        
            // draw instance arrive ver lines
            for(std::vector<int>::size_type lineCount = 0; 
                lineCount != data.threads[processCount].verLines.size(); 
                lineCount++) {
                    e.drawVerLines(data.threads[processCount].verLines[lineCount], yCor);
            }
            // draw executing
            for(std::vector<int>::size_type taskCount = 0; 
                taskCount != data.threads[processCount].tasks.size(); 
                taskCount++) {
                    for(std::vector<int>::size_type point = 0; 
                        point != data.threads[processCount].tasks[taskCount].excecuting.size(); 
                        point++) {
                            e.drawExcetuting(data.threads[processCount].tasks[taskCount].excecuting[point], yCor, 30);
                            // draw server status if this task priority is higher than server
                            if(data.threads[processCount].pr < data.ps) {
                                e.drawExcetuting(data.threads[processCount].tasks[taskCount].excecuting[point], serverStatusLineY, 15);
                            }
                    }
            }
        }

        // draw aperopic task line
        float yCorServerLine = CONTENT_END_Y + 20 - (TOTAL_HEIGHT/2.5);
        e.drawAperiodicTaskTimeLine(yCorServerLine);
        // draw instance arrive ver lines
        for(std::vector<int>::size_type lineCount = 0; 
            lineCount != data.sVerLines.size(); 
            lineCount++) {
                e.drawVerLines(data.sVerLines[lineCount], yCorServerLine);
        }
        // draw executing
        for(std::vector<int>::size_type tasks = 0; 
            tasks != data.aperiodicTask.size(); 
            tasks++) {
                for(std::vector<int>::size_type lintCount = 0; 
                    lintCount != data.aperiodicTask[tasks].excecuting.size(); 
                    lintCount++) {
                        e.drawExcetuting(data.aperiodicTask[tasks].excecuting[lintCount], yCorServerLine, 30);
                        e.drawExcetuting(data.aperiodicTask[tasks].excecuting[lintCount], serverStatusLineY, 15);
                    }
        }

        // draw server status (active/idle) line
        e.drawServerStatusLine(serverStatusLineY);

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
            // cout << CONTENT_END_Y+20 - (j)*serverCapacityLabelDis << "\n";
            e.drawServerLabelLine(CONTENT_END_Y - 10 - (j)*serverCapacityLabelDis);
        }

        // move current time line
        e.drawCurrentTimeLine(data.currentTime);

        al_unlock_mutex(data.mutex);

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
        
        al_lock_mutex(data->mutex);

        if(data->currentExc == data->ps) {
            data->currentCapacity = data->currentCapacity - WAIT_FACTOR;
        }

        data->serverCapacityCor.push_back(
            ServerCapacityCordinate(CONTENT_START_X+i, CONTENT_END_Y + 20 - (float)(data->currentCapacity) * serverCapacityLabelDis));
        
        al_unlock_mutex(data->mutex);
        // cout << CONTENT_END_Y + 20 - (data->currentCapacity-1) * serverCapacityLabelDis << "\n";
        
        al_rest(WAIT_FACTOR);
    }

   return NULL;
}

static void *SchedularFunc(ALLEGRO_THREAD *thr, void *arg){

    DATA *data  = (DATA*) arg;

    al_rest(INITAL_WAIT);
    // wait before eyes are setup

    while(!al_get_thread_should_stop(thr)){

        al_lock_mutex(data->mutex);
        int newPrio = 99;
        for(std::vector<int>::size_type taskCount = 0; 
                taskCount != data->threads.size(); 
                taskCount++) {
                    // cout << "Schedular: "<< newPrio << " " << data->threads[taskCount].name << " " << data->threads[taskCount].pr << " " << data->threads[taskCount].wantCPU << "\n";
                    if(data->threads[taskCount].pr < newPrio && data->threads[taskCount].wantCPU) {
                        newPrio = data->threads[taskCount].pr;
                    }
        }

        if(data->ps < newPrio && data->serverWantCPU){
            newPrio = data->ps;
        }
        // cout << newPrio << "\n";
        // check if priority changed
        if(newPrio != data->currentExc) {
            // cout << "Giving priority to: " << newPrio << "\n";
            
            data->currentExc = newPrio;
        }
        al_unlock_mutex(data->mutex);
        al_rest(WAIT_FACTOR);
    }

    return NULL;
}

static void *AperiodicTaskFunc(ALLEGRO_THREAD *thr, void *arg){

    DATA *data  = (DATA*) arg;
    vector<Replenish> replenish;

    int doneWithSec = -1;
    for(float i = 0; i < LOOP_TILL; i += 1) {
        if(i==0){
            // wait before eyes are setup
            al_rest(INITAL_WAIT);
        }

        al_lock_mutex(data->mutex);
        int currentTime = i*WAIT_FACTOR;

        if(doneWithSec != currentTime) {
            // check for replemish
            for(std::vector<int>::size_type index = 0; 
                index != replenish.size(); 
                index++) {
                    if(replenish[index].t == currentTime) {
                        data->currentCapacity += replenish[index].c;
                    }
            }

            // check for new instance        
            for(std::vector<int>::size_type taskCount = 0; 
                taskCount != data->aperiodicTask.size(); 
                taskCount++) {

                if(currentTime == (int)data->aperiodicTask[taskCount].a){
                    // push to queue
                    // new instace arrived
                    cout << "Aperiodic Task" << ": new instance arrived c:" << data->aperiodicTask[taskCount].c << ", arrived at:" << currentTime <<  "\n"; //", absolute deadline:" << threadData.d + currentTime << "\n"; 

                
                    data->sVerLines
                        .push_back(CONTENT_START_X + i);

                    // mark want CPU true
                    data->serverWantCPU = true;   
                }
            }
        }
        
        int temp = 0;
        for(std::vector<int>::size_type taskCount = 0; 
            taskCount != data->aperiodicTask.size(); 
            taskCount++) {
                if(currentTime < data->aperiodicTask[taskCount].a){
                    // do not consider future instances
                    continue;
                }
                // cout << data->aperiodicTask[taskCount].completed << " " << data->aperiodicTask[taskCount].a << "\n";
                // if task not completed excecute it
                if(data->aperiodicTask[taskCount].c - data->aperiodicTask[taskCount].completed > 0.05
                    && currentTime >= data->aperiodicTask[taskCount].a){
                        temp++;
                        if(data->currentExc == data->ps) {
                            // if first time excetuting then add to replenish list
                            if(data->aperiodicTask[taskCount].completed == 0){
                                replenish.push_back(Replenish(data->aperiodicTask[taskCount].c, data->ts + currentTime));
                            }
                            // exec task
                            data->aperiodicTask[taskCount].completed +=  WAIT_FACTOR;
                            data->aperiodicTask[taskCount].excecuting.push_back(CONTENT_START_X + i);
                            // cout << "Pushing " << i << "\n";
                        }else {
                            // request for exc if already not requested
                            // cout << "Aperiodic task: Requesting CPU.\n";
                            data->serverWantCPU = true;
                        }
                }
        }

        // do not need CPU
        if(temp == 0 && data->serverWantCPU) {
            // cout << "Aperiodic task: Returing CPU." << "\n";
            data->serverWantCPU = false;
        }
        al_unlock_mutex(data->mutex); 
        // cout << i << "\n";
        al_rest(WAIT_FACTOR);
        doneWithSec = currentTime;
    }

   return NULL;
}


static void *PeriodicTaskFunc(ALLEGRO_THREAD *thr, void *arg){

    DATA *data  = (DATA*) arg;

    // get unaquired thread
    PeriodicTask threadData;
    int taskIndex = -1;
    for(std::vector<int>::size_type processCount = 0; 
            processCount != data->threads.size(); 
            processCount++) {
                if(!data->threads[processCount].isAquired){
                    al_lock_mutex(data->mutex);
                    data->threads[processCount].isAquired = true;
                    al_unlock_mutex(data->mutex);
                    threadData = data->threads[processCount];
                    taskIndex = processCount;
                    break;
                }
    }

    if(taskIndex == -1) {
        printf("No task found for this thread, returning\n");
        return NULL;
    }

    int doneWithSec = -1;
    for(float i = 0; i < LOOP_TILL; i += 1) {
        if(i==0){
            // wait before eyes are setup
            al_rest(INITAL_WAIT);
        }

        al_lock_mutex(data->mutex);

        // al_lock_mutex(data->mutex);
        int currentTime = i*WAIT_FACTOR;

        if(doneWithSec != currentTime) {
            if(currentTime % (int)threadData.t == threadData.a){
                // new instace arrived
                // push to queue
                data->threads[taskIndex].tasks
                    .push_back(Task(threadData.c, currentTime, threadData.d + currentTime));
                data->threads[taskIndex].verLines
                    .push_back(CONTENT_START_X + i);
                data->threads[taskIndex].wantCPU = true;
                cout << threadData.name << ": new instance arrived c:" << threadData.c << ", arrived at:" << currentTime << ", absolute deadline:" << threadData.d + currentTime << "\n"; 

            }  
        }
        
        int temp=0;
        for(std::vector<int>::size_type instanceCount = 0; 
            instanceCount != data->threads[taskIndex].tasks.size(); 
            instanceCount++) {
                if(currentTime < data->threads[taskIndex].tasks[instanceCount].a){
                    // do not consider future instances
                    continue;
                }

                // if task not completed excecute it
                if(data->threads[taskIndex].tasks[instanceCount].c - data->threads[taskIndex].tasks[instanceCount].completed > 0.05
                    && currentTime >= data->threads[taskIndex].tasks[instanceCount].a){
                        temp++;
                        if(data->currentExc == data->threads[taskIndex].pr) {
                            // exec task
                            data->threads[taskIndex].tasks[instanceCount].completed +=  WAIT_FACTOR;
                            data->threads[taskIndex].tasks[instanceCount].excecuting.push_back(CONTENT_START_X + i);
                            // cout << "Pushing " << i << "\n";
                        }else {
                            // request for exc if already not requested
                            // cout << "Aperiodic task: Requesting CPU.\n";
                            data->threads[taskIndex].wantCPU = true;
                        }
                }
        }
        if(temp == 0) {
            // cout << threadData.name << ": Returing CPU." << "\n";
            data->threads[taskIndex].wantCPU = false;
        }
        al_unlock_mutex(data->mutex);
        al_rest(WAIT_FACTOR);
        doneWithSec = currentTime; 
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
