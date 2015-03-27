#include <TVout.h>
#include <avr/pgmspace.h>
#include <fontALL.h>
#include "pong_logo.h"
#include "gabriella_and_charlotte.h"

#define BEEP TV.tone(2000, 30)
#define PADDLE_HEIGHT 10
#define PADDLE_BUFFER 2

TVout TV;
byte horizontal_resolution, vertical_resolution; // Stores the horizontal and vertical resolution being displayed
byte ball_x_position, ball_y_position; // Stores the current position of the ball in the respective coordinate plane
char ball_x_direction = 1; // Stores the direction the ball is currently heading in the x coordinate (1 = right, -1 = left)
char ball_y_direction = 1; // Stores the direction the ball is currently heading in the y coordinate (1 = up, -1 = down)
byte score[] = {0, 0}; // Stores the current scores for computer and user respectively
byte leftpaddle_y, rightpaddle_y; // Stores the y position of the paddles
byte ball_is_covering_white_area = 0; // Stores whether the ball is currently over a existing white area such as text or the middle line
byte last_player_who_won = 0; // Stores who won the last game, this person is the first receiver in the next game

void setup()
{
    TV.begin(PAL);
    TV.select_font(font4x6);
    TV.delay_frame(60);
    display_introduction_screens();
    horizontal_resolution = TV.hres() - 2; // Default resolution is not visible on all TV's
    vertical_resolution = TV.vres() - 2;
    score[0] = 0;
    score[1] = 0;
    reset_game_display();
}

void display_introduction_screens()
{
    TV.bitmap(16, 26, pong_logo);
    TV.println(44, 57, "Created by");
    TV.println(8, 67, "Darko, Gabriella & Charlotte");
    TV.delay(3000);
    TV.bitmap(0,0, gabriella_and_charlotte);
    TV.delay(3000);
    TV.clear_screen();
}

void redraw_paddles()
{
    // Clear old paddles by drawing 1 pixel black line down each side of the screen
    TV.draw_rect(0, 0, 1, vertical_resolution, 0, 0);
    TV.draw_rect(horizontal_resolution-2, 0, 1, vertical_resolution, 0, 0);
  
    // Draw the paddles, which are 1 pixel wide (and PADDLE_HEIGHT high) on each side of the screen
    TV.draw_rect(0, leftpaddle_y, 1, PADDLE_HEIGHT, 1, 1);
    TV.draw_rect(horizontal_resolution-2, rightpaddle_y, 1, PADDLE_HEIGHT, 1, 1);
}

void reset_game_display()
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
    ball_x_position = (last_player_who_won) ?  1 : horizontal_resolution - 1;
    ball_x_direction = (last_player_who_won) ?  1 : -1;
  
    // Reset the paddle positions to the middle of the screen
    leftpaddle_y = vertical_resolution / 2;
  
    redraw_paddles();
}

void display_you_won_screen(){
  TV.clear_screen();
  TV.select_font(font8x8);
  TV.println(4, 38, "Congratulations");
  TV.println(32, 50, "You Won!");
  TV.select_font(font4x6);
  TV.delay_frame(200);
}

void display_game_over_screen(){
  TV.clear_screen();
  TV.select_font(font8x8);
  TV.println(24, 34, "Game Over!");
  TV.select_font(font4x6);
  TV.println(20, 50, "Better luck next time.");
  TV.delay_frame(200);
}

void player_won_a_point(byte player_who_won){ 
  last_player_who_won =  player_who_won; 
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
	
	reset_game_display();
}

void updateComputerPaddle(){
	if (ball_x_direction == 1) return; // Don't track the ball when its heading towards the user
	
	if (ball_x_position > (horizontal_resolution - 70)) return; // Set the reaction delay
	
	// Move the computer paddle up or down if the ball is above or below the middle of the paddle
	if (ball_y_position > (leftpaddle_y + (PADDLE_HEIGHT / 2)) && leftpaddle_y < (vertical_resolution - PADDLE_HEIGHT)){
		leftpaddle_y++;
	}
	else if (ball_y_position < (leftpaddle_y + (PADDLE_HEIGHT / 2)) && leftpaddle_y > 0){
		leftpaddle_y--;
	}
}

void loop()
{
    // Check if ball hit the top of bottom of the screen
    if (ball_y_position == vertical_resolution - 1 || ball_y_position == 1)
    {
        BEEP;
        ball_y_direction *= -1;
    }
    
    if (ball_x_position <= 2) // Check if it hit the computer paddle
    {
        if (ball_y_position > leftpaddle_y - PADDLE_BUFFER && ball_y_position < (leftpaddle_y + PADDLE_HEIGHT + PADDLE_BUFFER) && ball_x_direction < 0 )
        {
            BEEP;
            ball_x_direction = 1; 
        }
    }
    else if (ball_x_position >= horizontal_resolution - 2)  // Check if it hit the user paddle
    {
        if (ball_y_position > rightpaddle_y - PADDLE_BUFFER && ball_y_position < (rightpaddle_y + PADDLE_HEIGHT + PADDLE_BUFFER) && ball_x_direction > 0 )
        {
            BEEP;
            ball_x_direction = -1; 
        }
    }
    
    if (ball_x_position == 0) // Check if it hit the left wall
    {
        player_won_a_point(1);
        return;
    }
    else if (ball_x_position == horizontal_resolution) // Check if it hit the right wall
    {
        player_won_a_point(0);
        return;
    }

    // Read in the user paddle position from the potentiometer
    rightpaddle_y = map(analogRead(0), 0, 1024, 1, vertical_resolution - PADDLE_HEIGHT); 
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
