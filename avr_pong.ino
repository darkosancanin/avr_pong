#include <TVout.h>
#include <avr/pgmspace.h>
#include <fontALL.h>
#include "pong_logo.h"
#include "gabriella_and_charlotte.h"

#define BEEP TV.tone(500, 30)
#define PADDLE_BUFFER 2
#define PADDLE_WIDTH 2
#define TOP_AND_BOTTOM_LINE_HEIGHT 1
#define SKILL_LEVEL_EASY 0
#define SKILL_LEVEL_MEDIUM 1
#define SKILL_LEVEL_HARD 2
#define SKILL_LEVEL_EXPERT 3
#define MODE_INTRODUCTION 0
#define MODE_CHOOSE_SKILL_LEVEL 1
#define MODE_PLAY 2
#define MODE_PAUSE 3

// The following variables store general settings
long select_button_last_pressed_time = 0; // Stores when the select button was last pressed
long select_button_debounce_delay_ms = 150; // Stores the debouncing delay for the select button
volatile char mode = MODE_INTRODUCTION; // Stores the current mode, its values are one of the predefined MODE_xxx values
byte horizontal_resolution, vertical_resolution; // Stores the horizontal and vertical resolution being displayed
TVout TV;

// The following variables store the computer skill level settings
int skill_level = SKILL_LEVEL_EASY; // Stores the game skill level, its values are one of the predefined SKILL_LEVEL_xxx values
byte paddle_height; // Stores the height of the paddles, the height gets smaller and smaller based on the skill level
byte computer_reaction_delay; // Stores the reaction time of the computer
byte computer_loss_of_concentration_modulo_value; // The computer ignores the movement of the ball if the ball_x_position mod this value is 0

// The following variables store game play values
byte ball_x_position, ball_y_position; // Stores the current position of the ball in the respective coordinate plane
char ball_x_direction = 1; // Stores the direction the ball is currently heading in the x coordinate (1 = right, -1 = left)
char ball_y_direction = 1; // Stores the direction the ball is currently heading in the y coordinate (1 = up, -1 = down)
byte score[] = {0, 0}; // Stores the current scores for computer and user respectively
byte leftpaddle_y, rightpaddle_y; // Stores the y position of the paddles
byte ball_is_covering_white_area = 0; // Stores whether the ball is currently over a existing white area such as text or the middle line
byte who_served_last = 0; // Stores who served last, the serving rotates each game
byte min_ball_y_position = TOP_AND_BOTTOM_LINE_HEIGHT; // The min position is when it hits the top white line 
byte max_ball_y_position; // The max position is when it hits the bottom white line 
byte min_ball_x_position = PADDLE_WIDTH; // The min position is when it hits the left paddle 
byte max_ball_x_position; // The max position is when it hits the right paddle

void setup()
{
  digitalWrite(2, HIGH); 
  attachInterrupt(0, select_button_pressed, FALLING);
  Serial.begin(9600);
  TV.begin(PAL, 120, 96);
  TV.select_font(font4x6);
  TV.delay_frame(60);
  display_introduction_screens();
  display_choose_skill_level_screen();
  horizontal_resolution = TV.hres() - 2; // Default resolution is not visible on all TV's
  vertical_resolution = TV.vres() - 2;
  max_ball_y_position = vertical_resolution - TOP_AND_BOTTOM_LINE_HEIGHT; // The max position is when it hits the bottom white line 
  max_ball_x_position = horizontal_resolution - PADDLE_WIDTH; // The max position is when it hits the right paddle which is two pixels
  reset_scores();
  reset_game();
}

void display_introduction_screens()
{
  mode = MODE_INTRODUCTION;
  TV.bitmap(16, 26, pong_logo);
  TV.println(40, 57, "Created by");
  TV.println(4, 67, "Darko, Gabriella & Charlotte");
  // Display for n seconds or until the select button is pressed which changes the mode 
  unsigned int startMillis=millis();
  while(((millis() - startMillis) <= 3000) && mode == MODE_INTRODUCTION) {}
  TV.bitmap(0,0, gabriella_and_charlotte);
  while(((millis() - startMillis) <= 3000) && mode == MODE_INTRODUCTION) {}
  TV.clear_screen();
}

