
float sqrt(float y) {
  x = y / 2;
  for(i = 0; i < 10000; i+=1) {
    x = (x + y/x) / 2;
  }
  return x;
}

