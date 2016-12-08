/** file shapemotion.c
 *  brief This is a simple shape motion demo.
 *  This demo creates two layers containing shapes.
 *  One layer contains a rectangle and the other a circle.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */  
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include <stdio.h>

#define R_LENGTH 2
#define R_WIDTH 10
#define C1RAD 20
#define C2RAD 12

#define GREEN_LED BIT6
void mlCheckCollision();

AbRect rect10 = {abRectGetBounds, abRectCheck, {10,2}}; /**< 10x10 rectangle */
AbRArrow rightArrow = {abRArrowGetBounds, abRArrowCheck, 30};

AbRectOutline fieldOutline = { /* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2 - 10, screenHeight/2 - 10}
};
  

Layer layer3 = { /**< Layer with an orange circle */
  (AbShape *)&circle8,
  {(screenWidth/2)+10, (screenHeight/2)+5}, /**< bit below & right of center */
  {0,0}, {0,0},     /* last & next pos */
  COLOR_VIOLET,
  0,
};


Layer fieldLayer = { /* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},/**< center */
  {0,0}, {0,0},     /* last & next pos */
  COLOR_BLACK,
  &layer3
};

Layer layer1 = { /**< Layer with a red square */
  (AbShape *)&rect10,
  {screenWidth/2, screenHeight-15}, /**< center, bottom */
  {0,0}, {0,0},     /* last & next pos */
  COLOR_RED,
  &fieldLayer,
};

Layer layer0 = { /**< Layer with an orange circle */
  (AbShape *)&circle14,
  {(screenWidth/2)+10, (screenHeight/2)+5}, /**< bit below & right of center */
  {0,0}, {0,0},     /* last & next pos */
  COLOR_ORANGE,
  &layer1,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */
MovLayer ml3 = { &layer3, {1,1}, 0 }; /**< not all layers move */
MovLayer ml1 = { &layer1, {0,0}, &ml3 }; 
MovLayer ml0 = { &layer0, {2,1}, &ml1 }; 







movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8); /**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8); /**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
 bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
 Vec2 pixelPos = {col, row};
 u_int color = bgColor;
 Layer *probeLayer;
 for (probeLayer = layers; probeLayer; 
      probeLayer = probeLayer->next) { /* probe all layers, in order */
   if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
     color = probeLayer->color;
     break; 
   } /* if probe check */
} // for checking all layers at col, row
 lcd_writeColor(color);
      } // for col
    } // for row
    // mlCheckCollision();//check for the collision-user created function
  } // for moving layer being updated
}   

/*void mlCheckCollision(){
  Region bCircle1, bCircle2, bRect;
  layerGetBounds(&layer0, &bCircle1);
  layerGetBounds(&layer1, &bRect);
  layerGetBounds(&layer3, &bCircle2);

  if (bCircle1.botRight.axes[0]>= bRect.topLeft.axes[0] &&
      bCircle1.botRight.axes[0]<= bRect.botRight.axes[0]) {
           drawString5x7(20,20, "circle1 row", COLOR_GREEN, COLOR_RED);
  }//end 1st if
  
  if (bCircle1.topLeft.axes[1]>=bRect.topLeft.axes[1] &&
      bCircle1.topLeft.axes[1]<=bRect.botRight.axes[1]){
         drawString5x7(20,20, "circle1 left column", COLOR_GREEN, COLOR_RED);
  }//end 2nd if
  if (bCircle1.botRight.axes[1]>=bRect.topLeft.axes[1] && bCircle1.botRight.axes[1]<= bRect.botRight.axes[1]){
    drawString5x7(20,20, "circle1 right column", COLOR_GREEN, COLOR_RED);
  }//end 3rd if 
}
*/

//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *  
 *  am ml The moving shape to be advanced
 *  am fence The region which will serve as a boundary for ml
 */

void mlAdvance(MovLayer *ml, Region *fence, MovLayer *rect, MovLayer *circle1, MovLayer *circle2)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  Region rectBoundary;
  Region circleBoundary;
  Vec2 circle1Pos, rectPos;
  u_int circle1Rad;
  int rectRad = (R_WIDTH/2);
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
   (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
 int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
 newPos.axes[axis] += (2*velocity);
      } /**< if outside of fence */
    } /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml */

  // drawString5x7(circle1->layer->pos.axes[0], circle1->layer->pos.axes[1], "center should be here", COLOR_GREEN, COLOR_RED);
  if( (circle1->layer->pos.axes[1] + C1RAD >= rect->layer->pos.axes[1] + R_LENGTH) &&
      (circle1->layer->pos.axes[1] - C1RAD <= rect->layer->pos.axes[1]- R_LENGTH) &&
      (circle1->layer->pos.axes[0] - rectRad - C1RAD <= rect->layer->pos.axes[0] + R_WIDTH)
      
      ){
    // if(circle1Pos.axes[0] + circle1Rad > rectBoundary.topLeft.axes[0]){
              drawString5x7(10,10, "YOU LOSE! ;P", COLOR_GREEN, COLOR_RED);
	      
    //   }//end circle1 statement 
}
  if( (circle2->layer->pos.axes[1] + C2RAD >= rect->layer->pos.axes[1] + R_LENGTH) &&
      (circle2->layer->pos.axes[1] - C2RAD <= rect->layer->pos.axes[1]- R_LENGTH) &&
      (circle2->layer->pos.axes[0] - rectRad - C2RAD <= rect->layer->pos.axes[0] + R_WIDTH)
      
      ){
    // if(circle1Pos.axes[0] + circle1Rad > rectBoundary.topLeft.axes[0]){
              drawString5x7(10,10, "YOU LOSE! ;P", COLOR_GREEN, COLOR_RED);
	      
    //   }//end circle1 statement 
}

}


u_int bgColor = COLOR_BLUE;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence; /**< fence around playing field  */


/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED; /**< Green led on when CPU on */
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(15);

  shapeInit();

  layerInit(&layer0);
  layerDraw(&layer0);

  layerGetBounds(&fieldLayer, &fieldFence);


  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);               /**< GIE (enable interrupts) */


  for(;;) { //game loop
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);       /**< CPU OFF */
    }
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;

    u_int switches = p2sw_read();
     if(switches & BIT0){
               ml1.velocity.axes[0] = 1;      
     }
     if(switches & BIT1){
        ml1.velocity.axes[0] = -1;  
     }
    movLayerDraw(&ml0, &layer0);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;       /**< Green LED on when cpu on */
  count ++;
  if (count == 15) {
    mlAdvance(&ml0, &fieldFence,&ml1,&ml0, &ml3);
    if (p2sw_read())
      redrawScreen = 1;
    count = 0;
  } 
  P1OUT &= ~GREEN_LED;     /**< Green LED off when cpu off */
}


