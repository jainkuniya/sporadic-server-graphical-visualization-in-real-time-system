// g++ processscheduling.cpp -lallegro_primitives -lallegro

#include "allegro5/allegro.h"
#include <allegro5/allegro_primitives.h>
#include <cmath>
#include <vector>

#define PI M_PI

#define TOTAL_WIDTH 750
#define TOTAL_HEIGHT 500
#define TOTAL_TASKS 2

using namespace std;

static void *Func_Thread(ALLEGRO_THREAD *thr, void *arg);

class ThreadData {
    public:
        int c;
        int t;
        int a;
        int d;
        bool isAquired;
    public:
        ThreadData(int com, int ti, int arrival, int deadline) {
            c =com;
            t= ti;
            a=arrival;
            d=deadline;
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
      vector <ThreadData> threads;

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

    void drawProcessLine(float x1) {
        drawLine(x1, 330 , x1, 100, 5);
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
    ThreadData threadData1 = ThreadData(1, 5, 0, 10);
    ThreadData threadData2 = ThreadData(2, 10, 0, 20);
    data.threads.push_back(threadData1);
    data.threads.push_back(threadData2);
    thread_1 = al_create_thread(Func_Thread, &data);
    thread_2 = al_create_thread(Func_Thread, &data);
    al_start_thread(thread_1);
    al_start_thread(thread_2);

    engine e;
    float x1 = 0,y1 = 220, x2 = 125, y2 = 300;
    float separationDis = 50;
    for(float i = 0; i + 120 < 700; i += 2) {
        al_clear_to_color(al_map_rgb(255, 255, 255));

        // draw time line
        e.drawTimeLine(y2 + 20, 5);
        e.drawProcessLine(x1 + 60);

        // draw each process line
        for(std::vector<int>::size_type processCount = 0; 
            processCount != data.threads.size(); 
            processCount++) {
                e.drawTimeLine(y2 + 20 - (processCount + 1 )*separationDis, 3);
        }
        
        al_flip_display();
        al_rest(0.025);
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

    while(!al_get_thread_should_stop(thr)){

      al_lock_mutex(data->mutex);
      
      al_unlock_mutex(data->mutex);

      al_rest(0.01);

   }

   return NULL;
   }
