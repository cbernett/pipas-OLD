/*

 Visualization of Kite data
 
 NOTE: ENTIRE sketch is in HSB color mode at 0-255 scale
 
 -----useable files-----
 circle_still.txt
 circle_slow_sample.txt
 move_wait_move_wait_X.txt
 xyz_then_still.txt
 varied_fast_sample.txt
 varied_slow_sample.txt
 varied_motions.txt
 random_slow.txt
 
 
 */



String fileName = "LOG00056.TXT";


String[] lines;

//array for all the data points parsed from file
DataPoint[] dataList;

//Arrays to hold the riemann integrated velocity and position data
PVector[] velocities;

//arrays for paths that will be drawn
PVector[] path, xpath, ypath, zpath;

ArrayList<Particle> particles = new ArrayList<Particle>();

//current point being processed/drawn
int currentDataPoint = 0;

//Current point for animation
PVector pos;

//what we'll use to scale the visualization
float scaleVisuals = 1.0; 

//how big the visualization should be in coordinate space after scaling
float maxVisDimension = 600;  

//the header on the data file takes the 
//first 6 lines so data starts on line 7
int dataStartingLine = 6;

//How long to wait before drawing the next point, 
//i.e. how fast we're drawing the animation
float animationDelay = 20; 

//variable to keep track of time
long lastTime = 0;

//Object orientation
float yaw = 0;  //Z
float pitch = 0;  //Y
float roll = 0;  //X


//Visual/UI stuff
PImage backgroundGradient;
float backgroundHue;
PVector worldCenter;

//colors
color xCol, yCol, zCol;

//camera stuff
PVector easedCameraPos;
PVector rawCameraPos;
float easingSpeed = 0.08;

//timer to wait after finished drawing
int waitAfterComplete = 5000;

//helper variable
float pctComplete = 0;

//switch to start and stop playback
boolean playing = true;


void setup() {

  frameRate(60);
  size(1000, 600, P3D);
  //fullScreen(P3D);

  maxVisDimension = 650;

  colorMode(HSB, 255);

  //set colors at random, these can be connected to interface inputs
  //
  xCol = color(random(255), 255, 255, 150);
  yCol = color(random(255), 255, 255, 150);
  zCol = color(random(255), 255, 255, 150);

  //gradient is a fake, we're just placing a transparent sun burst over a colored background
  backgroundGradient = loadImage("circleGradient.png");

  //colored background
  backgroundHue = random(255);
  //backgroundHue = color(0);


  //camera controls
  rawCameraPos = new PVector(0, 0);
  easedCameraPos = new PVector(0, 0);


  //fill lines array with lines from file
  lines = loadStrings(fileName);

  //subtract file header lines 
  dataList = new DataPoint[lines.length - dataStartingLine];

  //index of actual data
  int dataIndex = 0;

  //parse through data line by line
  for (int i = dataStartingLine; i < lines.length; i++) {
    String[] pieces = split(lines[i], TAB);

    //pass this line of the data file to the constructor
    //and let it parse the data into the correct variables
    dataList[dataIndex] = new DataPoint(pieces);

    dataIndex++;
  }


  //get the average value of all acceleration data  
  PVector average = new PVector(0, 0, 0);

  for (int i = 0; i < dataList.length; i++) {
    average.add(dataList[i].linAcc);
  }

  average.div(dataList.length);


  //Go through each datapoint and subtract the average value from each
  //point to try to normalize any drifting
  for (int i = 0; i < dataList.length; i++) {
    dataList[i].linAcc.sub(average);
  }


  //Now we do Riemann sum integrations on the sensor data 
  //first initialize arrays using trapezoidal method:
  // trapezoid = 0.5 * base * (height 1 + height 2)
  //           = 0.5 * time increment * (linear accel_1 + linear accel_2)

  velocities = new PVector[dataList.length - 1];  

  //make first slot zero
  velocities[0] = new PVector(0, 0, 0);

  //now go through the array and integrate the acceleration
  //to fill the velocities array

  //the sum of all the accelerations up to the current point
  PVector velSum = new PVector(0, 0, 0);

  for (int i = 1; i < velocities.length; i++) {

    //the current velocity to be added
    PVector added = new PVector(0, 0, 0);

    //sum of current linear accel plus next linear accel
    added = PVector.add(dataList[i].linAcc, dataList[i - 1].linAcc);     // (height 1 + height 2)

    //then multiply by the base of the trapezoid 
    velSum.add(PVector.mult(added, 0.5 * dataList[i].sampleTime));       // 0.5 * base * (above result)

    //store it
    velocities[i] = new PVector(velSum.x, velSum.y, velSum.z);
  }


  //    SCALING FOR BETTER VISUALIZATION

  //find the center point of all the data so we can make that the 
  //center of the coordinate system for better viewing
  worldCenter = new PVector(0, 0, 0);

  //and get the max dimension so we can scale so everything fits
  PVector maxDimension = new PVector(0, 0, 0);

  for (int i = 0; i < velocities.length; i++) {

    if (velocities[i].mag() > maxDimension.mag()) 
      maxDimension.set(velocities[i].x, velocities[i].y, velocities[i].z);

    //add everything so we can find the center of all the data later
    worldCenter.add(velocities[i]);
  }

  //Find how much we need to scale the visualization so the maximum 
  //dimension still fits inside the viewport 
  scaleVisuals = maxVisDimension/maxDimension.mag();

  //divide by two since the maxDimension is a radius, not diameter
  //Nevermind, keep it 1, it looks better a little blown up
  scaleVisuals /= 1; 

  //now divide to get the center of all the data
  worldCenter.div(velocities.length);
  worldCenter.mult(scaleVisuals);


  //Box at head of data is drawn at "pos"

  //initialize position
  pos = new PVector(0, 0, 0);

  //initialize path arrays we're gonna draw
  path = new PVector[velocities.length];
  xpath = new PVector[velocities.length];
  ypath = new PVector[velocities.length];
  zpath = new PVector[velocities.length];

  //fill with PVectors
  for (int i = 0; i < path.length; i++) {
    path[i] = new PVector(0, 0, 0);
    xpath[i] = new PVector(0, 0, 0);
    ypath[i] = new PVector(0, 0, 0);
    zpath[i] = new PVector(0, 0, 0);
  }
}



