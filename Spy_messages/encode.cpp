#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
using namespace cv;

int bits = 4;

///Converting a decimal into a binary number
int intToBin(int decimal) {
    int binary = 0;
    int remainder, i = 1;
    while (decimal!=0)
    {
        remainder = decimal%2;
        decimal /= 2;
        binary += remainder*i;
        i *= 10;
    }
    return binary;
}

///Converting a binary number into a decimal
int binToInt(int binary) {
    int decimal = 0, i = 0, remainder;
    while (binary!=0)
    {
        remainder = binary%10;
        binary /= 10;
        decimal += remainder*pow(2,i);
        ++i;
    }
    return decimal;
}

///Merging the two bitgroups together
int mergeBGR(int n, int o) {
    ///Converting to string to make concatenating easier
    string s = to_string(n);
    string t = to_string(o);

    ///Dropping the last "bits" bits
    s = s.substr(0,s.size()-bits);
    t = t.substr(0,t.size()-bits);

    ///Adding zeros in front of the second bitgroup so the alignment is still correct
    while(t.size()<4)
        t="0"+t;

    ///Adding them together
    string test = s+t;
    return stoi(test);
}

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
    if (image_image1_loc.empty() || image_image2_loc.empty()){
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
    if(img2.rows > img1.rows || img2.cols > img1.cols) {
        cerr << "Image 1 size is lower than image 2 size!";
        return -1;
    }

    ///Showing the images on screen
    imshow("1", img1 );
    imshow("2", img2 );
    waitKey(0);

    ///A black default image
    Mat canvas = Mat::zeros(Size(img1.size()), CV_8UC3);

    ///Splitting the images into the BGR channels
    vector<Mat> channels1, channels2, channels3;
    split(img1,channels1);
    split(img2,channels2);
    split(canvas,channels3);

    /// Triple for-loop that irritates over each pixel and its color channel
    for(int row = 0; row < img1.rows; row++) {
        for(int column = 0; column < img1.cols; column++) {
            for(int i = 0; i < 3; i++) {
                int bgr, bgr1, bgr2;
                bgr1 = intToBin(channels1[i].at<uchar>(row, column));
                bgr2 = intToBin(0); ///Take black as default
                if(row < img2.rows && column < img2.cols) ///If-test to check if still in range of second image
                    bgr2 = intToBin(channels2[i].at<uchar>(row, column));
                bgr = mergeBGR(bgr1,bgr2);
                int pixelvalue = binToInt(bgr);
                channels3[i].at<uchar>(row, column) = pixelvalue;
            }
        }
    }

    ///Merging the channels into an image
    merge(channels3, canvas);

    ///Saving the image and showing it on screen
    imwrite( "encrypted.png", canvas); ///important to save as an .png because .jpg alters the pixelvalues
    imshow("Jones", canvas );
    waitKey(0);
    return 0;
}
