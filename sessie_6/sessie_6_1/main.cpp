#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void detectAndDisplay(Mat frame);

String face_cascade_name = "haarcascade_frontalface_alt.xml";
String betterface_cascade_name = "lbpcascade_frontalface_improved.xml";
CascadeClassifier face_cascade;
CascadeClassifier betterface_cascade;
RNG rng(12345);

int main(int argc, const char** argv)
{
    /// Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{ help h usage ? || show this message }"
        "{ video v1|| (required) path to image }"
    );

    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }

    /// Collect data from arguments
    string video_video_loc(parser.get<string>("video"));
    if (video_video_loc.empty()){
        cerr << "error while reading your video, check if you put the correct path.";
        parser.printMessage();
        return -1;
    }

    /// Create a VideoCapture object and open the input file
    /// If the input is the web camera, pass 0 instead of the video file name
    VideoCapture cap(video_video_loc);

    /// Check if camera opened successfully
    if(!cap.isOpened()){
        cout << "Error opening video file" << endl;
        return -1;
    }


   /// Load the cascades
   if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading 1\n"); return -1; };
   if( !betterface_cascade.load( betterface_cascade_name ) ){ printf("--(!)Error loading 2\n"); return -1; };

    while(1) {

        Mat frame;
        /// Capture frame-by-frame
        cap >> frame;

        /// If the frame is empty, break immediately
        if (frame.empty())
            break;

        /// Display the resulting frame
        detectAndDisplay(frame);

        /// Press  ESC on keyboard to exit
        char c=(char)waitKey(25);
        if(c==27)
            break;
    }

    /// When everything done, release the video capture object
    cap.release();

    /// Closes all the frames
    destroyAllWindows();

    return 0;

}

void detectAndDisplay(Mat frame)
{
    vector<Rect> faces;
    vector<Rect> betterfaces;

    vector<int> faces_score;
    vector<int> betterfaces_score;

    Mat frame_gray;

    cvtColor( frame, frame_gray, CV_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );

    /// Detect faces
    face_cascade.detectMultiScale( frame_gray, faces, faces_score, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );

    for( size_t i = 0; i < faces.size(); i++ )
    {
        Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
        ellipse( frame, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 2, 8, 0 );
        Point pt1(faces[i].x, faces[i].y);
        putText(frame, to_string(faces_score[i]), pt1, 1, 1, Scalar(0,0,255));
    }

    betterface_cascade.detectMultiScale( frame_gray, betterfaces, betterfaces_score, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30) );

    for( size_t i = 0; i < betterfaces.size(); i++ )
    {
        Point pt1(betterfaces[i].x + betterfaces[i].width, betterfaces[i].y + betterfaces[i].height);
        rectangle(frame, betterfaces[i], Scalar( 0, 255, 0 ), 2);
        putText(frame, to_string(betterfaces_score[i]), pt1, 1, 1, Scalar(0,0,255));
    }

  /// Show what you got
  imshow("Jones", frame );
}
