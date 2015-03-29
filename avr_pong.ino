#include <TVout.h>
#include <avr/pgmspace.h>
#include <fontALL.h>
#include "pong_logo.h"
#include "gabriella_and_charlotte.h"

#define BEEP TV.tone(2000, 30)
#define PADDLE_BUFFER 2

#define SKILL_LEVEL_EASY 0
#define SKILL_LEVEL_MEDIUM 1
#define SKILL_LEVEL_HARD 2
#define SKILL_LEVEL_EXPERT 3

TVout TV;
int skill_level;
byte paddle_height; 
byte horizontal_resolution, vertical_resolution; // Stores the horizontal and vertical resolution being displayed
byte ball_x_position, ball_y_position; // Stores the current position of the ball in the respective coordinate plane
char ball_x_direction = 1; // Stores the direction the ball is currently heading in the x coordinate (1 = right, -1 = left)
char ball_y_direction = 1; // Stores the direction the ball is currently heading in the y coordinate (1 = up, -1 = down)
byte score[] = {0, 0}; // Stores the current scores for computer and user respectively
byte leftpaddle_y, rightpaddle_y; // Stores the y position of the paddles
byte ball_is_covering_white_area = 0; // Stores whether the ball is currently over a existing white area such as text or the middle line
byte who_served_last = 0; // Stores who served last, the serving rotates each game
byte min_ball_y_position = 1; // The min position is when it hits the top white line which is one pixel 
byte max_ball_y_position;
byte min_ball_x_position = 2; // The min position is when it hits the left paddle which is two pixels
byte max_ball_x_position;

void setup()
{
    TV.begin(PAL, 120, 96);
    TV.select_font(font4x6);
    TV.delay_frame(60);
    //display_introduction_screens();
    display_choose_skill_level_screen();
    horizontal_resolution = TV.hres() - 2; // Default resolution is not visible on all TV's
    vertical_resolution = TV.vres() - 2;
    max_ball_y_position = vertical_resolution - 1; // The max position is when it hits the bottom white line which is one pixel
    max_ball_x_position = horizontal_resolution - 2; // The max position is when it hits the right paddle which is two pixels
    score[0] = 0;
    score[1] = 0;
    reset_game();
}

void display_introduction_screens()
{
    TV.bitmap(16, 26, pong_logo);
    TV.println(40, 57, "Created by");
    TV.println(4, 67, "Darko, Gabriella & Charlotte");
    TV.delay(5000);
    TV.bitmap(0,0, gabriella_and_charlotte);
    TV.delay(5000);
    TV.clear_screen();
}

void display_choose_skill_level_screen(){
    TV.select_font(font6x8);
    TV.println(6, 10, "Choose Skill Level");
    TV.select_font(font4x6);
    TV.println(50, 35, "Easy");
    TV.println(50, 50, "Medium");
    TV.println(50, 65, "Hard");
    TV.println(50, 80, "Expert");
    unsigned int startMillis=millis();
    while((millis()-startMillis)<=3000) {
      skill_level = map(analogRead(0), 0, 1023, 0, 3);
      TV.draw_rect(43, 35, 2, 50, 0, 0);
      if(skill_level == SKILL_LEVEL_EASY){
        TV.draw_rect(43, 35, 2, 5, 1, 1);
        paddle_height = 9;
      }
      else if(skill_level == SKILL_LEVEL_MEDIUM){
        TV.draw_rect(43, 50, 2, 5, 1, 1);
        paddle_height = 7;
      }
      else if(skill_level == SKILL_LEVEL_HARD){
        TV.draw_rect(43, 65, 2, 5, 1, 1);
        paddle_height = 5;
      }
      else if(skill_level == SKILL_LEVEL_EXPERT){
        TV.draw_rect(43, 80, 2, 5, 1, 1);
        paddle_height = 3;
      }
    }
    TV.clear_screen();
}

