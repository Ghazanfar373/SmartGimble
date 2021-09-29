#include <libserial/SerialPort.h>                                   //         C++14     g++7.5.0
//#include <cstdlib>
//#include <fstream>
//#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <JetsonGPIO.h>
using namespace cv;
using namespace std;
//using namespace GPIO;
int input_pin = 18; // BOARD pin 12, BCM pin 18

void setupGPIO(){
// Jetson Pin Definitions
GPIO::setmode(GPIO::BOARD);
// set pin as an input pin to read modes from Node MCU
GPIO::setup(input_pin, GPIO::IN);
}

int gimbleMode = 0;
int x,y;
bool flag=false;
float fps;

 Rect2d bbox(360, 260, 80, 80);
 Ptr<Tracker> tracker;
 string trackerType ;
 
// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()

//...................................................... Serial Port.............................
constexpr const char* const SERIAL_PORT = "/dev/ttyUSB0";
using namespace LibSerial;
 SerialPort serial_port;
int openSerialPort(){
try{
    serial_port.Open(SERIAL_PORT);
    }
catch (const OpenFailed&)
   {
        std::cerr << "The Serial port did not open correctly." << std::endl;
        return EXIT_FAILURE;
    }
    serial_port.SetBaudRate(BaudRate::BAUD_115200);
    serial_port.SetCharacterSize(CharacterSize::CHAR_SIZE_8);
    serial_port.SetFlowControl(FlowControl::FLOW_CONTROL_NONE);
    serial_port.SetParity(Parity::PARITY_NONE);
    serial_port.SetStopBits(StopBits::STOP_BITS_1);
    std::cout << "waiting  for tracker to send packet bytes" << std::endl;
    }

//....................................................................end Serial Port...........................
void initializeTracker(){
    // List of tracker types in OpenCV 3.4.1
    string trackerTypes[8] = {"BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN", "MOSSE", "CSRT"};
    // vector <string> trackerTypes(types, std::end(types));
    // Create a tracker
     trackerType = trackerTypes[2];
    //Ptr<Tracker> tracker;
    #if (CV_MINOR_VERSION < 3)
    {
        tracker = Tracker::create(trackerType);
    }
    #else
    {
        if (trackerType == "BOOSTING")
            tracker = TrackerBoosting::create();
        if (trackerType == "MIL")
            tracker = TrackerMIL::create();
        if (trackerType == "KCF")
            tracker = TrackerKCF::create();
        if (trackerType == "TLD")
            tracker = TrackerTLD::create();
        if (trackerType == "MEDIANFLOW")
            tracker = TrackerMedianFlow::create();
        if (trackerType == "GOTURN")
            tracker = TrackerGOTURN::create();
        if (trackerType == "MOSSE")
            tracker = TrackerMOSSE::create();
        if (trackerType == "CSRT")
            tracker = TrackerCSRT::create();
    }
    #endif
    }



int main(int argc, char **argv)
{
char buff[5];
buff[0]=0x4d;
buff[4]=0x6a;
buff[2]=0x2d;

openSerialPort();

setupGPIO();

initializeTracker();
    
    // Read video
    VideoCapture video("/dev/video0");
video.set(CAP_PROP_FRAME_WIDTH,800);
video.set(CAP_PROP_FRAME_HEIGHT,600);    
    // Exit if video is not opened
    if(!video.isOpened())
    {
        cout << "Could not read video file" << endl; 
        return 1; 
    } 

    // Read first frame 
    Mat frame; 
    bool ok = video.read(frame); 
    // Define initial bounding box 
    //Rect2d bbox(360, 260, 80, 80);
    // Uncomment the line below to select a different bounding box 
     //bbox = selectROI(frame, false); 
    // Display bounding box. 
    //rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 ); 
    //imshow("Tracking", frame);
    
    while(video.read(frame))
    {
        if(flag){
        // Start timer
        double timer = (double)getTickCount();
        
        // Update the tracking result
        bool ok = tracker->update(frame, bbox);
        
        // Calculate Frames per second (FPS)
         fps = getTickFrequency() / ((double)getTickCount() - timer);
        
        if (ok)
        {
            // Tracking success : Draw the tracked object
            rectangle(frame, bbox, Scalar( 0, 0, 255 ), 2, 1 );
               
             Point center = Point(bbox.x + (bbox.width)/2 , bbox.y +(bbox.height)/2);
           
        
           putText(frame, "X : " + SSTR(center.x) + " Y : " + SSTR(center.y), Point(100,80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
           x = center.x;    x = x-200; x=x/2;
           y = center.y;    y = y/2;
           buff[1]= (char)x;
           buff[3]= (char)y;
           serial_port.Write(buff);
           circle(frame, Point(center.x,center.y),5, Scalar(50,170,255),-1,8,0);
            
        }
        else
        {
            // Tracking failure detected.
            putText(frame, "Tracking failure detected", Point(100,80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255),2);
        }
        }else{
        rectangle(frame, bbox, Scalar(50,170,50),2,1);
        //TO check resolution of frame   
        //int width = video.get(CV_CAP_PROP_FRAME_WIDTH);
        //int height = video.get(CV_CAP_PROP_FRAME_HEIGHT); 
        //putText(frame, SSTR(int(width))+" * "+  SSTR(int(height)), Point(100,70), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
                }
        // Display tracker type on frame
        trackerType = "GOTURN";
        putText(frame, trackerType + " Tracker", Point(100,20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50),2);
        
        // Display FPS on frame
        putText(frame, "FPS : " + SSTR(int(fps)), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
        
                // Display frame.
        imshow("Turbo Tracker", frame);
        
        
        // Exit if ESC pressed.
        int k = waitKey(1);
       gimbleMode =	GPIO::input(input_pin);
        cout << "Mode: " << gimbleMode <<endl;
        if(k == 97)
        //if(gimbleMode == 1)
        {
                flag = true;
                tracker->init(frame, bbox);
        }
        //if(k == 27 || gimbleMode == 0 && flag == true)
        if(k == 27)  {
           flag=false;
        bbox.width  = 80;
        bbox.height = 80;
        bbox.x      = 360;
        bbox.y      = 260;   
        tracker->~Tracker();
        initializeTracker();
            
        }if(k == 113)  {
            break;
            cvDestroyWindow("Turbo Tracker");
            serial_port.Close();
            }

    }
}


