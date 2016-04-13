#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
#include "transformer.h"
#include "graphics.h"

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <string>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <ctime>

using std::min;
using std::max;
using namespace rgb_matrix;

bool extractDate(const std::string& s, int& d, int& m, int& y);

class SendText : public ThreadedCanvasManipulator {
public:
  SendText(Canvas *m) : ThreadedCanvasManipulator(m) {}
  void Run(){
    
    //load the text file and put it into a string:
    std::ifstream in("programMessage.txt");
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string message  = buffer.str();
    
    size_t pos1 = 0;
    size_t pos2;

    //create matrix to store the strings.
    std::string str[1000][5];

    for(int y=0; y<=2; y++){
      for (int x=0; x<=4; x++){
       if(x==4){
	   pos2 = message.find("^",pos1);
	 } else {
	   pos2 = message.find("|",pos1);
	 }
        str[y][x] = message.substr(pos1, (pos2-pos1));
     	
        std::cout << str[y][x] << std::endl;
        std::cout << "pos1:" << pos1 << ", pos2:" << pos2 << std::endl;
        pos1 = pos2+1;

	if(str[y][x]== ""){
	  y=1001; 
	}
      }
      
    }

    //Get Today date
    time_t t = time(0);
    struct tm * now = localtime(& t);
    int dn,mn,yn;
    dn = now->tm_mday;
    mn = now->tm_mon + 1;
    yn = now->tm_year + 1900;
   
    //Choose only  today massage to send to the display
    std::string strText;
    for(int j=0; j<=2; j++){
     int d,m,y; 
     extractDate(str[j][2], d, m, y);
     if (d==dn && m==mn && y==yn){
       strText= str[j][1];
     }
   }
    
    //Get date Message
    /* int d,m,y;
    if (extractDate(str[0][2], d, m, y))
        std::cout << "date " 
                  << y << "-" << m << "-" << d
                  << " is valid" << std::endl;
    else
    std::cout << "date is invalid" << std::endl;
    */
    
  Color color(100, 0, 200);
  const int width = canvas()->width()-1; 
  const char *bdf_font_file = "fonts/10x20.bdf";


  const char * c = strText.c_str();  
  const char *text= c;

  printf("este es el size  %u",(unsigned)strlen(text));
  rgb_matrix::Font font;
  font.LoadFont(bdf_font_file);

 int i = 0;
 int x;
  while(true){
    x = width - i;
    DrawText(canvas(),font,x, 6 + font.baseline(), color,text);
    usleep(30000);
    canvas()->Clear();
    ++i;

    if(x==(0-((int)strlen(text)*10))){
      i=0;
    }
  }
  }
};

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s <options> -D <demo-nr> [optional parameter]\n",
          progname);
  return 1;
}

int main(int argc, char *argv[]) {
  GPIO io;
  bool as_daemon = false;
  int runtime_seconds = -1;
  int rows = 32;
  int chain = 4;
  int parallel = 1;
  int pwm_bits = -1;
  int brightness = 100;
  int rotation = -1;
  bool do_luminance_correct = true;
 
  // Initialize GPIO pins
  if (!io.Init())
    return 1;

  // Start daemon
  if (as_daemon) {
    if (fork() != 0)
      return 0;
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }

  // The matrix and display updater.
  RGBMatrix *matrix = new RGBMatrix(&io, rows, chain, parallel);
  matrix->set_luminance_correct(do_luminance_correct);
  matrix->SetBrightness(brightness);
  if (pwm_bits >= 0 && !matrix->SetPWMBits(pwm_bits)) {
    fprintf(stderr, "Invalid range of pwm-bits\n");
    return 1;
  }

  LinkedTransformer *transformer = new LinkedTransformer();
  matrix->SetTransformer(transformer);

  if (rotation > 0) {
    transformer->AddTransformer(new RotateTransformer(rotation));
  }

  Canvas *canvas = matrix;

  // The ThreadedCanvasManipulator objects are filling
  // the matrix continuously.
  ThreadedCanvasManipulator *image_gen = NULL;
  
   image_gen = new SendText(canvas);
 
  if (image_gen == NULL)
    return usage(argv[0]);

  //Run Text on Display.
  image_gen->Start();

   
  
  if (as_daemon) {
    sleep(runtime_seconds > 0 ? runtime_seconds : INT_MAX);
  } else if (runtime_seconds > 0) {
    sleep(runtime_seconds);
  } else {
    // Things are set up. Just wait for <RETURN> to be pressed.
    printf("Press <RETURN> to exit and reset LEDs\n");
    getchar();
  }

  // Stop image generating thread.
  delete image_gen;
  delete canvas;

  transformer->DeleteTransformers();
  delete transformer;

  return 0;
}



bool extractDate(const std::string& s, int& d, int& m, int& y){
    std::istringstream is(s);
    char delimiter;
    if (is >> y >> delimiter >> m >> delimiter >> d) {
        struct tm t = {0};
        t.tm_mday = d;
        t.tm_mon = m - 1;
        t.tm_year = y - 1900;
        t.tm_isdst = -1;

        // normalize:
        time_t when = mktime(&t);
        const struct tm *norm = localtime(&when);
        return (
	        norm->tm_year == y - 1900  &&
		norm->tm_mon  == m - 1 &&
		norm->tm_mday == d   
               );
    }
    return false;
}
