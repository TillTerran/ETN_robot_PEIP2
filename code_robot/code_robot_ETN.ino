//void analogWrite(int pin, int value)

int state;

unsigned long time;
long delta_time;
const unsigned long TIME_THRESHOLD = 300.0;
const unsigned long TIME_0_TO_255 = 1;

const bool LEFT=true;
const bool RIGHT=false;

const int PIN_LEFT_LIGHT_STRONGER = 13;

const int PIN_DIST_LEFT_INF_21 = 4;
const int PIN_DIST_LEFT_INF_42 = 5;
const int PIN_DIST_LEFT_INF_70 = 6;

const int PIN_DIST_RIGHT_INF_21 = 7;
const int PIN_DIST_RIGHT_INF_42 = 8;
const int PIN_DIST_RIGHT_INF_70 = 9;

const int PIN_LEFT_DIST_LOWER = 12;

const int PIN_LEFT_WHEEL_POWER = 10;
const int PIN_RIGHT_WHEEL_POWER = 11;
const int PIN_LEFT_WHEEL_DIR = 2;
const int PIN_RIGHT_WHEEL_DIR = 3;

//____________________________________________________________________


bool left_light_stronger;

bool dist_left_inf_21;
bool dist_left_inf_42;
bool dist_left_inf_70;

bool dist_right_inf_21;
bool dist_right_inf_42;
bool dist_right_inf_70;

bool left_dist_lower;

int left_wheel_power;
int right_wheel_power;

bool left_wheel_dir;
bool right_wheel_dir;



int c_speed_l_engine = 0;  // current avg tension in per 255 for the left engine
int c_speed_r_engine = 0;  // current avg tension in per 255 for the right engine


int desired_speed = 255;  //-255 to 255

bool not_aligned = true;
bool obstacle_on_side = true;
int nb_obstacles_on_side = 0;

int turn_left;
int prev_turn_left;



enum {ENTER_MAZE,FIRST_TURN,FIRST_TURN_ADJUSTMENTS,FIRST_CORRIDOR,SECOND_TURN,INTER_CORRIDOR,THIRD_TURN,LAST_CORRIDOR,LAST_TURN,EXIT_MAZE,
TEST_ENTER_MAZE,TEST_FIRST_TURN,TEST_FIRST_TURN_ADJUSTMENTS,TEST_FIRST_CORRIDOR,TEST_SECOND_TURN,TEST_INTER_CORRIDOR,TEST_THIRD_TURN,TEST_LAST_CORRIDOR,TEST_LAST_TURN,TEST_EXIT_MAZE,
ETN3_ENTER_MAZE,ETN3_FIRST_TURN,ETN3_FIRST_TURN_ADJUSTMENTS,ETN3_FIRST_CORRIDOR,ETN3_SECOND_TURN,ETN3_INTER_CORRIDOR,ETN3_THIRD_TURN,ETN3_LAST_CORRIDOR,ETN3_LAST_TURN,ETN3_EXIT_MAZE,
ETN3_ENTER_MAZE_NO_LIGHT};



int sign_of_int(int x) {
  return int(int(x > 0) - int(x < 0));
}


//timer_condition is not used
bool timer_condition() {
  return time - millis() > TIME_THRESHOLD;
}


int accelerate_left_wheel(int desired_left_wheel_power)//, unsigned long delta) 
{
  /*
  change the current tension a bit towards the desired tension in the left wheel
  */
  c_speed_l_engine = c_speed_l_engine + 15 * sign_of_int(int(desired_left_wheel_power - c_speed_l_engine));  //abs(delta*10/TIME_0_TO_255);
  //c_speed_l_engine=sign_of_int(c_speed_l_engine)*min(abs(c_speed_l_engine),abs(desired_left_wheel_power));
  if (sign_of_int(desired_left_wheel_power) * c_speed_l_engine > abs(desired_left_wheel_power)) {
    c_speed_l_engine = desired_left_wheel_power;
  }
  return c_speed_l_engine;
}


int accelerate_right_wheel(int desired_right_wheel_power)//, unsigned long delta) 
{
  /*
  change the current tension a bit towards the desired tension in the right wheel
  */

  c_speed_r_engine = c_speed_r_engine + 3 * sign_of_int(int(desired_right_wheel_power - c_speed_r_engine));  //abs(delta*100/TIME_0_TO_255);
  //c_speed_r_engine=sign_of_int(c_speed_r_engine)*min(abs(c_speed_r_engine),abs(desired_right_wheel_power));
  if (sign_of_int(desired_right_wheel_power) * c_speed_r_engine > abs(desired_right_wheel_power)) {
    c_speed_r_engine = desired_right_wheel_power;
  }
  return c_speed_r_engine;
}

