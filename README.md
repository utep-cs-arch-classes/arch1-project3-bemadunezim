Project3
Brian Madunezim

At the top of the file we would include the relevant headers such as the shape definitions and library for printf..
Next we would define our constants such as the radius for the circles and the size of the paddles.
We would then declare the global layer structures, including: ping pong table, the two paddles, and the ping pong ball.
We would initialize the structures with the constants such as the radius and rectangle size previously declared. 
The ping pong ball would have an initial velocity in the y direction.
Next we would declare two score variables as u_ints one for player one and the other for player two.
The next thing we would need to do is create a motion function which changes the position of the layer based on the 
velocity of the layer(layers include ping pong table, the two paddles ,and the ball).
When the ball hits a wall(detected by the x coordinate being less than 0 or greater than the width of the screen)
the x component of the velocity is reverese. If the ball passes the top of the screen(detected by the y coordinate
being less than zero) player 1 recieves a point, then the ball will be re-initiliazed back to the center of the screen
if the ball passes the bottom of the screen(detected by the y coordinates being greater than the screen height)
then player two will receive a point. If either player scores is greater than or equal to 10 that player wins and the
the game is over. The game will then display the score for player1 at the bottom of the screen and player 2 at the top
of the screen. The game over message will be displayed next to the winner's score. If a ball collides with a paddle
as dected using the rectangle's radius method the y velocity component is reversed and the paddles x's velocity 
component is added to the ball's x velocity.
The main function contains device initilaization and the main game loop. The motion function is tied to the timer, the
paddle x velocity component is tied to the game controls.