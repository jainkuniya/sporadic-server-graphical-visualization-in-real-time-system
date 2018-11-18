// g++ processscheduling.cpp -lallegro_primitives -lallegro

#include "allegro5/allegro.h"
#include <allegro5/allegro_primitives.h>
#include <cmath>

#define PI M_PI

#define TOTAL_WIDTH 750
#define TOTAL_HEIGHT 500

class engine {
public :
    void drawLine(int x1,int y1, int x2, int y2, int t) {
        al_draw_line(x1, y1, x2, y2, al_map_rgb(0, 0, 0), t);
    }

    void drawTimeLine(float y1) {
        drawLine(50, y1, TOTAL_WIDTH - 50, y1, 5);
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

    engine e;
    float x1 = 0,y1 = 220, x2 = 125, y2 = 300;
    for(float i = 0; i + 120 < 700; i += 2) {
        al_clear_to_color(al_map_rgb(255, 255, 255));

        // draw time line
        e.drawTimeLine(y2 + 20);
        e.drawProcessLine(x1 + 60);
        
        al_flip_display();
        al_rest(0.025);
    }
    al_destroy_display(display);
    return 0;
}
