### Overview 
Arduino based implementation of the classic game PONG.

### PAL Overview
- The PAL TV signal in the game is generated completely by the TVOut Arduino library (https://code.google.com/p/arduino-tvout/). The overview below is purely for the sake of learning.
- PAL (Phase Alternating Line) requires 625 scanlines to create one image on the screen.
- Not all 625 lines can be seen, some are used for sync pulses (and later for teletext etc), ~576 lines are visible.
- The picture is drawn one half at a time (thus the Alternating part in the name), all even lines are drawn first, then the next cycle will display all odd lines and so on. 
- Voltage levels are used to determine shades of between white and black it will render, black = 0.3V and white = 1V. (Color signal is generated differently, not covered here.)
- A voltage ladder is created by using a 470 Ohm and a 1K Ohm resistor to generate the two required voltage levels.
- Each scan line takes 64us (microseconds) to render. 12us sync signals, then 52us for the image signals.
- The vertical frequency is 50Hz, so the screen is updated with 50 half images per second, which is 25 full images per second. 
- See more at http://en.wikipedia.org/wiki/PAL

### Implementation Overview
- The game implementation is based on the code for the Pong game that is part of the Video Game Shield from Wayne and Layne (http://www.wayneandlayne.com/projects/video-game-shield/).
- I made it a one player game with very basic AI for the computer and added 3 skill levels to choose from.
- Other added features include: ability to pause, 'you won' or 'you lost' screens along with self promoting introduction screens.
- The user paddle changes sizes based on the skill level chosen.
- The ball direction when hit is based on where the ball hits the paddle, hitting the ball on the middle of the paddle will send it back in a straight line but hitting the far edges of the paddle will send the ball back at a much sharper angle.
- For the computer AI, the computer paddle moves one pixel closer in the direction of the ball every loop cycle but based on the skill level chosen it has a variable delay in reaction (it doesn't start moving until the ball is x pixels away from it). Another variable between skill levels is the 'loss of concentration' of the computer, basically the computer will not move its paddle every x amount of horizontal pixels that the ball moves across (implemented via the computer_loss_of_concentration_modulo_value variable).
- The TV signal generated is PAL and is purely implemented via TVOut Arduino library (https://code.google.com/p/arduino-tvout/). The resolution used is 120 x 96 pixels.
- The user paddle position is controlled by a analog to digital conversion reading from a 10K Ohm potentiometer.

### Schematic
![Schematic](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/schematic.png)

### Breadboard
![Angled View](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/breadboard_angle.png)

![Top View](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/breadboard_top.png)

![Back View](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/breadboard_back.png)

### TV
![Introduction Screen](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/tv_1.png)

![Introduction Screen Two](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/tv_2.png)

![Game Play](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/tv_3.png)

![Paused Screen](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/tv_4.png)

![Game Over Screen](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/tv_5.png)

### PCB Layout
![Top](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/pcb_front.png)

![Bottom](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/pcb_back.png)

### Final Product
![Top View](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/final_top.png)

![Back View](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/final_back.png)

![Front View](https://raw.githubusercontent.com/darkosancanin/avr_pong/master/images/final_front.png)