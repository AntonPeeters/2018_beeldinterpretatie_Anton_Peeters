#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

vector<Point> detectAndDisplay(Mat frame, vector<Point> path);

HOGDescriptor hog;

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

    /// Open the input file
    VideoCapture cap(video_video_loc);

    /// Check if read successfully
    if(!cap.isOpened()){
        cout << "Error opening video file" << endl;
        return -1;
    }

    vector<Point> path;

    while(1) {

        Mat frame;
        /// Capture frame-by-frame
        cap >> frame;

        /// If the frame is empty, break immediately
        if (frame.empty())
            break;

        /// Display the resulting frame
        path = detectAndDisplay(frame, path);

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

vector<Point> detectAndDisplay(Mat frame, vector<Point> path)
{
    vector<Rect> person;
    vector<double> weights;

    Mat frame_gray;

    /// Make image bigger so it tracks better
    resize(frame, frame, Size(), 2, 2);

    /// Convert to gray
    cvtColor( frame, frame_gray, CV_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );

    /// Detect person
    hog = HOGDescriptor();
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
    hog.detectMultiScale(frame_gray, person, weights);

    /// Drawing rectangle around person
    for( size_t i = 0; i < person.size(); i++ )
    {
        Point center( person[i].x + person[i].width*0.5, person[i].y + person[i].height*0.5 );
        path.push_back(center);
        rectangle(frame, person[i], Scalar( 0, 255, 0 ), 2);
        Point pt1(person[i].x, person[i].y);
        putText(frame, to_string(weights[i]), pt1, 1, 1, Scalar(0,0,255));
    }

    /// Drawing the path the person takes
    for(size_t i = 0; i < path.size(); i++)
    {
        polylines(frame, path, false, Scalar(255,255,255), 4);
    }

    imshow("Hog", frame );
    return path;
}
