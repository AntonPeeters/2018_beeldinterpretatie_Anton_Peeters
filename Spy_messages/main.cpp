#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


int main(int argc, const char** argv)
{
    /// Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{ help h usage ? || show this message }"
        "{ image1 i1|| (required) path to image }"
        "{ image2 i2|| (required) path to image }"
    );

    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }

    /// Collect data from arguments
    string image_image1_loc(parser.get<string>("image1"));
    string image_image2_loc(parser.get<string>("image2"));
    if (image_image1_loc.empty() || image_image1_loc.empty()){
        cerr << "error while reading your images, check if you put the correct path.";
        parser.printMessage();
        return -1;
    }

    /// Loading the image and showing it on screen
    Mat img1 = imread(image_image1_loc);
    Mat img2 = imread(image_image2_loc);
    if(img1.empty()) {
        cerr << "error while loading your image1, check if you put the correct path.";
        return -1;
    }
    if(img2.empty()) {
        cerr << "error while loading your image2, check if you put the correct path.";
        return -1;
    }
    return 0;
}
