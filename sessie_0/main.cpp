#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
    /// Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{ help h usage ? || show this message }"
        "{ grey g|| (required) path to image }"
        "{ color c|| (required) path to image }"
    );

    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }

    /// Collect data from arguments
    string image_grey_loc(parser.get<string>("grey"));
    string image_color_loc(parser.get<string>("color"));
    if (image_grey_loc.empty() || image_color_loc.empty()){
        cerr << "error while reading your images, check if you put the correct path.";
        parser.printMessage();
        return -1;
    }

    /// Loading the gray image and showing it on screen
    Mat imageGrey = imread(image_grey_loc);
    if(imageGrey.empty()) {
        cerr << "error while loading your images, check if you put the correct path.";
        return -1;
    }
    imshow("EAVISE Logo - grey", imageGrey);
    waitKey(0);

    /// Loading the color image and showing it on screen
    Mat imageColor = imread(image_color_loc);
    if(imageColor.empty()) {
        return -1;
    }
    imshow("EAVISE Logo - color", imageColor);
    waitKey(0);

    /// Splitting the color image into the 3 rgb values
    vector<Mat> channels;
    split(imageColor,channels);
    imshow("Blue value", channels[0]);
    imshow("Green value", channels[1]);
    imshow("Red value", channels[2]);
    waitKey(0);

    /// Converting the color image into a grey image
    Mat image_to_grey;
    cvtColor(imageColor, image_to_grey, COLOR_BGR2GRAY);
    imshow("Color to grey", image_to_grey);
    waitKey(0);

    /// Prints values of greyscale to the command line;
    for(int row = 0; row < image_to_grey.rows; row++)
    {
        for(int column = 0; column < image_to_grey.cols; column++) {
            cout << (int)image_to_grey.at<uchar>(row, column);
            cout << ";";
        }
        cout << endl;
    }
    cout << endl;
    waitKey(0);

    /// Drawing a rectangle, circle and ellipse on a black canvas
    Mat canvas = Mat::zeros(Size(250,250), CV_8UC3);
    rectangle(canvas, Point(50, 50), Point(150, 100), Scalar(255, 0, 0), 3);
    circle(canvas, Point(100, 100), 20, Scalar(0, 255, 0));
    ellipse(canvas, Point(150,200),Size(50,30),0.0,0.0,360.0, Scalar(0, 0, 255),2);
    imshow("Canvas", canvas);
    waitKey(0);
    return 0;
}
