#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
    /// Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{ help h usage ? || show this message }"
        "{ color c|| (required) path to image }"
    );

    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }

    /// Collect data from arguments
    string image_color_loc(parser.get<string>("color"));
    if (image_color_loc.empty()){
        cerr << "error while reading your images, check if you put the correct path.";
        parser.printMessage();
        return -1;
    }

    /// Loading the gray image and showing it on screen
    Mat imageColor= imread(image_color_loc);
    if(imageColor.empty()) {
        cerr << "error while loading your images, check if you put the correct path.";
        return -1;
    }
    imshow("OriginalAdaptedColor", imageColor);
    waitKey(0);

    /// Segmenting all the skin pixels from loaded picture
    vector<Mat> channels;
    split(imageColor,channels);
    Mat BLUE = channels[0];
    Mat GREEN = channels[1];
    Mat RED = channels[2];
    Mat canvas = Mat::zeros(imageColor.rows, imageColor.cols, CV_8UC1);

    canvas = (RED>95) & (GREEN>40) & (BLUE>20) & ((max(RED,max(GREEN,BLUE)) - min(RED,min(GREEN,BLUE)))>15) & (abs(RED-GREEN)>15) & (RED>GREEN) & (RED>BLUE);
    canvas = canvas*255;
    imshow("All skin pixels", canvas);
    waitKey(0);

    /// Using erode and dilate to suppress the noise
    /// First getting rid of noise
    erode(canvas, canvas, Mat(), Point(-1,-1),2);
    dilate(canvas, canvas, Mat(), Point(-1,-1),2);
    imshow("no noise", canvas);
    waitKey(0);

    /// Then connecting blobs
    dilate(canvas, canvas, Mat(), Point(-1,-1),5);
    erode(canvas, canvas, Mat(), Point(-1,-1),5);
    imshow("connecting blobs", canvas);
    waitKey(0);

    /// Drawing contours on the binary image
    vector<vector<Point>> contours;
    findContours(canvas.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    vector<vector<Point>> hulls;
    for(size_t i=0; i < contours.size(); i++) {
        vector<Point> hull;
        convexHull(contours[i],hull);
        hulls.push_back(hull);
    }
    drawContours(canvas, hulls, -1, 255, -1);

    /// Merging the original image with the contoured image in order to get a "mask"
    Mat finaal(imageColor.rows, imageColor.cols, CV_8UC3);
    Mat PBLUE = channels[0] & canvas;
    Mat PGREEN = channels[1] & canvas;
    Mat PRED = channels[2] & canvas;

    Mat in[] = {PBLUE, PGREEN, PRED};
    int from_to[] = {0,0,1,1,2,2};
    mixChannels(in, 3, &finaal, 1, from_to, 3);

    imshow("Mask", finaal);
    waitKey(0);

    return 0;
}