void redraw_paddles()
{
    // Clear old paddles by drawing 1 pixel black line down each side of the screen
    TV.draw_rect(0, 1, 1, vertical_resolution - 2, 0, 0);
    TV.draw_rect(horizontal_resolution-2, 1, 1, vertical_resolution - 2, 0, 0);
  
    // Draw the paddles, which are 1 pixel wide (and PADDLE_HEIGHT high) on each side of the screen
    TV.draw_rect(0, leftpaddle_y, 1, paddle_height, 1, 1);
    TV.draw_rect(horizontal_resolution-2, rightpaddle_y, 1, paddle_height, 1, 1);
}

void reset_game()
{
    TV.clear_screen();
    
    TV.draw_line(0, 0, horizontal_resolution, 0, 1); // Draw the top line
    TV.draw_line(0, vertical_resolution, horizontal_resolution, vertical_resolution, 1); // Draw the bottom line
    
    // Draw the dotted vertical middle line
    for (byte y = 1; y < vertical_resolution - 1; y += 6)
        TV.draw_line(horizontal_resolution / 2, y, horizontal_resolution / 2, y + 2, 1);
  
    // Update the scores
    TV.print_char((horizontal_resolution / 4) - 4, 3, '0' + score[0]); // Draw the computer (left side) score
    TV.print_char((horizontal_resolution / 4) + (horizontal_resolution / 2) - 4, 3, '0' + score[1]); // Draw the user (right side) score
  
    byte noise = analogRead(2); // Grab a random value from one of the analog pins
  
    // Randomize the ball position and direction
    ball_y_position = (noise & 0x10) ? ((noise & 0x20) ? vertical_resolution/4 : (vertical_resolution/4 + vertical_resolution/2)) : vertical_resolution / 2;
    ball_y_direction = (noise & 0x02) ? -1 :  1;
    ball_x_position = (who_served_last) ?  min_ball_x_position : max_ball_x_position;
    ball_x_direction = (who_served_last) ?  1 : -1;
    who_served_last = who_served_last ? 0 : 1;
  
    redraw_paddles();
}

void display_you_won_screen(){
  TV.clear_screen();
  TV.select_font(font8x8);
  TV.println(28, 50, "You Won!");
  TV.select_font(font4x6);
  TV.delay_frame(200);
}

void display_game_over_screen(){
  TV.clear_screen();
  TV.select_font(font8x8);
  TV.println(20, 34, "Game Over!");
  TV.select_font(font4x6);
  TV.println(16, 50, "Better luck next time.");
  TV.delay_frame(200);
}

void player_won_a_point(byte player_who_won){ 
  score[player_who_won]++; // Increase the score of the winner
    
    if (score[player_who_won] == 3) // Check if the winner of the point won the game
    {
        TV.delay_frame(50);
        if(player_who_won == 1){
          display_you_won_screen();
        }
        else{
          display_game_over_screen(); 
        }
	score[0] = 0;
        score[1] = 0;
    }
    else
    {
        TV.tone(500, 30);
        TV.delay_frame(50);
    }
	
	reset_game();
}

void updateComputerPaddle(){
	if (ball_x_direction == 1) return; // Don't track the ball when its heading towards the user
	
	if (ball_x_position > (horizontal_resolution - 70)) return; // Set the reaction delay
	
	// Move the computer paddle up or down if the ball is above or below the middle of the paddle
	if (ball_y_position > (leftpaddle_y + (paddle_height / 2)) && leftpaddle_y < (vertical_resolution - paddle_height)){
		leftpaddle_y++;
	}
	else if (ball_y_position < (leftpaddle_y + (paddle_height / 2)) && leftpaddle_y > 0){
		leftpaddle_y--;
	}
}