void update_engine_tension(int turn, int speed) {
  /*
  determine what's the tension desired for each engine
  then change the tension a bit towards that value

  turn must be between -255 and 255
  speed must be between 0 and 255

  (turn < 0) => turn to the right
  else turn to the left
  */
  //delta_time += millis();


  // determine what's the tension desired for each engine
  left_wheel_power = -turn + speed;
  right_wheel_power = turn + speed;

  if (left_wheel_power >= 256) {
    right_wheel_power += 255 - left_wheel_power;
    left_wheel_power = 255;
    right_wheel_power=0;
    left_wheel_power=0;
  } else if (right_wheel_power >= 256) {
    left_wheel_power += 255 - right_wheel_power;
    right_wheel_power = 255;
    right_wheel_power=0;
    left_wheel_power=0;
  
  }

  //accelerate towards the desired speed
  left_wheel_power = int(accelerate_left_wheel(left_wheel_power));//, delta_time));
  right_wheel_power = int(accelerate_right_wheel(right_wheel_power));//, delta_time));


  left_wheel_dir = left_wheel_power <= 0;
  right_wheel_dir = right_wheel_power <= 0;
  left_wheel_power = abs(left_wheel_power);
  right_wheel_power = abs(right_wheel_power);


  //delta_time = -millis();
}

void turn_side(bool left)
{
  if (left)
  {
    turn_left = 127;
    if (dist_left_inf_70) {
      turn_left = 30;  //turn less sharp left
    }
    if (dist_right_inf_21) {
      desired_speed = 120;  //turn sharp left
    }
  }else{
    turn_left = -127;
    if (dist_right_inf_70) {
      turn_left = -30;  //turn less sharp right
    }
    if (dist_left_inf_21) {
      turn_left = -187;  //turn sharp right
    }
  }
}

void handle_first_alignement(bool has_to_go_left) {
  /*
  if (not_aligned && sign_of_int(_turn_left) == sign_of_int(turn_left)) {
    turn_left *= 4;  
  } 
  else 
  {
    not_aligned = false;
  }
  */
    
  
  if (millis() - time < 10000)  // ajust value for the right distance
  {
    
    if (has_to_go_left){
      turn_left*=2*(turn_left>0)+2;
    }
    else
    {
      turn_left*=2*(turn_left<0)+2;
    }
  }
  
}

void adjust_desired_speed() {
  desired_speed = 255 - abs(turn_left);
}





void setup() {
  pinMode(PIN_LEFT_LIGHT_STRONGER, INPUT);  // light_on_right<light_on_left

  //pinMode(PIN_LEFT_WHEEL_CRENEAU,INPUT);// pin /no pin left wheel
  //pinMode(PIN_RIGHT_WHEEL_CRENEAU,INPUT);// pin /no pin right wheel


  // telemeters' signals
  pinMode(PIN_DIST_LEFT_INF_21, INPUT);  //dist_left<21cm
  pinMode(PIN_DIST_LEFT_INF_42, INPUT);  //dist_left<42cm
  pinMode(PIN_DIST_LEFT_INF_70, INPUT);  //dist_left<70cm

  pinMode(PIN_DIST_RIGHT_INF_21, INPUT);  //dist_right<21cm
  pinMode(PIN_DIST_RIGHT_INF_42, INPUT);  //dist_right<42cm
  pinMode(PIN_DIST_RIGHT_INF_70, INPUT);  //dist_right<70cm

  pinMode(PIN_LEFT_DIST_LOWER, INPUT);  //dist_left<dist_right

  //wheel control (outputs)
  pinMode(PIN_LEFT_WHEEL_POWER, OUTPUT);   //left wheel
  pinMode(PIN_RIGHT_WHEEL_POWER, OUTPUT);  //right wheel
  pinMode(PIN_LEFT_WHEEL_DIR, OUTPUT);
  pinMode(PIN_RIGHT_WHEEL_DIR, OUTPUT);


  delay(300);  // it's there because I was looking for a way to correct a weird behaviour


  //init for the code
  time = millis();

  state = 20;
}

