#include <iostream>
#include <opencv2/opencv.hpp>

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

///extracting the last bits of the encrypted image which are from the hidden image
int extractBGR(int n) {
    int macht = pow(10,8-bits);
    int rest = n%macht;
    return pow(10,bits)*rest;
}

int main(int argc, const char** argv)
{
    /// Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{ help h usage ? || show this message }"
        "{ image1 i1|| (required) path to image }"
    );

    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }

    /// Collect data from arguments
    string image_image1_loc(parser.get<string>("image1"));
    if (image_image1_loc.empty()){
        cerr << "error while reading your image, check if you put the correct path.";
        parser.printMessage();
        return -1;
    }

    /// Loading the image and showing it on screen
    Mat img1 = imread(image_image1_loc);
    if(img1.empty()) {
        cerr << "error while loading your image1, check if you put the correct path.";
        return -1;
    }

    ///Showing encrypted image on screen
    imshow("1", img1 );
    waitKey(0);

    Mat canvas = Mat::zeros(Size(img1.size()), CV_8UC3); ///A black default image
    int original_w, original_h; ///Width and height of the original hidden image

    ///Splitting the images into the BGR channels
    vector<Mat> channels1, channels2;
    split(img1,channels1);
    split(canvas,channels2);

    /// Triple for-loop that iterates over each pixel and its color channel
    for(int row = 0; row < img1.rows; row++) {
        for(int column = 0; column < img1.cols; column++) {
            for(int i = 0; i < 3; i++) {
                int bgr, bgr1;
                bgr1 = intToBin(channels1[i].at<uchar>(row, column));
                bgr = extractBGR(bgr1);
                int pixelvalue = binToInt(bgr);
                channels2[i].at<uchar>(row, column) = pixelvalue;
            }

            ///If-test to check if hidden image is smaller than the original, so we can crop the black bars
            if(channels2[0].at<uchar>(row, column)!=0 || channels2[1].at<uchar>(row, column)!=0 || channels2[2].at<uchar>(row, column)!=0) {
                original_h = row+1;
                original_w = column+1;
            }
        }
    }
    ///Merging the channels into an image
    merge(channels2, canvas);

    ///Cropping the image
    Mat nieuw = canvas(Rect(0,0,original_w,original_h));

    ///Saving the image and showing it on screen
    imwrite( "decrypted.jpg", nieuw);
    imshow("Decrypted image", nieuw );
    waitKey(0);
    return 0;
}
