class Particle{
  
  PVector pos, birthPlace;
  PVector vel;

  float trans;
  float size;
  float damping;
      
  public
  Particle(PVector _pos){
  
    birthPlace = new PVector(_pos.x, _pos.y, _pos.z);
    
    pos = new PVector(_pos.x, _pos.y, _pos.z);
    
    float v = 1.0;
    vel = new PVector(random(-v, v), random(-v, v), random(-v, v));

    
    damping = 0.91;
    trans = 255;
    size = random(0.5, 2);


  }  
  
  void update(){
    
    pos.add(vel);
    vel.mult(damping);
    
    //fade out
    if(trans > 0){
      trans -= 4.5;
    }
    
  }
  
  void display(){
    
    pushMatrix();


    strokeWeight(1);
    stroke(255, trans);
    
    translate(pos.x, pos.y, pos.z);
    quad(-size, 0, 0, -size, size, 0, 0, size);
    
    rotateY(PI/2);
    quad(-size, 0, 0, -size, size, 0, 0, size);

    rotateX(PI/2);
    quad(-size, 0, 0, -size, size, 0, 0, size);
    
    
    popMatrix();

    line(birthPlace.x, birthPlace.y, birthPlace.z, pos.x, pos.y, pos.z);
    
  }
  
  
  
}