void draw() {

  //not needed in web
  surface.setTitle(str(frameRate));

  //if, by chance the interface has a chance to change speed you can tweak this value
  //10 is about as fast as you can go, 100 is about as slow as you want. 
//  animationDelay = 20;


  //hsb mode
  colorMode(HSB, 255);

  //background, desaturate and reduce brightness
  background(backgroundHue, 200, 80);
  //background(0);


  //gradient fake out: image of transparent bright spot 
  //in front of a colored background
  imageMode(CENTER);
  tint(255, 200);

  //need to push it back in the Z direction so it's behind the visuals
  pushMatrix();
  translate(width/2, height/2, 0); 
  image(backgroundGradient, 0, 0, width, height);
  popMatrix();

  //makes the box look purty (kinda)
  lights();


  //this is where the data is processed and the path information is determined

  if (playing && millis() - lastTime > animationDelay && currentDataPoint < dataList.length - 1) {

    //get orientation from the sensor data in 3-2-1 (ZYX) order
    yaw = radians(dataList[currentDataPoint].euler.z);
    pitch = radians(dataList[currentDataPoint].euler.y);
    roll = radians(dataList[currentDataPoint].euler.x);

    //set the current position of the box to the velocity data
    pos.set(new PVector(velocities[currentDataPoint].x, velocities[currentDataPoint].y, velocities[currentDataPoint].z));

    //scale it so it fits within the viewport
    pos.mult(scaleVisuals);

    //add the current point to the main path
    path[currentDataPoint].set(pos.x, pos.y, pos.z);

    //add points on xyz path lines too
    float distFromBaseline = constrain(map(dataList[currentDataPoint].linAcc.mag(), -10, 10, 1, 5), 1, 5);

    //create unit vectors to use them for rotations
    PVector xAxis = new PVector(1, 0, 0);
    PVector yAxis = new PVector(0, -1, 0);
    PVector zAxis = new PVector(0, 0, 1);    

    //build a vector in X
    PVector x = new PVector(1, 0, 0);
    //rotate in 3-2-1 scheme (Z-Y-X)
    x = rotate3D(x, zAxis, yaw);
    x = rotate3D(x, yAxis, pitch);
    x = rotate3D(x, xAxis, roll);    
    x.mult(distFromBaseline);

    xpath[currentDataPoint].set(PVector.add(pos, x));

    //build a vector in Y
    PVector y = new PVector(0, 1, 0);
    //rotate in 3-2-1 scheme (Z-Y-X)
    y = rotate3D(y, zAxis, yaw);
    y = rotate3D(y, yAxis, pitch);
    y = rotate3D(y, xAxis, roll);
    y.mult(distFromBaseline);

    ypath[currentDataPoint].set(PVector.add(pos, y));

    //build a vector in Z
    PVector z = new PVector(0, 0, 1);
    //rotate in 3-2-1 scheme (Z-Y-X)
    z = rotate3D(z, zAxis, yaw);
    z = rotate3D(z, yAxis, pitch);
    z = rotate3D(z, xAxis, roll);    
    z.mult(distFromBaseline);

    zpath[currentDataPoint].set(PVector.add(pos, z));


    //move to next point in data
    currentDataPoint++;

    //log the time so we know when to get the next datapoint
    lastTime = millis();
  }


  //calculate where we are in the animation from 0-1
  pctComplete = float(currentDataPoint)/float(dataList.length - 1);




  //replay when we've gone through all the points
  //but reset things first
  if (currentDataPoint == dataList.length - 1 && millis() - lastTime > waitAfterComplete) {

    //reset the counter
    currentDataPoint = 0;

    //and clear the path information
    for (int i = 0; i < path.length; i++) {
      path[i].set(0, 0, 0);
      xpath[i].set(0, 0, 0);
      ypath[i].set(0, 0, 0);
      zpath[i].set(0, 0, 0);
    }

    particles.clear();
  }



  //add particles here box head is if we're animating
  if (playing && pctComplete < 1.0) {
    particles.add(new Particle(pos));
    particles.add(new Particle(pos));
    particles.add(new Particle(pos));
  }




  //rotate globally with mouse 
  //DO NOT USE THE orbitControl() method. This is better! 

  //if the mouse is pressed, change camera position to mouse pos
  if (mousePressed) {
    float camX = map(mouseX, 0, width, -PI, PI);
    float camY = map(mouseY, 0, height, -PI, PI);

    rawCameraPos.set(camX, camY);
  } 
  else {

    //if mouse isn't pressed, make the camera move slowly through X
    //and up-down sinusoidally in Y

    //degrees rotated per frame
    float xSpeed = radians(0.1); 

    //how much the Y oscillates
    float yAmp = radians(30);  
    float ySpeed = 0.3;
    float time = float(millis())/1000.0;

    //loop the number around so X value never gets too big.
    //prevents weird transitions from auto pan to mouse mode and back.
    if (xSpeed > radians(360)) {
      xSpeed -= radians(360);
    }

    if(playing){
      //Add the X
      rawCameraPos.x = rawCameraPos.x + xSpeed;
      //Set the Y
      rawCameraPos.y = yAmp * sin(time * ySpeed);
    }
  }

  //apply easing to raw position
  easedCameraPos.lerp(rawCameraPos, easingSpeed);




  //global matrix rotated according to camera
  pushMatrix();
  translate(width/2, height/2, 200);
  rotate(easedCameraPos.x, 0, 1, 0);
  rotate(easedCameraPos.y, -1, 0, 0);

  //draw a circle at maximum visual dimension in XY plane
  noFill();
  strokeWeight(1);
  stroke(0, 255 * 0.15);

  //ellipse(0, 0, maxVisDimension, maxVisDimension);

  //draw one in XZ plane...
  pushMatrix();
  rotateY(PI/2);
  //ellipse(0, 0, maxVisDimension, maxVisDimension);
  popMatrix();

  //and YZ plane
  pushMatrix();
  rotateX(PI/2);
  //ellipse(0, 0, maxVisDimension, maxVisDimension);
  popMatrix();


  //move everything so it sits in the middle of the viewport
  translate(-worldCenter.x, -worldCenter.y, -worldCenter.z); 

  //Draw anchor box but only if we're animating, go away once we're done
  if (pctComplete < 1.0) {
    strokeWeight(1);
    stroke(50);
    fill(255);
    box(5, 5, 5);
  }


  //draw all the points in the path arrayList
  //two points needed to draw a line so make sure theres more than 1 in there
  if (path.length > 1) {

    //draw all the lines
    for (int i = 0; i < currentDataPoint - 1; i++) {

      float lineTrans = 255;
      //BASELINE
      strokeWeight(1);
      strokeJoin(ROUND);
      strokeCap(ROUND);
      stroke(0);
      line(path[i].x, path[i].y, path[i].z, path[i + 1].x, path[i + 1].y, path[i + 1].z);

      //----------X LINE----------
      strokeWeight(1);
      strokeJoin(ROUND);
      strokeCap(ROUND);
      stroke(xCol, lineTrans);
      //X itself
      line(xpath[i].x, xpath[i].y, xpath[i].z, xpath[i + 1].x, xpath[i + 1].y, xpath[i + 1].z);

      //now from X to the baseline 
      line(path[i].x, path[i].y, path[i].z, xpath[i].x, xpath[i].y, xpath[i].z);

      //now the criss cross between the path and the X line
      line(path[i].x, path[i].y, path[i].z, xpath[i + 1].x, xpath[i + 1].y, xpath[i + 1].z);
      line(path[i + 1].x, path[i + 1].y, path[i + 1].z, xpath[i].x, xpath[i].y, xpath[i].z);


      //----------Y LINE----------
      strokeWeight(1);
      strokeJoin(ROUND);
      strokeCap(ROUND);
      stroke(yCol, lineTrans);
      //Y line
      line(ypath[i].x, ypath[i].y, ypath[i].z, ypath[i + 1].x, ypath[i + 1].y, ypath[i + 1].z);
      //line from baseline to Y
      line(path[i].x, path[i].y, path[i].z, ypath[i].x, ypath[i].y, ypath[i].z);
      //criss cross
      line(path[i].x, path[i].y, path[i].z, ypath[i + 1].x, ypath[i + 1].y, ypath[i + 1].z);
      line(path[i + 1].x, path[i + 1].y, path[i + 1].z, ypath[i].x, ypath[i].y, ypath[i].z);

      //----------Z LINE----------
      strokeWeight(1);
      strokeJoin(ROUND);
      strokeCap(ROUND);
      stroke(zCol, lineTrans);
      //Z line
      line(zpath[i].x, zpath[i].y, zpath[i].z, zpath[i + 1].x, zpath[i + 1].y, zpath[i + 1].z);      
      //Z to baseline
      line(path[i].x, path[i].y, path[i].z, zpath[i].x, zpath[i].y, zpath[i].z);
      //criss cross
      line(path[i].x, path[i].y, path[i].z, zpath[i + 1].x, zpath[i + 1].y, zpath[i + 1].z);
      line(path[i + 1].x, path[i + 1].y, path[i + 1].z, zpath[i].x, zpath[i].y, zpath[i].z);
    }
  }



  //update and particles if we're playing
  if (playing) {
    for (int i = 0; i < particles.size(); i++) {

      Particle p = particles.get(i);

      p.update();

      //remove particles that are old
      if (p.trans < 3) {
        particles.remove(i);
      }
    }
  }


  //but draw them at all times
  for (int i = 0; i < particles.size(); i++) {

    Particle p = particles.get(i);
    p.display();
    
  }

  //draw box at Pos to denote current orientation
  //box() method draws at origin so translate matrix to pos
  pushMatrix();
  translate(pos.x, pos.y, pos.z);

  //rotate 3-2-1 
  rotateZ(yaw);
  rotateY(-pitch);    //not sure why but this needs to be negative to look right
  rotateX(roll);


  if (pctComplete < 1.0) {
    strokeWeight(1);
    stroke(50);
    fill(255);
    box(5, 5, 5);
  }


  popMatrix();     // x matrix

    popMatrix();     // global mouse roted matrix





    //draw the percentage in the corner. This might be nixed
  fill(255);
  textSize(20);
  textAlign(LEFT, TOP);
  //text("Completo: " + str(round(100 * pctComplete)) + "%", 10, 10);
}

void keyPressed() {
  playing = !playing;
}


// Modified and corrected version of code from "JohnG" @
//https://www.processing.org/discourse/beta/num_1203497293.html

//Rotates a 3D vector around an arbitrary axis by an angle (in radians)
PVector rotate3D(PVector v, PVector _axis, float ang)
{

  _axis.normalize();
  PVector vnorm = v;
  vnorm.normalize();

  float _parallel = PVector.dot(_axis, vnorm); //dot product
  PVector parallel = PVector.mult(_axis, _parallel); //multiply all elements by a value
  PVector perp = PVector.sub(parallel, vnorm); //subtract one vector from another
  PVector Cross = v.cross(_axis); //cross product

  PVector result = new PVector(0, 0, 0);

  result.add(parallel);
  result.add(PVector.mult(Cross, -sin(ang)));
  result.add(PVector.mult(perp, -cos(ang)));

  return result;
} 