void loop() {
  left_light_stronger = digitalRead(PIN_LEFT_LIGHT_STRONGER);  // condition_1

  dist_left_inf_21 = digitalRead(PIN_DIST_LEFT_INF_21);
  dist_left_inf_42 = digitalRead(PIN_DIST_LEFT_INF_42);
  dist_left_inf_70 = digitalRead(PIN_DIST_LEFT_INF_70);

  dist_right_inf_21 = digitalRead(PIN_DIST_RIGHT_INF_21);
  dist_right_inf_42 = digitalRead(PIN_DIST_RIGHT_INF_42);
  dist_right_inf_70 = digitalRead(PIN_DIST_RIGHT_INF_70);

  left_dist_lower = digitalRead(PIN_LEFT_DIST_LOWER);



  desired_speed = 255;


  switch (state) {



    case ENTER_MAZE:

      if (millis() - time > 250) {
        if (not_aligned && ((left_light_stronger * 2 - 1) * 125 == turn_left)) {
          turn_left = (left_light_stronger * 2 - 1) * 125;  //if >0 : light is more on the right else : more on the left
          desired_speed = 0;
        } else {
          turn_left = (left_light_stronger * 2 - 1) * 30;
          adjust_desired_speed();
          not_aligned = false;
        }
      } else {
        not_aligned = true;
        turn_left = (left_light_stronger * 2 - 1) * 125;
        prev_turn_left = sign_of_int(turn_left);
        desired_speed = 0;
      }




      if (dist_right_inf_42) {
        state += 1;
        time = millis();
      }
      break;
    case TEST_ENTER_MAZE:

      if (millis() - time > 1500) {
        if (not_aligned && ((left_light_stronger * 2 - 1) * 255 == turn_left)) {
          turn_left = (left_light_stronger * 2 - 1) * 255;  //if >0 : light is more on the right else : more on the left
        } else {
          turn_left = (left_light_stronger * 2 - 1) * 25;
          not_aligned = false;
          adjust_desired_speed();
        }
      } else {
        not_aligned = true;
        turn_left = (left_light_stronger * 2 - 1) * 255;
      }




      if (dist_left_inf_42 && dist_right_inf_42) {
        turn_left = 0;
        desired_speed = 0;
      }
      break;
    case ETN3_ENTER_MAZE:
      if (millis() - time > 250) {
        if (not_aligned && ((left_light_stronger * 2 - 1) * 125 == turn_left)) {
          turn_left = (left_light_stronger * 2 - 1) * 125;  //if >0 : light is more on the right else : more on the left
          desired_speed = 0;
        } else {
          turn_left = (left_light_stronger * 2 - 1) * 30;
          desired_speed = 225;
          not_aligned = false;
        }
      } else {
        not_aligned = true;
        turn_left = (left_light_stronger * 2 - 1) * 125;
        desired_speed = 0;
      }




      if (dist_right_inf_42) {
        state += 1;
        time = millis();
        delta_time=millis();
        not_aligned = true;
        nb_obstacles_on_side = 0;
      }
      break;

    case ETN3_ENTER_MAZE_NO_LIGHT:
      turn_left = 0;
      if (dist_right_inf_42) {
        state = 21;
        nb_obstacles_on_side = 0;
        obstacle_on_side=true;
        time = millis();
        not_aligned = true;
      }
      break;

    default:

      turn_left = (left_dist_lower * 2 - 1) * 25;
      break;
  }
  switch (state) {
    case FIRST_TURN:
      turn_side(RIGHT);

      if (nb_obstacles_on_side == 0)
      {
        if (not(obstacle_on_side) && (millis() - delta_time > 250)) {
          nb_obstacles_on_side += dist_left_inf_42;
          delta_time=millis();
        }
        if (obstacle_on_side) {
          delta_time = millis();
        }
        obstacle_on_side = dist_left_inf_42;
      }


      if (nb_obstacles_on_side >= 1 || millis()-time>6000)  // && dist_right_inf_42)
      {
        nb_obstacles_on_side = 0;
        state += 2;
        //desired_speed=200;
        time = millis();
      }
      break;

    case FIRST_TURN_ADJUSTMENTS:
      turn_left = -180;

      if (dist_right_inf_21) {
        turn_left = -turn_left;  //turn less sharp right
                                 // allumer une led bleue pour les tests
      } else if (dist_left_inf_21) {
        //turn sharp right
      }

      if (millis() - time > 6000)  // ajust value for the right distance
      {
        state += 1;
        time = millis();
      }

      break;
    case FIRST_CORRIDOR:
      handle_first_alignement(RIGHT);

      if (not(dist_left_inf_70)) {
        state += 1;
        time = millis();
        not_aligned = true;
        nb_obstacles_on_side=0;
      }

      break;
    case SECOND_TURN:
      turn_side(LEFT);

      if (not(obstacle_on_side) && millis() - time > 2000) {
        nb_obstacles_on_side += dist_right_inf_42;
      }
      if (obstacle_on_side) {
        time = millis();
      }
      obstacle_on_side = dist_right_inf_42;


      if (nb_obstacles_on_side >= 1)  // && dist_right_inf_42)
      {
        nb_obstacles_on_side = 0;
        state += 2;
        time = millis();
      }


      break;
    case THIRD_TURN:
      turn_side(LEFT)

      if (not(obstacle_on_side) && millis() - time > 2000) {
        nb_obstacles_on_side += dist_right_inf_42;
      }
      if (obstacle_on_side) {
        time = millis();
      }
      obstacle_on_side = dist_right_inf_42;
      if (nb_obstacles_on_side >= 1) {
        state += 1;
        time = millis();
      }
      break;
    case LAST_CORRIDOR:
      handle_first_alignement(LEFT);
      if (not(dist_right_inf_70)) {
        state += 1;
        time = millis();
        not_aligned = true;
        nb_obstacles_on_side=0;
      }
      break;
    case LAST_TURN:
      turn_side(RIGHT)

      if (not(obstacle_on_side) && millis() - time > 2000) {
        nb_obstacles_on_side += dist_left_inf_42;
      }
      if (obstacle_on_side) {
        time = millis();
      }
      obstacle_on_side = dist_left_inf_42;



      if (nb_obstacles_on_side >= 1)  // && dist_right_inf_42)
      {
        nb_obstacles_on_side = 0;
        state += 1;
        //desired_speed=200;
        time = millis();
      }

      break;
    case EXIT_MAZE:
      turn_left*=2;

      if (not(dist_right_inf_70) && not(dist_left_inf_70)) {
        turn_left = 0;
        desired_speed = 255;
      }
      break;





    // for complex maze (ETN3)
    case ETN3_FIRST_TURN:
      
      turn_side(RIGHT);

      if (nb_obstacles_on_side == 0)
      {
        if (not(obstacle_on_side) && (millis() - delta_time > 1000)) {
          nb_obstacles_on_side += dist_left_inf_42;
          delta_time=millis();
        }
        if (obstacle_on_side) {
          delta_time = millis();
        }
        obstacle_on_side = dist_left_inf_42;
      }


      if (nb_obstacles_on_side >= 1 || millis()-time>6000)  // && dist_right_inf_42)
      {
        nb_obstacles_on_side = 0;
        state += 2;
        //desired_speed=200;
        time = millis();
      }
      break;

    case ETN3_FIRST_TURN_ADJUSTMENTS:
      turn_left = -180;

      if (dist_right_inf_21) {
        turn_left = -turn_left;  //turn less sharp right
                                 // allumer une led bleue pour les tests
      } else if (dist_left_inf_21) {
        //turn sharp right
      }

      if (millis() - time > 6000)  // ajust value for the right distance
      {
        state += 1;
        time = millis();
      }
      break;
    case ETN3_FIRST_CORRIDOR:
      handle_first_alignement(RIGHT);

      if (not(dist_left_inf_70)) {
        state += 1;
        time = millis();
        not_aligned = true;
        nb_obstacles_on_side=0;
      }

      break;
    case ETN3_SECOND_TURN:
      turn_side(LEFT);

      if (not(obstacle_on_side) && millis() - time > 2000) {
        nb_obstacles_on_side += dist_right_inf_42;
      }
      if (obstacle_on_side) {
        time = millis();
      }
      obstacle_on_side = dist_right_inf_42;


      if (nb_obstacles_on_side >= 1)  // && dist_right_inf_42)
      {
        nb_obstacles_on_side = 0;
        state += 1;
        time = millis();
      }
      break;
    case ETN3_INTER_CORRIDOR:
      //turn_left = (left_dist_lower * 2 - 1) * 40;
      handle_first_alignement(LEFT);

      if (not(dist_left_inf_70)) {
        state += 1;
        time = millis();
        not_aligned = true;
      }
      break;

    case ETN3_THIRD_TURN:
      turn_side(LEFT)

      if (not(obstacle_on_side) && millis() - time > 2000) {
        nb_obstacles_on_side += dist_right_inf_42;
      }
      if (obstacle_on_side) {
        time = millis();
      }
      obstacle_on_side = dist_right_inf_42;
      if (nb_obstacles_on_side >= 1) {
        state += 1;
        time = millis();
      }
      break;
    case ETN3_LAST_CORRIDOR:
      handle_first_alignement(LEFT);
      if (not(dist_right_inf_70)) {
        state += 1;
        time = millis();
        not_aligned = true;
        nb_obstacles_on_side=0;
      }
      break;
    case ETN3_LAST_TURN:
      
      turn_side(RIGHT)

      if (not(obstacle_on_side) && millis() - time > 2000) {
        nb_obstacles_on_side += dist_left_inf_42;
      }
      if (obstacle_on_side) {
        time = millis();
      }
      obstacle_on_side = dist_left_inf_42;



      if (nb_obstacles_on_side >= 1)  // && dist_right_inf_42)
      {
        nb_obstacles_on_side = 0;
        state += 1;
        //desired_speed=200;
        time = millis();
      }

      break;
    case ETN3_EXIT_MAZE:
      
      turn_left*=2;

      if (not(dist_right_inf_70) && not(dist_left_inf_70)) {
        turn_left = 0;
        desired_speed = 255;
      }

      //handle_first_alignement(false);

      break;
    // tests :
    case TEST_FIRST_TURN:
      //go forward for a few centimeters

      if (millis() - time > 1000)  // ajust value for the right distance
      {
        turn_left = 0;
        desired_speed = 0;
      }
      break;
    case TEST_FIRST_TURN_ADJUSTMENTS:
      turn_left = -100;
      if (dist_right_inf_21) {
        turn_left = -turn_left;  //turn less sharp right
                                 // allumer une led bleue pour les tests
      } else if (dist_left_inf_21) {
        //turn sharp right
      }

      if (millis() - time > 6000)  // ajust value for the right distance
      {
        turn_left = 0;
        desired_speed = 0;
      }
      break;
    case TEST_FIRST_CORRIDOR:
      if (not(dist_left_inf_70)) {
        turn_left = 0;
        desired_speed = 0;
      }
      break;
    case TEST_SECOND_TURN:
      turn_left = 125;
      //turn=-int(0.9*TURN_SPEED)
      if (dist_left_inf_21) {
        turn_left = -turn_left;  //turn less sharp left
                                 // allumer une led bleue pour les tests
      } else if (dist_right_inf_21) {
        //turn sharp left
      }


      if (millis() - time > 5000)  // ajust value for the right distance

      {
        turn_left = 0;
        desired_speed = 0;
      }
      break;

    case TEST_THIRD_TURN:
      turn_left = 250;
      desired_speed = 200;
      break;
    case TEST_LAST_CORRIDOR:
      if (not(dist_right_inf_70)) {
        turn_left = 0;
        desired_speed = 0;
      }
      break;
    case TEST_LAST_TURN:
      turn_left = -180;

      if (dist_right_inf_21) {
        turn_left = -(turn_left);  //turn less sharp right
      } else if (dist_left_inf_21) {
        //turn sharp right
      }

      if (millis() - time > 8000)  // ajust value for the right distance
      {
        turn_left = 0;
        desired_speed = 0;
        time = millis();
      }
      break;
    case TEST_EXIT_MAZE:
      desired_speed = 255;
      break;
  }

  prev_turn_left = sign_of_int(turn_left);

  if (state % 10 != 0) {
    adjust_desired_speed();
  }

  //accelerate
  update_engine_tension(turn_left, desired_speed);



  // OUTPUTS
  analogWrite(PIN_LEFT_WHEEL_POWER, left_wheel_power);
  analogWrite(PIN_RIGHT_WHEEL_POWER, right_wheel_power);
  digitalWrite(PIN_LEFT_WHEEL_DIR, left_wheel_dir);
  digitalWrite(PIN_RIGHT_WHEEL_DIR, right_wheel_dir);

  delay(15);
}

//const int PIN_LEFT_WHEEL_CRENEAU=2;// not used because lack of pins
//const int PIN_RIGHT_WHEEL_CRENEAU=3;// not used because lack of pins


//bool left_wheel_creneau;
//bool right_wheel_creneau;

// bool prev_l_creneau;
// bool prev_r_creneau;


/*
int left_wheel_rotation(bool left_wheel_creneau){
  return int(left_wheel_creneau!=prev_l_creneau);
}

int right_wheel_rotation(bool right_wheel_creneau){
  return int(right_wheel_creneau!=prev_r_creneau);
}
*/




