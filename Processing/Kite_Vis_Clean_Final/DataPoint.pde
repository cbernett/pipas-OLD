class DataPoint{
  
  float sampleTime;
  PVector euler;
  PVector linAcc;

  
  public
  DataPoint(String[] pieces){
    
    //multiply by 0.001 to convert from milliseconds to seconds
    sampleTime = float(pieces[0]) * 0.001;
    
    //Sensor outputs ZYX, so re-order pieces in the Euler pvector for clarity 
    euler = new PVector(float(pieces[3]), float(pieces[2]), float(pieces[1]));
    linAcc = new PVector(float(pieces[4]), float(pieces[5]), float(pieces[6]));
    
  }
  
  
}