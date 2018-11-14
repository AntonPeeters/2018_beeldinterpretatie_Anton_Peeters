#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int h1;
int h2;
int s1;
const int alpha_slider_max = 255;
int alpha_slider = 168;
const int alpha_slider_max2 = 255;
int alpha_slider2 = 10;
const int alpha_slider_max3 = 255;
int alpha_slider3 = 130;

static void in_trackbar(int, void*)
{
    h1=alpha_slider;
}

static void in_trackbar2(int, void*)
{
    h2=alpha_slider2;
}

static void in_trackbar3(int, void*)
{
    s1=alpha_slider3;
}

int main(int argc, const char** argv)
{
    /// Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{ help h usage ? || show this message }"
        "{ sign1 s1|| (required) path to image }"
    );

    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }

    /// Collect data from arguments
    string image_sign1_loc(parser.get<string>("sign1"));
    if (image_sign1_loc.empty()){
        cerr << "error while reading your images, check if you put the correct path.";
        parser.printMessage();
        return -1;
    }

    /// Loading the traffic sign image and showing it on screen
    Mat imageSign1= imread(image_sign1_loc);
    if(imageSign1.empty()) {
        cerr << "error while loading your images, check if you put the correct path.";
        return -1;
    }
    imshow("OriginalSign1", imageSign1);
    waitKey(0);

    /// Segmenting all the red pixels from loaded picture using BGR
    vector<Mat> channels;
    split(imageSign1,channels);
    Mat BLUE = channels[0];
    Mat GREEN = channels[1];
    Mat RED = channels[2];

    Mat RED_th;
    threshold(RED, RED_th, 150, 255,THRESH_BINARY);
    imshow("Only traffic sign", RED_th);
    waitKey(0);

    /// Merging the original image with the segmented image in order to get a "mask"
    Mat finaal(imageSign1.rows, imageSign1.cols, CV_8UC3);
    Mat PBLUE = channels[0] & RED_th;
    Mat PGREEN = channels[1] & RED_th;
    Mat PRED = channels[2] & RED_th;

    Mat in[] = {PBLUE, PGREEN, PRED};
    int from_to[] = {0,0,1,1,2,2};
    mixChannels(in, 3, &finaal, 1, from_to, 3);

    imshow("Mask", finaal);
    waitKey(0);

    /// Create Windows
    namedWindow("With trackbars", 1);

    /// Create Trackbars
    createTrackbar( "Hue min", "With trackbars", &alpha_slider, alpha_slider_max, in_trackbar );
    createTrackbar( "Hue max", "With trackbars", &alpha_slider2, alpha_slider_max2, in_trackbar2 );
    createTrackbar( "Saturation", "With trackbars", &alpha_slider3, alpha_slider_max3, in_trackbar3 );

    /// Show some stuff
    in_trackbar( alpha_slider, 0 );
    in_trackbar2( alpha_slider2, 0 );
    in_trackbar3( alpha_slider3, 0 );

    while(true)
    {
        /// Segmenting all the red pixels from loaded picture using HSV
        vector<Mat> channels2;
        Mat imageSign1_hsv;
        cvtColor(imageSign1, imageSign1_hsv, CV_BGR2HSV);

        split(imageSign1_hsv,channels2);
        Mat HUE = channels2[0];
        Mat SAT = channels2[1];
        Mat VAL = channels2[2];

        Mat h_dest1, h_dest2, s_dest;
        inRange(HUE, h1,180,h_dest1);
        inRange(HUE, 0,h2,h_dest2);
        inRange(SAT, s1,255,s_dest);

        HUE = (h_dest1 | h_dest2) & s_dest;

        /// Using erode and dilate to suppress the noise
        /// First getting rid of noise
        erode(HUE, HUE, Mat(), Point(-1,-1),2);
        dilate(HUE, HUE, Mat(), Point(-1,-1),2);

        /// Then connecting blobs
        dilate(HUE, HUE, Mat(), Point(-1,-1),5);
        erode(HUE, HUE, Mat(), Point(-1,-1),5);

        imshow("With trackbars", HUE);

        /// Wait until user press some key for 50ms
        int iKey = waitKey(50);

        ///if user press 'Enter' key
        if (iKey == 13)
        {
            break;
        }
    }

    /// Drawing contours on the traffic sign
    vector<Mat> channels3;
    Mat imageSign1_hsv;
    cvtColor(imageSign1, imageSign1_hsv, CV_BGR2HSV);

    split(imageSign1_hsv,channels3);
    Mat HUE = channels3[0];
    Mat SAT = channels3[1];
    Mat VAL = channels3[2];

    /// Using the above selected values of the user
    Mat h_dest1, h_dest2, s_dest;
    inRange(HUE, h1,180,h_dest1);
    inRange(HUE, 0,h2,h_dest2);
    inRange(SAT, s1,255,s_dest);

    HUE = (h_dest1 | h_dest2) & s_dest;
	
	/// Displaying contours(blue) and convexHull(purple) to outline the traffic sign
    Mat canvas = imageSign1.clone();
    vector<vector<Point>> contours;
    findContours(HUE.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    vector<Point> grootste_blob = contours[0];
    vector<Point> hull_blob = contours[0];
    for(size_t i=0; i < contours.size(); i++) {
        if(contourArea(contours[i]) > contourArea(grootste_blob)){
            grootste_blob = contours[i];
        }
        if(contourArea(contours[i]) > contourArea(hull_blob)){
            convexHull(contours[i],hull_blob);
        }
    }
    vector<vector<Point>> temp;
    vector<vector<Point>> hulls;
    temp.push_back(grootste_blob);
    hulls.push_back(hull_blob);
    drawContours(canvas, temp, -1, Scalar(255,255,0), 3);
    drawContours(canvas, hulls, -1, Scalar(255,0,255), 3);
    imshow("verkeersbord",canvas);
    waitKey(0);

    return 0;
}
