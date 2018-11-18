#include <stdio.h>
#include <allegro5/allegro.h>

class DATA{

   public:

      ALLEGRO_MUTEX *mutex;
      ALLEGRO_COND  *cond;
      float          posiX;
      float          posiY;
      bool           modi_X;
      bool           ready;

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

const float FPS        = 30;
const int SCREEN_W     = 640;
const int SCREEN_H     = 480;
const int BOUNCER_SIZE = 32;

static void *Func_Thread(ALLEGRO_THREAD *thr, void *arg);

int main(int argc, char **argv){

   ALLEGRO_DISPLAY     *display     = NULL;
   ALLEGRO_EVENT_QUEUE *event_queue = NULL;
   ALLEGRO_TIMER       *timer       = NULL;
   ALLEGRO_BITMAP      *bouncer     = NULL;
   ALLEGRO_THREAD      *thread_1    = NULL;
   ALLEGRO_THREAD      *thread_2    = NULL;

   bool redraw = true;

   if(!al_init()) {
      fprintf(stderr, "failed to initialize allegro!\n");
      return -1;
   }

   if(!al_install_mouse()) {
      fprintf(stderr, "failed to initialize the mouse!\n");
      return -1;
   }

   timer = al_create_timer(1.0 / FPS);
   if(!timer) {
      fprintf(stderr, "failed to create timer!\n");
      return -1;
   }

   display = al_create_display(SCREEN_W, SCREEN_H);
   if(!display) {
      fprintf(stderr, "failed to create display!\n");
      al_destroy_timer(timer);
      return -1;
   }

   bouncer = al_create_bitmap(BOUNCER_SIZE, BOUNCER_SIZE);
   if(!bouncer) {
      fprintf(stderr, "failed to create bouncer bitmap!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }

   al_set_target_bitmap(bouncer);
   al_clear_to_color(al_map_rgb(255, 0, 255));
   al_set_target_bitmap(al_get_backbuffer(display));
   event_queue = al_create_event_queue();

   if(!event_queue) {
      fprintf(stderr, "failed to create event_queue!\n");
      al_destroy_bitmap(bouncer);
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }

   al_register_event_source(event_queue, al_get_display_event_source(display));
   al_register_event_source(event_queue, al_get_timer_event_source(timer));
   al_register_event_source(event_queue, al_get_mouse_event_source());
   al_clear_to_color(al_map_rgb(0,0,0));
   al_flip_display();
   al_start_timer(timer);

   DATA data;

   thread_1 = al_create_thread(Func_Thread, &data);
   al_start_thread(thread_1);

   al_lock_mutex(data.mutex);
   while (!data.ready){

      al_wait_cond(data.cond, data.mutex);

   }
   al_unlock_mutex(data.mutex);

   al_lock_mutex(data.mutex);
   data.modi_X = true;
   data.ready  = false;
   al_unlock_mutex(data.mutex);

   thread_2 = al_create_thread(Func_Thread, &data);
   al_start_thread(thread_2);

   al_lock_mutex(data.mutex);
   while (!data.ready){

      al_wait_cond(data.cond, data.mutex);

   }
   al_unlock_mutex(data.mutex);


   while(1)
   {
      ALLEGRO_EVENT ev;
      al_wait_for_event(event_queue, &ev);

      if(ev.type == ALLEGRO_EVENT_TIMER) {
         redraw = true;
      }
      else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
         break;
      }
      else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
         break;
      }
      if(redraw && al_is_event_queue_empty(event_queue)) {
         redraw = false;

         al_lock_mutex(data.mutex);
         float X = data.posiX;
         float Y = data.posiY;
         al_unlock_mutex(data.mutex);
         
         al_draw_bitmap(bouncer, X, Y, 0);

         al_flip_display();
      }
   }
   al_destroy_thread(thread_1);
   al_destroy_thread(thread_2);

   al_destroy_bitmap(bouncer);
   al_destroy_timer(timer);
   al_destroy_display(display);
   al_destroy_event_queue(event_queue);

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
      if(modi_X)
         data->posiX += num;
      else
         data->posiY += num;
      al_unlock_mutex(data->mutex);

      al_rest(0.01);

   }


   return NULL;
   }