void change_y_direction_of_ball(byte ball_y_position, byte paddle_y_position){  
  char distance_of_ball_from_center_of_paddle = ball_y_position - (paddle_y_position + ((paddle_height + 1) / 2));
  if(distance_of_ball_from_center_of_paddle == 0) ball_y_direction = 0;
  if(skill_level == SKILL_LEVEL_EASY){
    if(distance_of_ball_from_center_of_paddle < -3) 
      ball_y_direction = -2;
    else if(distance_of_ball_from_center_of_paddle < 0) 
      ball_y_direction = -1;
    else if(distance_of_ball_from_center_of_paddle > 3) 
      ball_y_direction = 2;
    else if(distance_of_ball_from_center_of_paddle > 0) 
      ball_y_direction = 1;
  }
  else if(skill_level == SKILL_LEVEL_MEDIUM || skill_level == SKILL_LEVEL_HARD){
    if(distance_of_ball_from_center_of_paddle < -2) 
      ball_y_direction = -2;
    else if(distance_of_ball_from_center_of_paddle < 0) 
      ball_y_direction = -1;
    else if(distance_of_ball_from_center_of_paddle > 2) 
      ball_y_direction = 2;
    else if(distance_of_ball_from_center_of_paddle > 0) 
      ball_y_direction = 1;
  }
  else if(skill_level == SKILL_LEVEL_EXPERT){
    if(distance_of_ball_from_center_of_paddle < -1) 
      ball_y_direction = -2;
    else if(distance_of_ball_from_center_of_paddle < 0) 
      ball_y_direction = -1;
    else if(distance_of_ball_from_center_of_paddle > 1) 
      ball_y_direction = 2;
    else if(distance_of_ball_from_center_of_paddle > 0) 
      ball_y_direction = 1;
  }
}

void loop()
{    
    if (ball_x_position == min_ball_x_position) // Check if it hit the computer paddle
    {
        if (ball_y_position > leftpaddle_y - PADDLE_BUFFER && ball_y_position < (leftpaddle_y + paddle_height + PADDLE_BUFFER) && ball_x_direction < 0 )
        {
           BEEP;
           change_y_direction_of_ball(ball_y_position, leftpaddle_y);
           ball_x_direction = 1; 
        }
    }
    else if (ball_x_position == max_ball_x_position)  // Check if it hit the user paddle
    {
        if (ball_y_position > rightpaddle_y - PADDLE_BUFFER && ball_y_position < (rightpaddle_y + paddle_height + PADDLE_BUFFER) && ball_x_direction > 0 )
        {
          BEEP;
          change_y_direction_of_ball(ball_y_position, rightpaddle_y);  
          ball_x_direction = -1; 
        }
    }
    
    // Check if ball hit the top of bottom of the screen
    if (ball_y_position >= max_ball_y_position || ball_y_position <= min_ball_y_position)
    {
        BEEP;
        ball_y_direction *= -1;
    }
    
    if (ball_x_position < min_ball_x_position) // Check if it hit the left wall
    {
        player_won_a_point(1);
        return;
    }
    else if (ball_x_position > max_ball_x_position) // Check if it hit the right wall
    {
        player_won_a_point(0);
        return;
    }

    // Read in the user paddle position from the potentiometer
    rightpaddle_y = map(analogRead(0), 0, 1024, 1, vertical_resolution - paddle_height); 
    updateComputerPaddle();

    // Update the ball position
    if(ball_is_covering_white_area == 0){
      TV.set_pixel(ball_x_position, ball_y_position, 0); // Set the previous ball position to be black
    }
    ball_x_position += ball_x_direction;
    ball_y_position += ball_y_direction;
    if(TV.get_pixel(ball_x_position, ball_y_position) == 1){
        ball_is_covering_white_area = 1;
    }
    else{
      ball_is_covering_white_area = 0;  
      TV.set_pixel(ball_x_position, ball_y_position, 1); // Draw the white ball on the screen
    }
    
    redraw_paddles();
      
    TV.delay_frame(1);
}

/*
EASY
- (-5) -2
1 (-4) -2
2 (-3) -1
3 (-2) -1
4 (-1) -1
5 (0) 0
6 (1) 1
7 (2) 1
8 (3) 1 
9 (4) 2
- (5) 2

MEDIUM
- (-4) -2
1 (-3) -2
2 (-2) -1
3 (-1) -1
4 (0) 0
5 (1)  1
6 (2)  1
7 (3)  2
- (4)  2

HARD
- (-3) -2
1 (-2) -1
2 (-1) -1
3 (0)  0
4 (1)  1
5 (2)  1
- (3)  2

EXPERT
- (-2) -2
1 (-1) -1
2 (0)  0
3 (1)  1
- (2)  2
*/