void display_choose_skill_level_screen(){
  mode = MODE_CHOOSE_SKILL_LEVEL;
  TV.select_font(font6x8);
  TV.println(6, 10, "Choose Skill Level");
  TV.select_font(font4x6);
  TV.println(50, 35, "Easy");
  TV.println(50, 50, "Medium");
  TV.println(50, 65, "Hard");
  TV.println(50, 80, "Expert");
  // Display for n seconds or until the select button is pressed which changes the mode 
  unsigned int startMillis=millis();
  while(((millis() - startMillis) <= 2000) && mode == MODE_CHOOSE_SKILL_LEVEL) 
  {
    skill_level = map(analogRead(0), 0, 1023, 0, 3); // Read the value that the user is selecting (0-3)
    TV.draw_rect(43, 35, 2, 50, 0, 0); // Clear all possible selections
	// Draw the selection rectangle for the skill level they chose and set the computer skill variable settings
    if(skill_level == SKILL_LEVEL_EASY){
      TV.draw_rect(43, 35, 2, 5, 1, 1);
      paddle_height = 9;
      computer_reaction_delay = 70;
      computer_loss_of_concentration_modulo_value = 10;
    }
    else if(skill_level == SKILL_LEVEL_MEDIUM)
	{
      TV.draw_rect(43, 50, 2, 5, 1, 1);
      paddle_height = 7;
      computer_reaction_delay = 60;
      computer_loss_of_concentration_modulo_value = 11;
    }
    else if(skill_level == SKILL_LEVEL_HARD){
      TV.draw_rect(43, 65, 2, 5, 1, 1);
      paddle_height = 5;
      computer_reaction_delay = 50;
      computer_loss_of_concentration_modulo_value = 12;
    }
    else if(skill_level == SKILL_LEVEL_EXPERT){
      TV.draw_rect(43, 80, 2, 5, 1, 1);
      paddle_height = 3;
      computer_reaction_delay = 40;
      computer_loss_of_concentration_modulo_value = 13;
    }
  }
  TV.clear_screen();
}

void reset_scores()
{
  score[0] = 0;
  score[1] = 0;	
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

void redraw_vertical_middle_line(){
  // Draw the dotted vertical middle line
  for (byte y = 1; y < vertical_resolution - 1; y += 6)
    TV.draw_line(horizontal_resolution / 2, y, horizontal_resolution / 2, y + 2, 1);
}

void reset_game()
{
  TV.clear_screen();
  
  TV.draw_line(0, 0, horizontal_resolution, 0, 1); // Draw the top line
  TV.draw_line(0, vertical_resolution, horizontal_resolution, vertical_resolution, 1); // Draw the bottom line
  
  redraw_vertical_middle_line();
  
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
  TV.println(28, 30, "You Won!");
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
  
  TV.printPGM((player_who_won) ? (8) : (horizontal_resolution - 36), (vertical_resolution / 2) - 4, PSTR("Winner!"));
  TV.printPGM((player_who_won) ? (horizontal_resolution - 36) : (8), (vertical_resolution / 2) - 4, PSTR("Missed!"));
  BEEP;
  TV.delay_frame(50);
	
  if (score[player_who_won] == 3) // Check if the winner of the point won the game
  {
    TV.delay_frame(50);
    if(player_who_won == 1){
      display_you_won_screen();
    }
    else
	{
      display_game_over_screen(); 
    }
    reset_scores();
  }

  reset_game();
}

void updateComputerPaddle(){
  if (ball_x_direction == 1) return; // Don't track the ball when its heading towards the user
  if (ball_x_position > (horizontal_resolution - computer_reaction_delay)) return; // Don't track the ball if its within its reaction delay
  if (ball_x_position % computer_loss_of_concentration_modulo_value == 0) return; // Ignore the ball movement every so many movements
    
  // Move the computer paddle up or down if the ball is above or below the middle of the paddle
  if (ball_y_position > (leftpaddle_y + (paddle_height / 2)) && leftpaddle_y < (vertical_resolution - paddle_height))
  {
    leftpaddle_y++;
  }
  else if (ball_y_position < (leftpaddle_y + (paddle_height / 2)) && leftpaddle_y > 0)
  {
    leftpaddle_y--;
  }
}

// This method changes the direction of the ball based on where the ball is hit on the paddle.
// If it hits directly in the center then the angle is straight back (0), if its right at the edges
// then the angle is -2 or 2 and the other angle is -1 or 1.
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

void redraw_ball(){
  if(ball_is_covering_white_area == 0)
  {
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
}

void display_pause_screen(){
  TV.draw_rect(43, 40, 34, 16, 0, 0);
  TV.printPGM(48, 45, PSTR("Paused"));
  // Display until the select button is pressed which changes the mode back to MODE_PLAY
  while(mode == MODE_PAUSE) {}
  TV.draw_rect(43, 40, 34, 16, 0, 0);
  redraw_vertical_middle_line();
  redraw_ball();
  TV.delay_frame(1);
}

void loop()
{    
  if(mode == MODE_PAUSE)
  {
    display_pause_screen();
    return;
  }
    
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
    
  // Check if ball hit the top or bottom of the screen
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
  
  redraw_ball();
  redraw_paddles();
      
  TV.delay_frame(1);
}

// Interrupt handler for the select button 
void select_button_pressed(){
  // Ignore the button press if its with the debounce delay time from its last press
  if ((millis() - select_button_last_pressed_time) > select_button_debounce_delay_ms) {
    if(mode == MODE_INTRODUCTION)
    {
      mode = MODE_CHOOSE_SKILL_LEVEL;
    }
    else if(mode == MODE_CHOOSE_SKILL_LEVEL)
    {
      mode = MODE_PLAY;
    }
    else if(mode == MODE_PLAY)
    {
      mode = MODE_PAUSE;
    }
    else if(mode == MODE_PAUSE)
    {
      mode = MODE_PLAY;
    }
    select_button_last_pressed_time = millis();
  }
}
