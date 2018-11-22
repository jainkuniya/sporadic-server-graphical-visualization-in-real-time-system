# Graphical Visualization of Sporadic Server in Real Time System

This project is made to visualize graphically, how periodic and aperiodic tasks are executed in Sporadic Server under fixed priority servers.

Here is the project video: https://www.youtube.com/watch?v=Uk_JrIUiWAw

Read more about Sporadic Server [here](http://bit.ly/2R6L7eN) (page 160).

Follow me on twitter for more updates: https://twitter.com/jainkuniya

## Setup
### Ubuntu (Linux)
#### Setup Allegro 5
```
sudo add-apt-repository ppa:allegro/5.2
sudo apt-get update
sudo apt-get install liballegro5-dev
```

## Run
```
git clone https://github.com/jainkuniya/sporadic-server-graphical-visualization-in-real-time-system

cd sporadic-server-graphical-visualization-in-real-time-system

g++ processscheduling.cpp -lallegro_primitives -lallegro -lpthread -lallegro_font -lallegro_ttf

./a.out
```

[![Looks like this](http://img.youtube.com/vi/Uk_JrIUiWAw/0.jpg)](https://www.youtube.com/watch?v=Uk_JrIUiWAw "Sporadic Server | Simulation | Real Time Systems | Allegro | Thread | Mutex")

PS: This code is written from scratch, complete commit history can be observer on [Github](https://github.com/jainkuniya/sporadic-server-graphical-visualization-in-real-time-system/commits)

PS: Do not copy code :).

## Some keywords
* Sporadic Server
* Simulation
* Real Time Systems
* Allegro
* Thread
* Mutex
